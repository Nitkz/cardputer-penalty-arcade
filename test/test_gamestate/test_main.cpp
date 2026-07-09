#include <unity.h>
#include <cmath>

#include "../../src/GameState.h"

void setUp(void) {}
void tearDown(void) {}

void test_shoot_state_change(void) {
    GameState gs;
    gs.currentState = State::START;
    gs.totalAttempts = 0;

    gs.shoot(150, 10.0, 20.0, 25.0);

    TEST_ASSERT_EQUAL(State::PLAY, gs.currentState);
    TEST_ASSERT_EQUAL_INT(1, gs.totalAttempts);
    TEST_ASSERT_EQUAL_INT(150, gs.ballTargetX);
}

void test_shoot_speed_calculation(void) {
    GameState gs;
    gs.currentBallSpeed = 10.0;

    // Test with simple 3-4-5 triangle
    // dx = 3, dy = 4, dist = 5
    gs.shoot(100, 3.0, 4.0, 5.0);

    TEST_ASSERT_EQUAL_FLOAT(6.0, gs.ballSpeedX); // (3/5) * 10 = 6
    TEST_ASSERT_EQUAL_FLOAT(8.0, gs.ballSpeedY); // (4/5) * 10 = 8
}

void test_shoot_zero_distance(void) {
    GameState gs;
    gs.currentBallSpeed = 10.0;

    // Original code doesn't guard against division by zero
    // Just test that the method runs and state updates correctly
    gs.shoot(100, 0.0, 0.0, 0.0);

    TEST_ASSERT_EQUAL(State::PLAY, gs.currentState);
    TEST_ASSERT_TRUE(std::isnan(gs.ballSpeedX) || std::isinf(gs.ballSpeedX));
    TEST_ASSERT_TRUE(std::isnan(gs.ballSpeedY) || std::isinf(gs.ballSpeedY));
}

void test_shoot_negative_values(void) {
    GameState gs;
    gs.currentBallSpeed = 15.0;

    // dx = -3, dy = 4, dist = 5
    gs.shoot(50, -3.0, 4.0, 5.0);

    TEST_ASSERT_EQUAL_FLOAT(-9.0, gs.ballSpeedX); // (-3/5) * 15 = -9
    TEST_ASSERT_EQUAL_FLOAT(12.0, gs.ballSpeedY); // (4/5) * 15 = 12
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_shoot_state_change);
    RUN_TEST(test_shoot_speed_calculation);
    RUN_TEST(test_shoot_zero_distance);
    RUN_TEST(test_shoot_negative_values);
    return UNITY_END();
}
