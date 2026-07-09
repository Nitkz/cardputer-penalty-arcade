#include <unity.h>
#include <cstdlib>
#include <cmath>

// Mock Arduino functions used in Goalkeeper.h
#define random(x) (rand() % (x))
#define abs(x) (std::abs(x))

#include "Goalkeeper.h"

void test_register_player_shot_valid_lanes() {
    Goalkeeper gk;
    // Initial state: {1, 1, 1}

    gk.registerPlayerShot(0);
    TEST_ASSERT_EQUAL(2, gk.playerHabits[0]);

    gk.registerPlayerShot(1);
    TEST_ASSERT_EQUAL(2, gk.playerHabits[1]);

    gk.registerPlayerShot(2);
    TEST_ASSERT_EQUAL(2, gk.playerHabits[2]);
}

void test_register_player_shot_invalid_lanes() {
    Goalkeeper gk;
    // Initial state: {1, 1, 1}

    // Test negative lane
    gk.registerPlayerShot(-1);
    TEST_ASSERT_EQUAL(1, gk.playerHabits[0]);
    TEST_ASSERT_EQUAL(1, gk.playerHabits[1]);
    TEST_ASSERT_EQUAL(1, gk.playerHabits[2]);

    // Test out of bounds lane (>2)
    gk.registerPlayerShot(3);
    TEST_ASSERT_EQUAL(1, gk.playerHabits[0]);
    TEST_ASSERT_EQUAL(1, gk.playerHabits[1]);
    TEST_ASSERT_EQUAL(1, gk.playerHabits[2]);

    gk.registerPlayerShot(100);
    TEST_ASSERT_EQUAL(1, gk.playerHabits[0]);
    TEST_ASSERT_EQUAL(1, gk.playerHabits[1]);
    TEST_ASSERT_EQUAL(1, gk.playerHabits[2]);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_register_player_shot_valid_lanes);
    RUN_TEST(test_register_player_shot_invalid_lanes);

    return UNITY_END();
}
