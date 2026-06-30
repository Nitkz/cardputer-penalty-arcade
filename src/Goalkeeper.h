#ifndef GOALKEEPER_H
#define GOALKEEPER_H

class Goalkeeper {
public:
  float x = 120.0;
  const int y = 30;
  const int width = 70;
  const int height = 15;
  float speed = 4.0;
  int direction = 1;

  void update(int screenWidth, int totalAttempts) {
    // Optionally scale speed slightly based on total attempts
    float currentSpeed = speed + (totalAttempts * 0.2f);
    if (currentSpeed > 10.0f) currentSpeed = 10.0f; // Cap speed

    x += currentSpeed * direction;
    if (x - width / 2 < 0) {
      x = width / 2;
      direction = 1;
    } else if (x + width / 2 > screenWidth) {
      x = screenWidth - width / 2;
      direction = -1;
    }
  }

  bool checkCollision(float ballX, float ballRadius) const {
    // True if ball X overlaps with goalkeeper X
    return (ballX + ballRadius > x - width / 2) && (ballX - ballRadius < x + width / 2);
  }
};

#endif // GOALKEEPER_H
