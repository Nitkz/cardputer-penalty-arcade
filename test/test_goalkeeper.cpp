#include <unity.h>
#include <math.h>

// Mock Arduino random functions required by Goalkeeper.h
long random(long max) { return 0; }
long random(long min, long max) { return min; }

#include "../src/Goalkeeper.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_collision_exact_overlap(void) {
    Goalkeeper gk;
    gk.x = 100.0f;
    // GK width is 70, meaning X bounds are [65, 135].
    // Ball at 100 with radius 5 is fully inside.
    TEST_ASSERT_TRUE(gk.checkCollision(100.0f, 5.0f));
}

void test_collision_touching_left_edge(void) {
    Goalkeeper gk;
    gk.x = 100.0f;
    // Left boundary is 65. Ball at 60, radius 5. Right edge of ball is 65.
    // Overlap condition is strictly greater than/less than, so touching returns false.
    TEST_ASSERT_FALSE(gk.checkCollision(60.0f, 5.0f));
}

void test_collision_overlapping_left_edge(void) {
    Goalkeeper gk;
    gk.x = 100.0f;
    // Left boundary is 65. Ball at 61, radius 5. Right edge of ball is 66.
    // 66 > 65 -> Overlaps.
    TEST_ASSERT_TRUE(gk.checkCollision(61.0f, 5.0f));
}

void test_collision_touching_right_edge(void) {
    Goalkeeper gk;
    gk.x = 100.0f;
    // Right boundary is 135. Ball at 140, radius 5. Left edge of ball is 135.
    // Overlap condition is strictly greater than/less than, so touching returns false.
    TEST_ASSERT_FALSE(gk.checkCollision(140.0f, 5.0f));
}

void test_collision_overlapping_right_edge(void) {
    Goalkeeper gk;
    gk.x = 100.0f;
    // Right boundary is 135. Ball at 139, radius 5. Left edge of ball is 134.
    // 134 < 135 -> Overlaps.
    TEST_ASSERT_TRUE(gk.checkCollision(139.0f, 5.0f));
}

void test_collision_far_left(void) {
    Goalkeeper gk;
    gk.x = 100.0f;
    // Far to the left, definitely false.
    TEST_ASSERT_FALSE(gk.checkCollision(0.0f, 5.0f));
}

void test_collision_far_right(void) {
    Goalkeeper gk;
    gk.x = 100.0f;
    // Far to the right, definitely false.
    TEST_ASSERT_FALSE(gk.checkCollision(200.0f, 5.0f));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_collision_exact_overlap);
    RUN_TEST(test_collision_touching_left_edge);
    RUN_TEST(test_collision_overlapping_left_edge);
    RUN_TEST(test_collision_touching_right_edge);
    RUN_TEST(test_collision_overlapping_right_edge);
    RUN_TEST(test_collision_far_left);
    RUN_TEST(test_collision_far_right);
    return UNITY_END();
}
