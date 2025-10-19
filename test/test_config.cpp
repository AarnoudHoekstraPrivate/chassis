#include <unity.h>
#if defined(__has_include)
#  if __has_include(<Arduino.h>)
#    include <Arduino.h>
#  elif __has_include(<arduino.h>)
#    include <arduino.h>
#  endif
#else
#  include <Arduino.h>
#endif

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

// setup()/loop() moved to test_runner.cpp to avoid multiple-definition errors
