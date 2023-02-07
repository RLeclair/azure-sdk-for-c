// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_test_definitions.h"
#include <azure/core/internal/az_hfsm_pipeline_internal.h>
#include <azure/core/internal/az_hfsm_internal.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/az_result.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

static az_hfsm_pipeline az_hfsm_pipeline_test;

//test_policy_outbound <--> test_policy_middle <--> test_policy_inbound.
static az_hfsm_policy test_policy_outbound;
static az_hfsm_policy test_policy_middle;
static az_hfsm_policy test_policy_inbound;

typedef enum
{
  POST_OUTBOUND_0 = _az_HFSM_MAKE_EVENT(_az_FACILITY_HFSM, 5),
  POST_INBOUND_0 = _az_HFSM_MAKE_EVENT(_az_FACILITY_HFSM, 6),
  SEND_INBOUND_0 = _az_HFSM_MAKE_EVENT(_az_FACILITY_HFSM, 7),
  SEND_INBOUND_1 = _az_HFSM_MAKE_EVENT(_az_FACILITY_HFSM, 8),
  SEND_INBOUND_2 = _az_HFSM_MAKE_EVENT(_az_FACILITY_HFSM, 9),
  SEND_INBOUND_3 = _az_HFSM_MAKE_EVENT(_az_FACILITY_HFSM, 10),
  SEND_OUTBOUND_0 = _az_HFSM_MAKE_EVENT(_az_FACILITY_HFSM, 11),
  SEND_OUTBOUND_1 = _az_HFSM_MAKE_EVENT(_az_FACILITY_HFSM, 12)
} test_az_hfsm_pipeline_event_type;

static az_hfsm_event post_outbound_0_evt = { POST_OUTBOUND_0, NULL };
static az_hfsm_event post_inbound_0_evt = { POST_INBOUND_0, NULL };
static az_hfsm_event send_inbound_0_evt = { SEND_INBOUND_0, NULL };
static az_hfsm_event send_inbound_1_evt = { SEND_INBOUND_1, NULL };
static az_hfsm_event send_inbound_2_evt = { SEND_INBOUND_2, NULL };
static az_hfsm_event send_inbound_3_evt = { SEND_INBOUND_3, NULL };
static az_hfsm_event send_outbound_0_evt = { SEND_OUTBOUND_0, NULL };
static az_hfsm_event send_outbound_1_evt = { SEND_OUTBOUND_1, NULL };

static int ref_policy_01_root = 0;
static int ref_policy_02_root = 0;
static int ref_policy_03_root = 0;
static int timeout_0 = 0;
static int timeout_1 = 0;
#ifdef PIPELINE_SYNC
static int loopout_0 = 0;
static int loopin_0 = 0;
#endif
static int post_outbound_0 = 0;
static int post_inbound_0 = 0;
static int send_inbound_0 = 0;
static int send_inbound_1 = 0;
static int send_inbound_2 = 0;
static int send_inbound_3 = 0;
static int send_outbound_0 = 0;
static int send_outbound_1 = 0;

static az_result policy_01_root(az_hfsm* me, az_hfsm_event event)
{
  int32_t ret = AZ_OK;
  (void)me;

  switch (event.type)
  {
    case AZ_HFSM_EVENT_ENTRY:
      ref_policy_01_root++;
      break;

    case AZ_HFSM_EVENT_EXIT:
      ref_policy_01_root--;
      break;

    case AZ_HFSM_EVENT_TIMEOUT:
      timeout_0++;
      if (timeout_0 > 1)
      {
        // To test timeout failure.
        ret = AZ_ERROR_ARG;
      }
      break;

    #ifdef PIPELINE_SYNC
    case AZ_HFSM_PIPELINE_EVENT_PROCESS_LOOP:
      loopout_0++;
      break;
    #endif

    case POST_OUTBOUND_0:
      post_outbound_0++;
      break;

    case SEND_INBOUND_0:
      send_inbound_0++;
      ret = az_hfsm_pipeline_send_inbound_event((az_hfsm_policy*)me, send_inbound_1_evt);
      break;

    case SEND_INBOUND_2:
      send_inbound_2++;
      ret = az_hfsm_pipeline_send_inbound_event((az_hfsm_policy*)me, send_inbound_3_evt);
      break;

    default:
      ret = AZ_HFSM_RETURN_HANDLE_BY_SUPERSTATE;
      break;
  }

  return ret;
}

