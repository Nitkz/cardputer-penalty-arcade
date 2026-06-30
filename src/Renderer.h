#ifndef RENDERER_H
#define RENDERER_H

#include <M5GFX.h>
#include "GameState.h"
#include "Goalkeeper.h"

class Renderer {
private:
  M5Canvas* sprite;
  int screenWidth;
  int screenHeight;

public:
  Renderer(M5Canvas* s, int width, int height) : sprite(s), screenWidth(width), screenHeight(height) {}

  void draw(const GameState& state, const Goalkeeper& gk) {
    // Fill background (Grass green)
    sprite->fillScreen(sprite->color565(19, 163, 69));

    // Draw Goal area line
    sprite->drawLine(0, gk.y, screenWidth, gk.y, TFT_WHITE);

    // Draw touch zones (subtle lines)
    sprite->drawLine(screenWidth / 3, 0, screenWidth / 3, screenHeight, sprite->color565(0, 100, 0));
    sprite->drawLine(2 * screenWidth / 3, 0, 2 * screenWidth / 3, screenHeight, sprite->color565(0, 100, 0));

    // Draw Goalkeeper
    sprite->fillCircle(gk.x, gk.y - gk.height / 2 + 3, 5, TFT_RED);
    sprite->fillRect(gk.x - 3, gk.y - gk.height / 2 + 8, 6, 8, TFT_BLUE);
    sprite->drawLine(gk.x - 3, gk.y - gk.height / 2 + 8, gk.x - 10, gk.y - gk.height / 2 + 12, TFT_RED);
    sprite->drawLine(gk.x + 3, gk.y - gk.height / 2 + 8, gk.x + 10, gk.y - gk.height / 2 + 12, TFT_RED);
    sprite->drawLine(gk.x - 2, gk.y - gk.height / 2 + 16, gk.x - 5, gk.y + gk.height / 2, TFT_RED);
    sprite->drawLine(gk.x + 2, gk.y - gk.height / 2 + 16, gk.x + 5, gk.y + gk.height / 2, TFT_RED);

    // Draw Ball
    sprite->fillCircle(state.ballX, state.ballY, state.ballRadius, TFT_WHITE);
    sprite->drawCircle(state.ballX, state.ballY, state.ballRadius, TFT_BLACK);
    sprite->fillCircle(state.ballX, state.ballY, state.ballRadius / 2, TFT_BLACK);
    sprite->drawLine(state.ballX, state.ballY - state.ballRadius / 2, state.ballX, state.ballY - state.ballRadius, TFT_BLACK);
    sprite->drawLine(state.ballX - state.ballRadius / 2 + 1, state.ballY + state.ballRadius / 2 - 1, state.ballX - state.ballRadius + 1, state.ballY + state.ballRadius - 1, TFT_BLACK);
    sprite->drawLine(state.ballX + state.ballRadius / 2 - 1, state.ballY + state.ballRadius / 2 - 1, state.ballX + state.ballRadius - 1, state.ballY + state.ballRadius - 1, TFT_BLACK);

    // Draw Text Overlays
    sprite->setTextDatum(middle_center);
    if (state.currentState == State::GAMEOVER) {
      sprite->setFont(&fonts::Orbitron_Light_32);
      sprite->setTextColor(TFT_RED);
      sprite->drawString("GAME OVER", screenWidth / 2, screenHeight / 2 - 40);

      sprite->setFont(&fonts::Roboto_Thin_24);
      sprite->setTextColor(TFT_WHITE);
      sprite->drawString("TAP TO RESTART", screenWidth / 2, screenHeight / 2 + 20);
    } else if (state.currentState == State::GOAL) {
      sprite->setFont(&fonts::Orbitron_Light_32);
      sprite->setTextColor(TFT_YELLOW);
      sprite->drawString("GOAL!", screenWidth / 2, screenHeight / 2);
    } else if (state.currentState == State::SAVED) {
      sprite->setFont(&fonts::Orbitron_Light_32);
      sprite->setTextColor(TFT_YELLOW);
      sprite->drawString("SAVED!", screenWidth / 2, screenHeight / 2);
    } else if (state.currentState == State::START) {
      sprite->setFont(&fonts::Roboto_Thin_24);
      sprite->setTextColor(TFT_WHITE);
      sprite->drawString("TAP TO SHOOT", screenWidth / 2, screenHeight / 2);
    }

    // Push the sprite to the physical screen
    sprite->pushSprite(0, 0);
  }
};

#endif // RENDERER_H
