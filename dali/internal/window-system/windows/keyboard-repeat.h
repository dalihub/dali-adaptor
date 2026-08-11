#ifndef DALI_INTERNAL_WINDOW_SYSTEM_WINDOWS_KEYBOARD_REPEAT_H
#define DALI_INTERNAL_WINDOW_SYSTEM_WINDOWS_KEYBOARD_REPEAT_H

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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace WindowsPlatform
{
/** Stores the process-local Windows keyboard repeat interval and delay. */
class KeyboardRepeatSettings
{
public:
  void InitializeFromSystem(uint32_t speed, uint32_t delaySetting)
  {
    const float frequency = 2.5f + std::clamp(speed, 0u, 31u) * 27.5f / 31.0f;
    mRate                 = 1.0f / frequency;
    mDelay                = 0.25f * (std::clamp(delaySetting, 0u, 3u) + 1u);
  }

  bool Set(float rate, float delay)
  {
    if(!std::isfinite(rate) || !std::isfinite(delay) || rate <= 0.0f || delay < 0.0f)
    {
      return false;
    }

    mRate  = rate;
    mDelay = delay;
    return true;
  }

  void Get(float& rate, float& delay) const
  {
    rate  = mRate;
    delay = mDelay;
  }

private:
  float mRate{1.0f / 30.0f};
  float mDelay{0.5f};
};

struct KeyboardRepeatKey
{
  uint32_t  message{0u};
  uintptr_t key{0u};
  intptr_t  nativeData{0};
};

constexpr bool IsNativeKeyRepeat(intptr_t nativeData)
{
  return (static_cast<uintptr_t>(nativeData) & (uintptr_t{1u} << 30u)) != 0u;
}

inline uint32_t KeyboardRepeatMilliseconds(float seconds)
{
  if(!std::isfinite(seconds) || seconds <= 0.0f)
  {
    return 1u;
  }

  const double milliseconds = static_cast<double>(seconds) * 1000.0;
  const double maximum      = static_cast<double>((std::numeric_limits<uint32_t>::max)());
  if(milliseconds >= maximum)
  {
    return (std::numeric_limits<uint32_t>::max)();
  }

  return (std::max)(1u, static_cast<uint32_t>(std::lround(milliseconds)));
}

/** Tracks which native key is currently repeated by the DALi-local timer. */
class KeyboardRepeatState
{
public:
  void Start(const KeyboardRepeatKey& key)
  {
    mKey            = key;
    mActive         = true;
    mNativeFallback = false;
  }

  bool Stop(uintptr_t key)
  {
    if(!mActive || mKey.key != key)
    {
      return false;
    }

    Stop();
    return true;
  }

  void Stop()
  {
    mActive         = false;
    mNativeFallback = false;
  }

  void EnableNativeFallback()
  {
    if(mActive)
    {
      mNativeFallback = true;
    }
  }

  bool ShouldForwardNativeRepeat(uintptr_t key) const
  {
    return mActive && mNativeFallback && mKey.key == key;
  }

  bool IsActive() const
  {
    return mActive;
  }

  const KeyboardRepeatKey& GetKey() const
  {
    return mKey;
  }

private:
  KeyboardRepeatKey mKey;
  bool              mActive{false};
  bool              mNativeFallback{false};
};

} // namespace WindowsPlatform
} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOW_SYSTEM_WINDOWS_KEYBOARD_REPEAT_H
