#ifndef LANE_UTILS_H
#define LANE_UTILS_H

class LaneUtils {
public:
  static int getLaneIndexFromTouch(int touchX, int screenWidth) {
    if (touchX < screenWidth / 3) {
      return 0; // Left zone
    } else if (touchX < 2 * screenWidth / 3) {
      return 1; // Center zone
    } else {
      return 2; // Right zone
    }
  }

  static float getLaneTargetX(int laneIndex, int screenWidth) {
    if (laneIndex == 0) {
      return screenWidth / 6.0f;
    } else if (laneIndex == 1) {
      return screenWidth / 2.0f;
    } else {
      return 5.0f * screenWidth / 6.0f;
    }
  }
};

#endif // LANE_UTILS_H