static az_result policy_02_root(az_hfsm* me, az_hfsm_event event)
{
  int32_t ret = AZ_OK;
  (void)me;

  switch (event.type)
  {
    case AZ_HFSM_EVENT_ENTRY:
      ref_policy_02_root++;
      break;

    case AZ_HFSM_EVENT_EXIT:
      ref_policy_02_root--;
      break;

    case AZ_HFSM_EVENT_ERROR:
      send_inbound_3++;
      break;

    case SEND_INBOUND_1:
      send_inbound_1++;
      break;

    case SEND_INBOUND_3:
      send_inbound_3++;
      ret = AZ_ERROR_ARG;
      break;

    case SEND_OUTBOUND_1:
      send_outbound_1++;
      break;

    default:
      ret = AZ_HFSM_RETURN_HANDLE_BY_SUPERSTATE;
      break;
  }

  return ret;
}

static az_result policy_03_root(az_hfsm* me, az_hfsm_event event)
{
  int32_t ret = AZ_OK;
  (void)me;

  switch (event.type)
  {
    case AZ_HFSM_EVENT_ENTRY:
      ref_policy_03_root++; 
      break;

    case AZ_HFSM_EVENT_EXIT:
      ref_policy_03_root--;
      break;

    #ifdef PIPELINE_SYNC
    case AZ_HFSM_PIPELINE_EVENT_PROCESS_LOOP:
      loopin_0++;
      break;
    #endif

    case AZ_HFSM_EVENT_ERROR:
      _az_PRECONDITION_NOT_NULL(event.data);
      az_hfsm_event_data_error* test_error = (az_hfsm_event_data_error*) event.data;
      if (test_error->error_type == AZ_ERROR_ARG)
      {
        timeout_1++;
      } else
      {
        ret = AZ_ERROR_NOT_IMPLEMENTED;
      }
      break;

    case POST_INBOUND_0:
      post_inbound_0++;
      break;

    case SEND_OUTBOUND_0:
      send_outbound_0++;
      ret = az_hfsm_pipeline_send_outbound_event((az_hfsm_policy*)me, send_outbound_1_evt);
      break;

    default:
      ret = AZ_HFSM_RETURN_HANDLE_BY_SUPERSTATE;
      break;
  }

  return ret;
}

static az_hfsm_state_handler policy_01_get_parent(az_hfsm_state_handler child_state)
{
  (void)child_state;

  return NULL;
}

static az_hfsm_state_handler policy_02_get_parent(az_hfsm_state_handler child_state)
{
  (void)child_state;

  return NULL;
}

static az_hfsm_state_handler policy_03_get_parent(az_hfsm_state_handler child_state)
{
  (void)child_state;
  
  return NULL;
}

static void test_az_hfsm_pipeline_init(void** state)
{
  (void)state;

  // Init test_policy_outbound policy
  assert_int_equal(
    az_hfsm_init((az_hfsm*) &test_policy_outbound, policy_01_root, policy_01_get_parent), AZ_OK);
  assert_true(test_policy_outbound.hfsm._internal.current_state == policy_01_root);
  assert_true(ref_policy_01_root == 1);

  // Init test_policy_middle policy
  assert_int_equal(
    az_hfsm_init((az_hfsm*) &test_policy_middle, policy_02_root, policy_02_get_parent), AZ_OK);
  assert_true(test_policy_middle.hfsm._internal.current_state == policy_02_root);
  assert_true(ref_policy_02_root == 1);

  // Init test_policy_inbound policy
  assert_int_equal(
    az_hfsm_init((az_hfsm*) &test_policy_inbound, policy_03_root, policy_03_get_parent), AZ_OK);
  assert_true(test_policy_inbound.hfsm._internal.current_state == policy_03_root);
  assert_true(ref_policy_03_root == 1);

  test_policy_outbound.inbound = &test_policy_middle;
  test_policy_middle.outbound = &test_policy_outbound;
  test_policy_middle.inbound = &test_policy_inbound;
  test_policy_inbound.outbound = &test_policy_middle;

  assert_int_equal(
    az_hfsm_pipeline_init(&az_hfsm_pipeline_test, &test_policy_outbound, &test_policy_inbound),
    AZ_OK);
}

static void test_az_hfsm_pipeline_post_outbound(void** state)
{
  (void)state;
  post_outbound_0 = 0;

  assert_int_equal(
    az_hfsm_pipeline_post_outbound_event(&az_hfsm_pipeline_test, post_outbound_0_evt), AZ_OK);
  assert_true(post_outbound_0 == 1);
}

