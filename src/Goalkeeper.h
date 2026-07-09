#ifndef GOALKEEPER_H
#define GOALKEEPER_H

class Goalkeeper {
public:
  float x = 120.0;
  const int y = 30;
  const int width = 70;
  const int height = 15;
  float baseSpeed = 4.0;
  float speedMultiplier = 1.0;
  int direction = 1;

  bool isDiving = false;
  float targetX = 0;
  float diveSpeed = 0;
  bool wasPlaying = false;

  // Adaptive AI: Track player's habits (Left, Center, Right)
  int playerHabits[3] = {1, 1, 1}; // Start with 1 to give equal chance initially

  void registerPlayerShot(int lane) {
    if (lane >= 0 && lane <= 2) {
      playerHabits[lane]++;
    }
  }

  void increaseSpeed() {
    speedMultiplier *= 1.15f;
  }

  void decreaseSpeed() {
    speedMultiplier *= 0.8f;
    if (speedMultiplier < 1.0f) speedMultiplier = 1.0f;
  }

  void resetSpeed() {
    speedMultiplier = 1.0f;
  }

  void update(int screenWidth, int totalAttempts, bool isPlaying) {
    // Optionally scale speed slightly based on total attempts
    float currentSpeed = (baseSpeed * speedMultiplier) + (totalAttempts * 0.2f);
    if (currentSpeed > 10.0f) currentSpeed = 10.0f; // Cap speed

    if (!isPlaying) {
      wasPlaying = false;
      isDiving = false;

      if (direction == 0) direction = (esp_random() % 2 == 0) ? 1 : -1;

      x += currentSpeed * direction;
      if (x - width / 2 < 0) {
        x = width / 2;
        direction = 1;
      } else if (x + width / 2 > screenWidth) {
        x = screenWidth - width / 2;
        direction = -1;
      }
    } else {
      if (!wasPlaying) {
        wasPlaying = true;
        isDiving = true;

        float lanes[3] = { screenWidth / 6.0f, screenWidth / 2.0f, 5.0f * screenWidth / 6.0f };

        // Find closest lane
        int currentLane = 0;
        float minDist = abs(x - lanes[0]);
        for (int i = 1; i < 3; ++i) {
          float dist = abs(x - lanes[i]);
          if (dist < minDist) {
            minDist = dist;
            currentLane = i;
          }
        }

        // Adaptive AI: Dive based on player's historical shot distribution
        int totalShots = playerHabits[0] + playerHabits[1] + playerHabits[2];
        int r = totalShots > 0 ? esp_random() % totalShots : 0;
        int targetLane = 0;
        
        if (r < playerHabits[0]) {
          targetLane = 0;
        } else if (r < playerHabits[0] + playerHabits[1]) {
          targetLane = 1;
        } else {
          targetLane = 2;
        }

        targetX = lanes[targetLane];
        diveSpeed = currentSpeed * 1.5f;
        if (abs(targetLane - currentLane) == 2) {
          diveSpeed = currentSpeed * 2.5f;
        }
        if (diveSpeed > 12.0f) diveSpeed = 12.0f; // Slightly higher cap for long dives

        if (targetX > x) {
          direction = 1;
        } else if (targetX < x) {
          direction = -1;
        } else {
          direction = 0;
        }
      }

      if (isDiving) {
        if (direction != 0) {
          x += diveSpeed * direction;

          // Stop if reached or overshot targetX
          if ((direction == 1 && x >= targetX) || (direction == -1 && x <= targetX)) {
            x = targetX;
            isDiving = false; // Finished diving
            direction = 0;
          }
        }
      }
    }
  }

  bool checkCollision(float ballX, float ballRadius) const {
    // True if ball X overlaps with goalkeeper X
    return (ballX + ballRadius > x - width / 2) && (ballX - ballRadius < x + width / 2);
  }
};

#endif // GOALKEEPER_H
