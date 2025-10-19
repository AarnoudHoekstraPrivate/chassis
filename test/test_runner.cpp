#include <unity.h>

// forward declarations for tests
void test_parse_config();
void test_placeholder();
void test_clamp();

extern "C" void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_parse_config);
  RUN_TEST(test_placeholder);
  RUN_TEST(test_clamp);
  UNITY_END();
}

extern "C" void loop() {}
