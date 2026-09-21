/*
File: test/test_power_limit/test_main.cpp
Version: 0.1.0
Date: 2026-09-21
Purpose: Native unit tests for vendor-neutral power-to-current and current-bound logic.
*/
#include <unity.h>

#include "../../src/core/PowerLimit.h"

void test_three_phase_power_to_current() {
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 6.0f, ga::currentFromPower(4140.0f, 3, 230.0f));
}

void test_power_zero_means_stop() {
  TEST_ASSERT_EQUAL_FLOAT(0.0f, ga::currentFromPower(0.0f, 3, 230.0f));
}

void test_current_below_minimum_becomes_stop() {
  TEST_ASSERT_EQUAL_FLOAT(0.0f, ga::normalizeCurrent(5.9f, 6.0f, 16.0f));
}

void test_current_above_maximum_is_clamped() {
  TEST_ASSERT_EQUAL_FLOAT(16.0f, ga::normalizeCurrent(25.0f, 6.0f, 16.0f));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_three_phase_power_to_current);
  RUN_TEST(test_power_zero_means_stop);
  RUN_TEST(test_current_below_minimum_becomes_stop);
  RUN_TEST(test_current_above_maximum_is_clamped);
  return UNITY_END();
}
