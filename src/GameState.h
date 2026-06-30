#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <Arduino.h>

enum class State {
  START,
  PLAY,
  GOAL,
  SAVED,
  GAMEOVER
};

class GameState {
public:
  State currentState = State::START;
  int scoreGoals = 0;
  int scoreSaves = 0;
  int totalAttempts = 0;
  int highScoreGoals = 0;
  int lives = 3;

  // Ball state
  float ballX = 120.0;
  float ballY = 280.0;
  const float ballBaseRadius = 8.0;
  float ballRadius = 8.0;
  float ballTargetX = 120.0;
  const int ballTargetY = 30;
  float ballSpeedX = 0;
  float ballSpeedY = 0;
  float ballBaseSpeed = 10.0;
  float currentBallSpeed = 10.0;

  unsigned long resultTime = 0;

  void resetBall(int screenWidth, int screenHeight) {
    ballX = screenWidth / 2.0;
    ballY = screenHeight - 40.0;
    ballRadius = ballBaseRadius;

    if (lives <= 0) {
      currentState = State::GAMEOVER;
    } else {
      currentState = State::START;
    }
  }

  void restartGame(int screenWidth, int screenHeight) {
    scoreGoals = 0;
    scoreSaves = 0;
    totalAttempts = 0;
    lives = 3;
    currentBallSpeed = ballBaseSpeed;
    resetBall(screenWidth, screenHeight);
  }

  void shoot(int targetX, float dx, float dy, float dist) {
    ballTargetX = targetX;
    ballSpeedX = (dx / dist) * currentBallSpeed;
    ballSpeedY = (dy / dist) * currentBallSpeed;
    currentState = State::PLAY;
    totalAttempts++;
  }
};

#endif // GAMESTATE_H
