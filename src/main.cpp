#include <Arduino.h>
#include <M5Unified.h>
#include <SPI.h>
#include <M5GFX.h>
#include <lgfx/v1/panel/Panel_ST7789.hpp>
#include <Preferences.h>

#include "GameState.h"
#include "Goalkeeper.h"
#include "SoundEffects.h"
#include "Renderer.h"

// ST7789 Pin mapping
#define TFT_CS    5
#define TFT_RST   3
#define TFT_DC    6
#define TFT_MOSI  14
#define TFT_SCLK  40
#define TFT_MISO  39

// Touch Pin
#define TOUCH_CS  4

// Touch Commands
#define CMD_READ_Y  0x91
#define CMD_READ_Z1 0xB1
#define CMD_READ_Z2 0xC1
#define CMD_READ_X  0xD1

// Create a custom M5GFX configuration
class Custom_ST7789 : public lgfx::LGFX_Device
{
  lgfx::v1::Panel_ST7789  _panel_instance;
  lgfx::v1::Bus_SPI       _bus_instance;

public:
  Custom_ST7789(void)
  {
    { // Bus Config
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = false;
      cfg.use_lock   = true; // Required to share SPI bus with touch
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = TFT_SCLK;
      cfg.pin_mosi = TFT_MOSI;
      cfg.pin_miso = TFT_MISO;
      cfg.pin_dc   = TFT_DC;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // Panel Config
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = TFT_CS;
      cfg.pin_rst          = TFT_RST;
      cfg.pin_busy         = -1;
      cfg.panel_width      = 240;
      cfg.panel_height     = 320;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.offset_rotation  = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = true;
      cfg.invert           = false;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = true; // Share the bus
      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};

Custom_ST7789 display;
SPIClass mySPI(FSPI);
M5Canvas sprite(&display);
M5Canvas internalSprite(&M5.Display);

GameState gameState;
Goalkeeper goalkeeper;
SoundEffects sound;
Renderer* renderer;
Preferences preferences;

// Raw SPI read function for XPT2046
uint16_t xpt2046_read_data(uint8_t command) {
  mySPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  digitalWrite(TOUCH_CS, LOW);
  mySPI.transfer(command);
  uint16_t val = mySPI.transfer16(0);
  digitalWrite(TOUCH_CS, HIGH);
  mySPI.endTransaction();
  return val >> 3; // 12-bit ADC value
}

int lastScore = -1;
int lastLives = -1;
int lastHighScore = -1;

void setup() {
  Serial.begin(115200);

  // Initialize M5Unified (includes Speaker support)
  auto cfg = M5.config();
  M5.begin(cfg);

  sound.begin();
  
  pinMode(TOUCH_CS, OUTPUT);
  digitalWrite(TOUCH_CS, HIGH);

  // Initialize shared SPI bus
  mySPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, -1);

  display.init();
  display.setRotation(2); // Match orientation
  display.fillScreen(TFT_BLACK);
  
  sprite.createSprite(display.width(), display.height());
  
  M5.Display.setRotation(1); // Ensure Cardputer display is landscape
  internalSprite.createSprite(M5.Display.width(), M5.Display.height());

  renderer = new Renderer(&sprite, display.width(), display.height());

  // Load high score
  preferences.begin("penalty-game", false);
  gameState.highScoreGoals = preferences.getInt("highscore", 0);

  // Set initial game state positions
  gameState.resetBall(display.width(), display.height());
}

void loop() {
  M5.update(); // Keeps M5Unified background tasks (like sound fading) happy
  
  static uint32_t lastFrameTime = 0;
  if (millis() - lastFrameTime < 20) {
    return;
  }
  lastFrameTime = millis();

  display.waitDisplay(); // Wait for external display DMA to finish before using SPI for touch

  // Read touch pressure (Z)
  uint16_t z1 = xpt2046_read_data(CMD_READ_Z1);
  uint16_t z2 = xpt2046_read_data(CMD_READ_Z2);
  int z = z1 + 4095 - z2;
  
  bool isTouched = (z > 400); // Z threshold
  
  int touchX = -1;
  int touchY = -1;

  if (isTouched) {
    uint16_t rawX = xpt2046_read_data(CMD_READ_X);
    uint16_t rawY = xpt2046_read_data(CMD_READ_Y);

    touchX = map(rawX, 300, 3800, 0, display.width());
    touchY = map(rawY, 300, 3800, display.height(), 0); // Inverted Y for rotation 2
    
    touchX = constrain(touchX, 0, display.width());
    touchY = constrain(touchY, 0, display.height());
  }

  // Goalkeeper logic
  goalkeeper.update(display.width(), gameState.totalAttempts, gameState.currentState == State::PLAY);

  // State Machine
  if (gameState.currentState == State::START) {
    if (isTouched) {
      int targetX;
      int laneIndex = 0;
      // Determine 3-lane zone
      if (touchX < display.width() / 3) {
        targetX = display.width() / 6.0; // Left zone
        laneIndex = 0;
      } else if (touchX < 2 * display.width() / 3) {
        targetX = display.width() / 2.0; // Center zone
        laneIndex = 1;
      } else {
        targetX = 5 * display.width() / 6.0; // Right zone
        laneIndex = 2;
      }

      float dx = targetX - gameState.ballX;
      float dy = gameState.ballTargetY - gameState.ballY;
      float dist = sqrt(dx * dx + dy * dy);

      goalkeeper.registerPlayerShot(laneIndex);
      gameState.shoot(targetX, dx, dy, dist);
      sound.playShootSound();
    }
  } else if (gameState.currentState == State::PLAY) {
    gameState.ballX += gameState.ballSpeedX;
    gameState.ballY += gameState.ballSpeedY;

    // Shrink ball to simulate 3D depth
    float startY = display.height() - 40.0;
    float progress = (startY - gameState.ballY) / (startY - gameState.ballTargetY);
    if (progress < 0) progress = 0;
    if (progress > 1) progress = 1;
    gameState.ballRadius = gameState.ballBaseRadius * (1.0f - progress * 0.5f);

    // Check collision at goal line
    if (gameState.ballY <= goalkeeper.y + goalkeeper.height / 2 + gameState.ballRadius) {
      gameState.ballY = goalkeeper.y + goalkeeper.height / 2 + gameState.ballRadius; // Snap to line

      if (goalkeeper.checkCollision(gameState.ballX, gameState.ballRadius)) {
        gameState.currentState = State::SAVED;
        gameState.scoreSaves++;
        gameState.lives--; // Deduct life
        goalkeeper.decreaseSpeed();
        sound.playSavedSound();
      } else {
        gameState.currentState = State::GOAL;
        gameState.scoreGoals++;
        goalkeeper.increaseSpeed();
        gameState.currentBallSpeed += 0.5f;
        if (gameState.currentBallSpeed > 20.0f) gameState.currentBallSpeed = 20.0f;
        sound.playGoalSound();

        // Update high score if beaten
        if (gameState.scoreGoals > gameState.highScoreGoals) {
          gameState.highScoreGoals = gameState.scoreGoals;
          preferences.putInt("highscore", gameState.highScoreGoals);
        }
      }
      gameState.resultTime = millis();
    }
  } else if (gameState.currentState == State::GOAL || gameState.currentState == State::SAVED) {
    if (millis() - gameState.resultTime > 2000) {
      gameState.resetBall(display.width(), display.height());
    }
  } else if (gameState.currentState == State::GAMEOVER) {
    if (isTouched) {
      gameState.restartGame(display.width(), display.height());
      goalkeeper.resetSpeed();
      sound.playShootSound(); // Provide audio feedback for restart
      delay(200); // debounce touch
    }
  }

  // Render Frame
  renderer->draw(gameState, goalkeeper);

  // Render to Internal Display ONLY when stats change to prevent display/SPI bottlenecks
  if (gameState.scoreGoals != lastScore || gameState.lives != lastLives || gameState.highScoreGoals != lastHighScore) {
    lastScore = gameState.scoreGoals;
    lastLives = gameState.lives;
    lastHighScore = gameState.highScoreGoals;

    internalSprite.fillScreen(TFT_BLACK);
    internalSprite.setFont(&fonts::Orbitron_Light_24);
    internalSprite.setTextColor(TFT_WHITE);
    internalSprite.setTextDatum(top_center);
    internalSprite.drawString("PENALTY ARCADE", internalSprite.width() / 2, 10);

    internalSprite.setFont(&fonts::Roboto_Thin_24);
    internalSprite.setTextDatum(middle_left);
    char buf[32];
    snprintf(buf, sizeof(buf), "Score: %d", gameState.scoreGoals);
    internalSprite.drawString(buf, 10, internalSprite.height() / 2);

    internalSprite.setTextDatum(middle_right);
    if (gameState.lives > 0) {
      internalSprite.setTextColor(TFT_GREEN);
    } else {
      internalSprite.setTextColor(TFT_RED);
    }
    snprintf(buf, sizeof(buf), "Lives: %d", gameState.lives);
    internalSprite.drawString(buf, internalSprite.width() - 10, internalSprite.height() / 2);

    internalSprite.setTextDatum(bottom_center);
    internalSprite.setTextColor(TFT_YELLOW);
    snprintf(buf, sizeof(buf), "High Score: %d", gameState.highScoreGoals);
    internalSprite.drawString(buf, internalSprite.width() / 2, internalSprite.height() - 10);

    internalSprite.pushSprite(0, 0);
  }

}
