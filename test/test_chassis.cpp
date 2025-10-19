#include <unity.h>

void test_placeholder() {
    TEST_ASSERT_EQUAL(1, 1);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_placeholder);
  UNITY_END();
}

void loop() {}
