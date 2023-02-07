// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_test_definitions.h"
#include <azure/core/az_platform.h>
#include <azure/core/az_result.h>

#include <az_test_precondition.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>


static void _test_az_platform_timer_callback(void* sdk_data)
{
  (void)sdk_data;
}

// Will record the time at which the callback was called
static void _test_az_platform_timer_callback_set_time(void* sdk_data)
{
  assert_int_equal(az_platform_clock_msec((int64_t*) sdk_data), AZ_OK);
}

#ifndef AZ_NO_PRECONDITION_CHECKING
ENABLE_PRECONDITION_CHECK_TESTS()
#endif // AZ_NO_PRECONDITION_CHECKING

// Checking for preconditions
#ifndef AZ_NO_PRECONDITION_CHECKING
static void test_az_platform_clock_msec_null(void** state)
{
  SETUP_PRECONDITION_CHECK_TESTS();
  (void)state;

  ASSERT_PRECONDITION_CHECKED(az_platform_clock_msec(NULL));
}

static void test_az_platform_get_random_null(void** state)
{
  SETUP_PRECONDITION_CHECK_TESTS();
  (void)state;

  ASSERT_PRECONDITION_CHECKED(az_platform_get_random(NULL));
}

static void test_az_platform_timer_create_null(void** state)
{
  SETUP_PRECONDITION_CHECK_TESTS();
  (void)state;

  ASSERT_PRECONDITION_CHECKED(
      az_platform_timer_create(NULL, _test_az_platform_timer_callback, NULL));
}

static void test_az_platform_timer_start_null(void** state)
{
  SETUP_PRECONDITION_CHECK_TESTS();
  (void)state;

  ASSERT_PRECONDITION_CHECKED(az_platform_timer_start(NULL, 0));
}

static void test_az_platform_timer_destroy_null(void** state)
{
  SETUP_PRECONDITION_CHECK_TESTS();
  (void)state;

  ASSERT_PRECONDITION_CHECKED(az_platform_timer_destroy(NULL));
}

static void test_az_platform_mutex_init_null(void** state)
{
  SETUP_PRECONDITION_CHECK_TESTS();
  (void)state;

  ASSERT_PRECONDITION_CHECKED(az_platform_mutex_init(NULL));
}

static void test_az_platform_mutex_acquire_null(void** state)
{
  SETUP_PRECONDITION_CHECK_TESTS();
  (void)state;

  ASSERT_PRECONDITION_CHECKED(az_platform_mutex_acquire(NULL));
}

static void test_az_platform_mutex_release_null(void** state)
{
  SETUP_PRECONDITION_CHECK_TESTS();
  (void)state;

  ASSERT_PRECONDITION_CHECKED(az_platform_mutex_release(NULL));
}

static void test_az_platform_mutex_destroy_null(void** state)
{
  SETUP_PRECONDITION_CHECK_TESTS();
  (void)state;

  ASSERT_PRECONDITION_CHECKED(az_platform_mutex_destroy(NULL));
}

#endif // AZ_NO_PRECONDITION_CHECKING

static void test_az_platform_clock_msec_once(void** state)
{
  (void)state;

  int64_t test_clock = 0;

  assert_int_equal(az_platform_clock_msec(&test_clock), AZ_OK);
  assert_true(test_clock >= 0);
}

static void test_az_platform_sleep_msec_one_second(void** state)
{
  (void)state;

  int64_t test_clock_one = 0;
  int64_t test_clock_two = 0;
  int64_t test_expected_time = 1000;

  assert_int_equal(az_platform_clock_msec(&test_clock_one), AZ_OK);
  assert_int_equal(az_platform_sleep_msec(1000), AZ_OK); // One second
  assert_int_equal(az_platform_clock_msec(&test_clock_two), AZ_OK);

  int64_t test_actual_time = test_clock_two - test_clock_one;
  
  assert_true(test_actual_time >= test_expected_time);
}

static void test_az_platform_get_random_once(void** state)
{
  (void)state;

  int32_t test_random = 0;

  assert_int_equal(az_platform_get_random(&test_random), AZ_OK);
  assert_true(test_random > 0);
}

static void test_az_platform_timer_one_sec(void** state)
{
  (void)state;

  az_platform_timer test_timer_handle;
  int64_t test_clock_one = 0;
  int64_t test_clock_two = 0;
  int64_t test_expected_time = 1000;

  assert_int_equal(az_platform_timer_create(
        &test_timer_handle,
        _test_az_platform_timer_callback_set_time,
        &test_clock_two),
        AZ_OK);
  assert_int_equal(az_platform_clock_msec(&test_clock_one), AZ_OK);
  assert_int_equal(az_platform_timer_start(&test_timer_handle, 1000), AZ_OK);
  assert_int_equal(az_platform_sleep_msec(2000), AZ_OK);

  int64_t test_actual_time = test_clock_two - test_clock_one;

  assert_true(test_actual_time >= test_expected_time);

  assert_int_equal(az_platform_timer_destroy(&test_timer_handle), AZ_OK);
}

static void test_az_platform_mutex(void** state)
{
  (void)state;

  az_platform_mutex test_mutex_handle;

  assert_int_equal(az_platform_mutex_init(&test_mutex_handle), AZ_OK);
  assert_int_equal(az_platform_mutex_acquire(&test_mutex_handle), AZ_OK);
  assert_int_equal(az_platform_mutex_release(&test_mutex_handle), AZ_OK);
  assert_int_equal(az_platform_mutex_destroy(&test_mutex_handle), AZ_OK);
}

int test_az_platform()
{
  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(test_az_platform_clock_msec_null),
    cmocka_unit_test(test_az_platform_get_random_null),
    cmocka_unit_test(test_az_platform_timer_create_null),
    cmocka_unit_test(test_az_platform_timer_start_null),
    cmocka_unit_test(test_az_platform_timer_destroy_null),
    cmocka_unit_test(test_az_platform_mutex_init_null),
    cmocka_unit_test(test_az_platform_mutex_acquire_null),
    cmocka_unit_test(test_az_platform_mutex_release_null),
    cmocka_unit_test(test_az_platform_mutex_destroy_null),
#endif
    cmocka_unit_test(test_az_platform_clock_msec_once),
    cmocka_unit_test(test_az_platform_sleep_msec_one_second),
    cmocka_unit_test(test_az_platform_get_random_once),
    cmocka_unit_test(test_az_platform_timer_one_sec),
    cmocka_unit_test(test_az_platform_mutex)
  };
  return cmocka_run_group_tests_name("az_platform", tests, NULL, NULL);
}
