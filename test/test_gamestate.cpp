#include <unity.h>
#include "GameState.h"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_resetBall_lives_greater_than_zero(void) {
    GameState state;
    state.lives = 3;
    state.resetBall(240, 320);

    TEST_ASSERT_EQUAL_FLOAT(120.0, state.ballX);
    TEST_ASSERT_EQUAL_FLOAT(280.0, state.ballY);
    TEST_ASSERT_EQUAL_FLOAT(state.ballBaseRadius, state.ballRadius);
    TEST_ASSERT_EQUAL(State::START, state.currentState);
}

void test_resetBall_lives_zero(void) {
    GameState state;
    state.lives = 0;
    state.resetBall(240, 320);

    TEST_ASSERT_EQUAL_FLOAT(120.0, state.ballX);
    TEST_ASSERT_EQUAL_FLOAT(280.0, state.ballY);
    TEST_ASSERT_EQUAL_FLOAT(state.ballBaseRadius, state.ballRadius);
    TEST_ASSERT_EQUAL(State::GAMEOVER, state.currentState);
}

void test_resetBall_lives_less_than_zero(void) {
    GameState state;
    state.lives = -1;
    state.resetBall(240, 320);

    TEST_ASSERT_EQUAL_FLOAT(120.0, state.ballX);
    TEST_ASSERT_EQUAL_FLOAT(280.0, state.ballY);
    TEST_ASSERT_EQUAL_FLOAT(state.ballBaseRadius, state.ballRadius);
    TEST_ASSERT_EQUAL(State::GAMEOVER, state.currentState);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_resetBall_lives_greater_than_zero);
    RUN_TEST(test_resetBall_lives_zero);
    RUN_TEST(test_resetBall_lives_less_than_zero);
    UNITY_END();

    return 0;
}
