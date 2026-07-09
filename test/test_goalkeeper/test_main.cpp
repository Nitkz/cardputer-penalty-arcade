#include <unity.h>
#include <cmath>

// Mock Arduino random function
long mock_random_val = 0;
long random(long max) {
    if (max == 0) return 0;
    return mock_random_val % max;
}

#include "../../src/Goalkeeper.h"

void setUp(void) {
    mock_random_val = 0;
}

void tearDown(void) {
}

void test_update_not_playing_moves_goalkeeper() {
    Goalkeeper gk;
    gk.x = 100;
    gk.direction = 1;
    gk.baseSpeed = 4.0;
    gk.speedMultiplier = 1.0;

    gk.update(320, 0, false);

    TEST_ASSERT_EQUAL_FLOAT(104.0, gk.x);
    TEST_ASSERT_FALSE(gk.wasPlaying);
    TEST_ASSERT_FALSE(gk.isDiving);
}

void test_update_caps_speed_at_10() {
    Goalkeeper gk;
    gk.x = 100;
    gk.direction = 1;
    gk.baseSpeed = 4.0;
    gk.speedMultiplier = 1.0;

    gk.update(320, 100, false);

    TEST_ASSERT_EQUAL_FLOAT(110.0, gk.x);
}

void test_update_bounces_at_edges() {
    Goalkeeper gk;
    // to match class default values
    gk.x = 320 - gk.width / 2; // right edge
    gk.direction = 1;
    gk.update(320, 0, false);

    TEST_ASSERT_EQUAL_FLOAT(320 - gk.width / 2, gk.x);
    TEST_ASSERT_EQUAL(-1, gk.direction);

    gk.x = gk.width / 2; // left edge
    gk.direction = -1;
    gk.update(320, 0, false);

    TEST_ASSERT_EQUAL_FLOAT(gk.width / 2, gk.x);
    TEST_ASSERT_EQUAL(1, gk.direction);
}

void test_update_starts_diving_when_playing() {
    Goalkeeper gk;
    gk.x = 160;
    gk.direction = 1;
    gk.wasPlaying = false;

    mock_random_val = 0; // Will pick lane 0 (Left)

    gk.update(320, 0, true);

    TEST_ASSERT_TRUE(gk.wasPlaying);
    TEST_ASSERT_TRUE(gk.isDiving);
    TEST_ASSERT_EQUAL(-1, gk.direction);
    TEST_ASSERT_EQUAL_FLOAT(6.0, gk.diveSpeed);
}

void test_update_long_dive_speed_boost() {
    Goalkeeper gk;
    gk.x = 320 / 6.0f; // Left lane
    gk.direction = 1;
    gk.wasPlaying = false;

    mock_random_val = 2; // Will pick lane 2 (Right)

    gk.update(320, 0, true);

    TEST_ASSERT_EQUAL_FLOAT(10.0, gk.diveSpeed);
    TEST_ASSERT_EQUAL(1, gk.direction);
}

void test_update_stops_diving_when_target_reached() {
    Goalkeeper gk;
    gk.wasPlaying = true;
    gk.isDiving = true;
    gk.x = 100;
    gk.targetX = 110;
    gk.direction = 1;
    gk.diveSpeed = 12.0;

    gk.update(320, 0, true);

    TEST_ASSERT_EQUAL_FLOAT(110.0, gk.x);
    TEST_ASSERT_FALSE(gk.isDiving);
    TEST_ASSERT_EQUAL(0, gk.direction);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_update_not_playing_moves_goalkeeper);
    RUN_TEST(test_update_caps_speed_at_10);
    RUN_TEST(test_update_bounces_at_edges);
    RUN_TEST(test_update_starts_diving_when_playing);
    RUN_TEST(test_update_long_dive_speed_boost);
    RUN_TEST(test_update_stops_diving_when_target_reached);
    return UNITY_END();
}
