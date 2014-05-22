/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>

#include <stdlib.h>
#include <tet_api.h>

#include <Ecore.h>

#include <dali/dali.h>

#include <dali-test-suite-utils.h>

#include <adaptors/tizen/internal/common/tilt-sensor-impl.h>

using namespace Dali;

static void Startup();
static void Cleanup();

extern "C" {
  void (*tet_startup)() = Startup;
  void (*tet_cleanup)() = Cleanup;
}

static void UtcDaliTiltSensorEnable();
static void UtcDaliTiltSensorDisable();
static void UtcDaliTiltSensorIsEnabled();
static void UtcDaliTiltSensorGetRoll();
static void UtcDaliTiltSensorGetPitch();
static void UtcDaliTiltSensorGetRotation();
static void UtcDaliTiltSensorSignalTilted();
static void UtcDaliTiltSensorSetUpdateFrequency();
static void UtcDaliTiltSensorGetUpdateFrequency();
static void UtcDaliTiltSensorSetRotationThreshold01();
static void UtcDaliTiltSensorSetRotationThreshold02();
static void UtcDaliTiltSensorGetRotationThreshold();

enum {
  POSITIVE_TC_IDX = 0x01,
  NEGATIVE_TC_IDX,
};

// Add test functionality for all APIs in the class (Positive and Negative)
extern "C" {
  struct tet_testlist tet_testlist[] = {
    { UtcDaliTiltSensorEnable, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorDisable, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorIsEnabled, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorGetRoll, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorGetPitch, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorGetRotation, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorSignalTilted, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorSetUpdateFrequency, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorGetUpdateFrequency, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorSetRotationThreshold01, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorSetRotationThreshold02, POSITIVE_TC_IDX },
    { UtcDaliTiltSensorGetRotationThreshold, POSITIVE_TC_IDX },
    { NULL, 0 }
  };
}

static const float ROTATION_EPSILON = 0.0001f;

/**
 * Helper to test whether timeout or tilt signal is received first
 */
struct SignalHelper : public ConnectionTracker
{
  SignalHelper()
  : mTiltSignalReceived( false ),
    mTimeoutOccurred( false )
  {
  }

  void OnTilted(const TiltSensor& sensor)
  {
    tet_printf("tilted signal received\n");

    mTiltSignalReceived = true;

    // quit the main loop to continue test
    ecore_main_loop_quit();
  }

  bool OnTimeout()
  {
    tet_printf("timeout occurred\n");

    mTimeoutOccurred = true;

    // quit the main loop to continue test
    ecore_main_loop_quit();

    return false;
  }

  bool mTiltSignalReceived; // True if tilted signal was received
  bool mTimeoutOccurred;    // True if timeout occured
};

// Called only once before first test is run.
static void Startup()
{
  ecore_init();
}

// Called only once after last test is run
static void Cleanup()
{
  ecore_shutdown();
}

TiltSensor GetTiltSensor()
{
  return Internal::Adaptor::TiltSensor::New();
}

static void UtcDaliTiltSensorEnable()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorEnable");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );

  sensor.Enable();
  DALI_TEST_CHECK( sensor.IsEnabled() );
}

static void UtcDaliTiltSensorDisable()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorDisable");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );

  sensor.Enable();
  DALI_TEST_CHECK( sensor.IsEnabled() );

  sensor.Disable();
  DALI_TEST_CHECK( !sensor.IsEnabled() );
}

static void UtcDaliTiltSensorIsEnabled()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorIsEnabled");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );

  // Should be disabled by default
  DALI_TEST_CHECK( !sensor.IsEnabled() );
}

static void UtcDaliTiltSensorGetRoll()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorGetRoll");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );

  float roll = sensor.GetRoll();
  DALI_TEST_CHECK( roll <= 1.0f && roll >= -1.0f ); // range check
}

static void UtcDaliTiltSensorGetPitch()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorGetPitch");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );

  float pitch = sensor.GetPitch();
  DALI_TEST_CHECK( pitch <= 1.0f && pitch >= -1.0f ); // range check
}

static void UtcDaliTiltSensorGetRotation()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorGetRotation");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );

  Quaternion rotation = sensor.GetRotation();

  float roll  = sensor.GetRoll();
  float pitch = sensor.GetPitch();

  Quaternion expectedRotation = Quaternion( roll  * Math::PI * -0.5f, Vector3::YAXIS ) *
                                Quaternion( pitch * Math::PI * -0.5f, Vector3::XAXIS );

  DALI_TEST_EQUALS( rotation, expectedRotation, ROTATION_EPSILON, TEST_LOCATION );
}

