#include <unity.h>

int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void test_clamp() {
  TEST_ASSERT_EQUAL(5, clampInt(3, 5, 10));
  TEST_ASSERT_EQUAL(10, clampInt(15, 5, 10));
  TEST_ASSERT_EQUAL(7, clampInt(7, 5, 10));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_clamp);
  UNITY_END();
}

void loop() {}