static void test_az_hfsm_pipeline_post_inbound(void** state)
{
  (void)state;
  post_inbound_0 = 0;

  assert_int_equal(az_hfsm_pipeline_post_inbound_event(&az_hfsm_pipeline_test, post_inbound_0_evt),
   AZ_OK);
  assert_true(post_inbound_0 == 1);
}

static void test_az_hfsm_pipeline_send_inbound(void** state)
{
  (void)state;
  send_inbound_0 = 0;
  send_inbound_1 = 0;
  
  assert_int_equal(az_hfsm_pipeline_post_outbound_event(&az_hfsm_pipeline_test, send_inbound_0_evt),
   AZ_OK);
  assert_true(send_inbound_0 == 1);
  assert_true(send_inbound_1 == 1);
}

static void test_az_hfsm_pipeline_send_inbound_failure(void** state)
{
  (void)state;
  send_inbound_2 = 0;
  send_inbound_3 = 0;

  assert_int_equal(az_hfsm_pipeline_post_outbound_event(&az_hfsm_pipeline_test, send_inbound_2_evt),
   AZ_OK);
  assert_true(send_inbound_2 == 1);
  assert_true(send_inbound_3 == 2);
}

static void test_az_hsfm_pipeline_send_outbound(void** state)
{
  (void)state;
  send_outbound_0 = 0;
  send_outbound_1 = 0;

  assert_int_equal(az_hfsm_pipeline_post_inbound_event(&az_hfsm_pipeline_test, send_outbound_0_evt),
   AZ_OK);
  assert_true(send_outbound_0 == 1);
  assert_true(send_outbound_1 == 1);
}

static void test_az_hfsm_pipeline_timer_create_cb_success(void** state)
{
  (void)state;
  az_hfsm_pipeline_timer test_timer;
  timeout_0 = 0; // Reset timeout counter
  
  assert_int_equal(az_hfsm_pipeline_timer_create(&az_hfsm_pipeline_test, &test_timer), AZ_OK);
  assert_int_equal(az_platform_timer_start(&(test_timer.platform_timer), 0), AZ_OK);
  test_timer.platform_timer.callback(test_timer.platform_timer.sdk_data);
  assert_true(timeout_0 == 1);
}

static void test_az_hfsm_pipeline_timer_create_cb_failure(void** state)
{
  (void)state;
  az_hfsm_pipeline_timer test_timer;
  timeout_0 = 1; // Will cause failure on timeout
  timeout_1 = 0;

  assert_int_equal(az_hfsm_pipeline_timer_create(&az_hfsm_pipeline_test, &test_timer), AZ_OK);
  assert_int_equal(az_platform_timer_start(&(test_timer.platform_timer), 0), AZ_OK);
  test_timer.platform_timer.callback(test_timer.platform_timer.sdk_data);
  assert_true(timeout_0 == 2);
  assert_true(timeout_1 == 1);

}

#ifdef PIPELINE_SYNC
static void test_az_hfsm_pipeline_sync_process_loop(void** state)
{
  (void)state;
  loopout_0 = 0;
  loopin_0 = 0;

  assert_int_equal(az_hfsm_pipeline_sync_process_loop(&az_hfsm_pipeline_test), AZ_OK);
  assert_true(loopout_0 == 1);
  assert_true(loopin_0 == 1);
}
#endif

int test_az_hfsm_pipeline()
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_az_hfsm_pipeline_init),
    cmocka_unit_test(test_az_hfsm_pipeline_post_outbound),
    cmocka_unit_test(test_az_hfsm_pipeline_post_inbound),
    cmocka_unit_test(test_az_hfsm_pipeline_send_inbound),
    cmocka_unit_test(test_az_hfsm_pipeline_send_inbound_failure),
    cmocka_unit_test(test_az_hsfm_pipeline_send_outbound),
    cmocka_unit_test(test_az_hfsm_pipeline_timer_create_cb_success),
    cmocka_unit_test(test_az_hfsm_pipeline_timer_create_cb_failure),
    #ifdef PIPELINE_SYNC
    cmocka_unit_test(test_az_hfsm_pipeline_sync_process_loop),
    #endif
  };
  return cmocka_run_group_tests_name("az_core_hfsm_pipeline", tests, NULL, NULL);
}