static void UtcDaliTiltSensorSignalTilted()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorSignalTilted");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );
  sensor.Enable();

  Radian angle(Degree(-45));
  //Setting a negative threshold for testing purpose
  sensor.SetRotationThreshold( angle );

  Timer timer = Timer::New( 500/* 1/2 second i.e. tilted signal should occur first */ );
  DALI_TEST_CHECK( timer );
  timer.Start();

  SignalHelper helper;
  timer.TickSignal().Connect( &helper, &SignalHelper::OnTimeout );
  sensor.TiltedSignal().Connect( &helper, &SignalHelper::OnTilted );

  ecore_main_loop_begin();

  DALI_TEST_CHECK( helper.mTiltSignalReceived );
  DALI_TEST_CHECK( !helper.mTimeoutOccurred );
}

static void UtcDaliTiltSensorSetUpdateFrequency()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorSetUpdateFrequency");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );
  sensor.Enable();

  sensor.SetUpdateFrequency( 1.0f/*hertz*/ );

  Timer timer = Timer::New( 500/* 1/2 second i.e. timeout should occur first */ );
  DALI_TEST_CHECK( timer );
  timer.Start();

  SignalHelper helper;
  timer.TickSignal().Connect( &helper, &SignalHelper::OnTimeout );
  sensor.TiltedSignal().Connect( &helper, &SignalHelper::OnTilted );

  ecore_main_loop_begin();

  DALI_TEST_CHECK( !helper.mTiltSignalReceived );
  DALI_TEST_CHECK( helper.mTimeoutOccurred );
}

static void UtcDaliTiltSensorGetUpdateFrequency()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorGetUpdateFrequency");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );

  DALI_TEST_EQUALS( sensor.GetUpdateFrequency(), 60.0f, TEST_LOCATION );

  sensor.SetUpdateFrequency( 30.0f/*hertz*/ );
  DALI_TEST_EQUALS( sensor.GetUpdateFrequency(), 30.0f, TEST_LOCATION );
}

static void UtcDaliTiltSensorSetRotationThreshold01()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorSetRotationThreshold01");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );
  sensor.Enable();

  Radian angle(Degree(-45));
  //Setting a negative threshold for testing purpose, to generate signal, as threshold is 0.0 by default
  sensor.SetRotationThreshold( angle );

  Timer timer = Timer::New( 500/* 1/2 second i.e. tilted signal should occur first */ );
  DALI_TEST_CHECK( timer );
  timer.Start();

  SignalHelper helper;
  timer.TickSignal().Connect( &helper, &SignalHelper::OnTimeout );
  sensor.TiltedSignal().Connect( &helper, &SignalHelper::OnTilted );

  ecore_main_loop_begin();

  DALI_TEST_CHECK( helper.mTiltSignalReceived );
  DALI_TEST_CHECK( !helper.mTimeoutOccurred );
}

static void UtcDaliTiltSensorSetRotationThreshold02()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorSetRotationThreshold02");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );
  sensor.Enable();

  //When pitch threshold is set, TiltedSignal should not be emitted
  sensor.SetRotationThreshold( Radian(Degree(45)) );

  Timer timer = Timer::New( 500/* 1/2 second i.e. tilted signal should occur first */ );
  DALI_TEST_CHECK( timer );
  timer.Start();

  SignalHelper helper;
  timer.TickSignal().Connect( &helper, &SignalHelper::OnTimeout );
  sensor.TiltedSignal().Connect( &helper, &SignalHelper::OnTilted );

  ecore_main_loop_begin();

  DALI_TEST_CHECK( !helper.mTiltSignalReceived );
  DALI_TEST_CHECK( helper.mTimeoutOccurred );
}

static void UtcDaliTiltSensorGetRotationThreshold()
{
  TestApplication application;

  tet_infoline("UtcDaliTiltSensorGetRotationThreshold");

  TiltSensor sensor = GetTiltSensor();
  DALI_TEST_CHECK( sensor );

  DALI_TEST_EQUALS( sensor.GetRotationThreshold(), 0.0f, TEST_LOCATION );

  sensor.SetRotationThreshold( Radian(Degree(45)) );
  DALI_TEST_EQUALS( sensor.GetRotationThreshold(), Radian(Degree(45)), TEST_LOCATION );
}
