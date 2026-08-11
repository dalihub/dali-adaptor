/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
 */

#include <dali-test-suite-utils.h>
#include <dali/internal/window-system/windows/keyboard-repeat.h>
#include <limits>

using Dali::Internal::Adaptor::WindowsPlatform::IsNativeKeyRepeat;
using Dali::Internal::Adaptor::WindowsPlatform::KeyboardRepeatKey;
using Dali::Internal::Adaptor::WindowsPlatform::KeyboardRepeatMilliseconds;
using Dali::Internal::Adaptor::WindowsPlatform::KeyboardRepeatSettings;
using Dali::Internal::Adaptor::WindowsPlatform::KeyboardRepeatState;

void utc_dali_windows_keyboard_repeat_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_windows_keyboard_repeat_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliWindowsKeyboardRepeatSettingsFromSystem(void)
{
  KeyboardRepeatSettings settings;
  float                  rate  = 0.0f;
  float                  delay = 0.0f;

  settings.InitializeFromSystem(31u, 1u);
  settings.Get(rate, delay);

  DALI_TEST_EQUALS(rate, 1.0f / 30.0f, 0.000001f, TEST_LOCATION);
  DALI_TEST_EQUALS(delay, 0.5f, 0.000001f, TEST_LOCATION);

  settings.InitializeFromSystem(0u, 3u);
  settings.Get(rate, delay);

  DALI_TEST_EQUALS(rate, 0.4f, 0.000001f, TEST_LOCATION);
  DALI_TEST_EQUALS(delay, 1.0f, 0.000001f, TEST_LOCATION);

  END_TEST;
}

int UtcDaliWindowsKeyboardRepeatSettingsLocalRoundTrip(void)
{
  KeyboardRepeatSettings settings;
  float                  rate  = 0.0f;
  float                  delay = 0.0f;

  DALI_TEST_CHECK(settings.Set(0.137f, 0.421f));
  settings.Get(rate, delay);

  DALI_TEST_EQUALS(rate, 0.137f, 0.000001f, TEST_LOCATION);
  DALI_TEST_EQUALS(delay, 0.421f, 0.000001f, TEST_LOCATION);

  END_TEST;
}

int UtcDaliWindowsKeyboardRepeatSettingsRejectInvalid(void)
{
  KeyboardRepeatSettings settings;
  float                  rate  = 0.0f;
  float                  delay = 0.0f;

  DALI_TEST_CHECK(settings.Set(0.1f, 0.3f));
  DALI_TEST_CHECK(!settings.Set(0.0f, 0.3f));
  DALI_TEST_CHECK(!settings.Set(-0.1f, 0.3f));
  DALI_TEST_CHECK(!settings.Set(0.1f, -0.3f));
  DALI_TEST_CHECK(!settings.Set(std::numeric_limits<float>::infinity(), 0.3f));
  DALI_TEST_CHECK(!settings.Set(0.1f, std::numeric_limits<float>::quiet_NaN()));

  settings.Get(rate, delay);
  DALI_TEST_EQUALS(rate, 0.1f, 0.000001f, TEST_LOCATION);
  DALI_TEST_EQUALS(delay, 0.3f, 0.000001f, TEST_LOCATION);

  END_TEST;
}

int UtcDaliWindowsKeyboardRepeatStateSuppressesNativeRepeat(void)
{
  KeyboardRepeatState     state;
  const KeyboardRepeatKey key{0x0100u, 0x41u, static_cast<intptr_t>(0x001e0001u)};

  DALI_TEST_CHECK(!state.ShouldForwardNativeRepeat(0x41u));
  DALI_TEST_CHECK(IsNativeKeyRepeat(static_cast<intptr_t>(1u << 30u)));
  DALI_TEST_CHECK(!IsNativeKeyRepeat(key.nativeData));

  state.Start(key);
  DALI_TEST_CHECK(state.IsActive());
  DALI_TEST_EQUALS(state.GetKey().key, static_cast<uintptr_t>(0x41u), TEST_LOCATION);
  DALI_TEST_CHECK(!state.ShouldForwardNativeRepeat(0x41u));

  END_TEST;
}

int UtcDaliWindowsKeyboardRepeatStateFallsBackForMatchingKey(void)
{
  KeyboardRepeatState state;

  state.Start({0x0100u, 0x41u, static_cast<intptr_t>(0x001e0001u)});
  state.EnableNativeFallback();

  DALI_TEST_CHECK(state.ShouldForwardNativeRepeat(0x41u));
  DALI_TEST_CHECK(!state.ShouldForwardNativeRepeat(0x42u));

  state.Start({0x0100u, 0x42u, static_cast<intptr_t>(0x00300001u)});
  DALI_TEST_CHECK(!state.ShouldForwardNativeRepeat(0x42u));

  END_TEST;
}

int UtcDaliWindowsKeyboardRepeatStateStopsMatchingKey(void)
{
  KeyboardRepeatState state;

  state.Start({0x0100u, 0x41u, static_cast<intptr_t>(0x001e0001u)});
  DALI_TEST_CHECK(!state.Stop(0x42u));
  DALI_TEST_CHECK(state.IsActive());
  DALI_TEST_CHECK(state.Stop(0x41u));
  DALI_TEST_CHECK(!state.IsActive());
  DALI_TEST_CHECK(!state.ShouldForwardNativeRepeat(0x41u));

  state.Start({0x0100u, 0x43u, static_cast<intptr_t>(0x002e0001u)});
  state.Stop();
  DALI_TEST_CHECK(!state.IsActive());

  END_TEST;
}

int UtcDaliWindowsKeyboardRepeatMillisecondsP(void)
{
  DALI_TEST_EQUALS(KeyboardRepeatMilliseconds(0.0f), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(KeyboardRepeatMilliseconds(0.137f), 137u, TEST_LOCATION);
  DALI_TEST_EQUALS(KeyboardRepeatMilliseconds(0.1376f), 138u, TEST_LOCATION);

  END_TEST;
}
