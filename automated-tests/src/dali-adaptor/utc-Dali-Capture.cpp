/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

#include <adaptor-test-application.h>
#include <dali-test-suite-utils.h>
#include <dali/public-api/capture/capture.h>

using namespace Dali;

void utc_dali_capture_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_capture_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliCaptureNewPositive(void)
{
  Dali::Capture instance = Dali::Capture::New();
  DALI_TEST_CHECK(instance);
  END_TEST;
}

int UtcDaliCaptureNew2Positive(void)
{
  CameraActor   cameraActor;
  Dali::Capture instance = Dali::Capture::New(cameraActor);
  DALI_TEST_CHECK(instance);
  END_TEST;
}

int UtcDaliCaptureDownCast(void)
{
  Dali::Capture instance;
  DALI_TEST_CHECK(!Capture::DownCast(instance));
  instance = Dali::Capture::New();
  DALI_TEST_CHECK(instance);
  DALI_TEST_CHECK(Capture::DownCast(instance));
  END_TEST;
}

int UtcDaliCaptureConstructorsPositive(void)
{
  Capture capture1 = Capture::New();

  // copy constructor
  Capture capture2 = Capture(capture1);
  DALI_TEST_CHECK(capture1 == capture2);

  capture1.Reset();
  DALI_TEST_CHECK(!capture1);

  // copy assignment
  capture1 = capture2;

  capture2.Reset();
  DALI_TEST_CHECK(capture1);
  DALI_TEST_CHECK(!capture2);

  // move constructor
  Capture capture3 = Capture(std::move(capture1));
  DALI_TEST_CHECK(capture3);

  // move assignemnt
  DALI_TEST_CHECK(!capture2);
  capture2 = std::move(capture3);
  DALI_TEST_CHECK(capture2);

  END_TEST;
}

int UtcDaliCaptureFinishedSignalNegative(void)
{
  Dali::Capture instance;
  try
  {
    instance.FinishedSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliCaptureSetImageQualityNegative(void)
{
  Dali::Capture instance;
  try
  {
    unsigned int arg1(0u);
    instance.SetImageQuality(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliCaptureStartNegative(void)
{
  Dali::Capture instance;
  try
  {
    Dali::Actor   arg1;
    Dali::Vector2 arg2;
    std::string   arg3;
    instance.Start(arg1, arg2, arg3);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliCaptureStart2Negative(void)
{
  Dali::Capture instance;
  try
  {
    Dali::Actor   arg1;
    Dali::Vector2 arg2;
    std::string   arg3;
    Dali::Vector4 arg4;
    instance.Start(arg1, arg2, arg3, arg4);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliCaptureStart3Negative(void)
{
  Dali::Capture instance;
  try
  {
    Dali::Actor   arg1;
    Dali::Vector2 arg2;
    std::string   arg3;
    Dali::Vector4 arg4;
    unsigned int  arg5(0u);
    instance.Start(arg1, arg2, arg3, arg4, arg5);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliCaptureStart4Negative(void)
{
  Dali::Capture instance;
  try
  {
    Dali::Actor   arg1;
    Dali::Vector2 arg2;
    Dali::Vector2 arg3;
    std::string   arg4;
    Dali::Vector4 arg5;
    instance.Start(arg1, arg2, arg3, arg4, arg5);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliCaptureGetNativeImageSourceNegative(void)
{
  Dali::Capture instance;
  try
  {
    instance.GetNativeImageSource();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}
