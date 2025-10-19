#include <unity.h>

// Simple parser: expects "KEY=VALUE" and returns value
static String parseValue(const String &line) {
  int idx = line.indexOf('=');
  if (idx < 0) return String();
  return line.substring(idx + 1);
}

void test_parse_config() {
  String s = "MODE=AUTO";
  TEST_ASSERT_EQUAL_STRING("AUTO", parseValue(s).c_str());
  TEST_ASSERT_EQUAL_STRING("", parseValue(String("INVALID")).c_str());
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_parse_config);
  UNITY_END();
}

void loop() {}
