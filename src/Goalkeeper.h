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

  void increaseSpeed() {
    speedMultiplier *= 1.15f;
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

      if (direction == 0) direction = (random(2) == 0) ? 1 : -1;

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

        // Randomly stay or dive (50% chance)
        bool stay = random(2) == 0;

        int targetLane = currentLane;
        if (!stay) {
          if (currentLane == 0) {
            targetLane = 1;
          } else if (currentLane == 2) {
            targetLane = 1;
          } else {
            targetLane = (random(2) == 0) ? 0 : 2;
          }
        }

        targetX = lanes[targetLane];
        diveSpeed = currentSpeed * 1.5f;
        if (diveSpeed > 10.0f) diveSpeed = 10.0f;

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
