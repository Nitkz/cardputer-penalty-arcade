#include <Arduino.h>
#include <M5GFX.h>
#include <lgfx/v1/panel/Panel_ST7789.hpp>
#include <SPI.h>

// ST7789 Pin mapping
#define TFT_CS    5
#define TFT_RST   3
#define TFT_DC    6
#define TFT_MOSI  14
#define TFT_SCLK  40
#define TFT_MISO  39

// Touch Pin
#define TOUCH_CS  4

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
      cfg.invert           = true;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = true; // Share the bus
      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};

Custom_ST7789 display;

// Standard SPI bus used by Touch
SPIClass mySPI(FSPI);

// Sprite to reduce flickering during drag
LGFX_Sprite sprite(&display);

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

// Game States
enum GameState {
  WAITING,
  SHOOTING,
  RESULT
};

GameState gameState = WAITING;

// Goalkeeper variables
float gkX = 120.0;
const int gkY = 30;
const int gkWidth = 70;
const int gkHeight = 15;
float gkSpeed = 4.0;
int gkDirection = 1;

// Ball variables
float ballX = 120.0;
float ballY = 280.0;
const int ballRadius = 8;
float ballTargetX = 120.0;
const int ballTargetY = 30;
float ballSpeedX = 0;
float ballSpeedY = 0;

// Score variables
int scoreGoals = 0;
int scoreSaves = 0;
unsigned long resultTime = 0;
String resultMessage = "";

void setup() {
  Serial.begin(115200);
  
  pinMode(TOUCH_CS, OUTPUT);
  digitalWrite(TOUCH_CS, HIGH);

  // Initialize shared SPI bus
  mySPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, -1);

  display.init();
  display.setRotation(2); // Portrait, flipped 180 degrees
  display.fillScreen(TFT_BLACK);
  
  // Create a sprite the size of the screen
  sprite.createSprite(display.width(), display.height());
  sprite.setTextDatum(middle_center);
}

void resetBall() {
  ballX = display.width() / 2;
  ballY = display.height() - 40;
  gameState = WAITING;
}

void loop() {
  // Read touch pressure (Z)
  uint16_t z1 = xpt2046_read_data(0xB1);
  uint16_t z2 = xpt2046_read_data(0xC1);
  int z = z1 + 4095 - z2;
  
  bool isTouched = (z > 400); // Z threshold
  
  int touchX = -1;
  int touchY = -1;

  if (isTouched) {
    // Read X and Y
    uint16_t rawX = xpt2046_read_data(0xD1);
    uint16_t rawY = xpt2046_read_data(0x91);

    touchX = map(rawX, 300, 3800, 0, display.width());
    touchY = map(rawY, 300, 3800, display.height(), 0); // Inverted Y for rotation 2
    
    // Constrain to screen bounds
    touchX = constrain(touchX, 0, display.width());
    touchY = constrain(touchY, 0, display.height());
  }

  // Goalkeeper logic
  gkX += gkSpeed * gkDirection;
  if (gkX - gkWidth / 2 < 0) {
    gkX = gkWidth / 2;
    gkDirection = 1;
  } else if (gkX + gkWidth / 2 > display.width()) {
    gkX = display.width() - gkWidth / 2;
    gkDirection = -1;
  }

  if (gameState == WAITING) {
    if (isTouched) {
      // Determine zone
      if (touchX < display.width() / 3) {
        ballTargetX = display.width() / 6.0; // Left zone center
      } else if (touchX < 2 * display.width() / 3) {
        ballTargetX = display.width() / 2.0; // Center zone center
      } else {
        ballTargetX = 5 * display.width() / 6.0; // Right zone center
      }

      // Calculate speed vectors
      float dx = ballTargetX - ballX;
      float dy = ballTargetY - ballY;
      float dist = sqrt(dx * dx + dy * dy);
      float speed = 10.0; // Ball speed
      ballSpeedX = (dx / dist) * speed;
      ballSpeedY = (dy / dist) * speed;

      gameState = SHOOTING;
    }
  } else if (gameState == SHOOTING) {
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    // Check collision at goal line
    if (ballY <= gkY + gkHeight / 2 + ballRadius) {
      ballY = gkY + gkHeight / 2 + ballRadius; // Snap to line

      // Check if ball overlaps goalkeeper
      if (ballX + ballRadius > gkX - gkWidth / 2 && ballX - ballRadius < gkX + gkWidth / 2) {
        resultMessage = "SAVED!";
        scoreSaves++;
      } else {
        resultMessage = "GOAL!";
        scoreGoals++;
      }
      resultTime = millis();
      gameState = RESULT;
    }
  } else if (gameState == RESULT) {
    if (millis() - resultTime > 2000) {
      resetBall();
    }
  }

  // Draw everything to the sprite first
  sprite.fillScreen(display.color565(34, 139, 34)); // Grass green

  // Draw Goal area lines
  sprite.drawLine(0, gkY, display.width(), gkY, TFT_WHITE);

  // Draw touch zones (subtle lines)
  sprite.drawLine(display.width() / 3, 0, display.width() / 3, display.height(), display.color565(0, 100, 0));
  sprite.drawLine(2 * display.width() / 3, 0, 2 * display.width() / 3, display.height(), display.color565(0, 100, 0));

  // Draw Goalkeeper
  sprite.fillRect(gkX - gkWidth / 2, gkY - gkHeight / 2, gkWidth, gkHeight, TFT_RED);

  // Draw Ball
  sprite.fillCircle(ballX, ballY, ballRadius, TFT_WHITE);
  sprite.drawCircle(ballX, ballY, ballRadius, TFT_BLACK);

  // Draw Result text
  if (gameState == RESULT) {
    sprite.setFont(&fonts::Orbitron_Light_32);
    sprite.setTextColor(TFT_YELLOW);
    sprite.drawString(resultMessage, display.width() / 2, display.height() / 2);
  } else if (gameState == WAITING) {
    sprite.setFont(&fonts::Roboto_Thin_24);
    sprite.setTextColor(TFT_WHITE);
    sprite.drawString("TAP TO SHOOT", display.width() / 2, display.height() / 2);
  }

  // Draw Score
  sprite.setFont(&fonts::Roboto_Thin_24);
  sprite.setTextColor(TFT_WHITE);
  sprite.drawString("Goals: " + String(scoreGoals), 50, display.height() - 20);
  sprite.drawString("Saves: " + String(scoreSaves), display.width() - 50, display.height() - 20);

  // Push the sprite to the physical screen
  sprite.pushSprite(0, 0);

  delay(20);
}