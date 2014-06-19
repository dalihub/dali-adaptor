#ifndef __DALI_HAPTIC_PLAYER_H__
#define __DALI_HAPTIC_PLAYER_H__

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

/**
 * @addtogroup CAPI_DALI_ADAPTOR_MODULE
 * @{
 */

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>

namespace Dali DALI_IMPORT_API
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class HapticPlayer;
}
}

/**
 * @brief Plays haptic effects.
 */
class HapticPlayer : public BaseHandle
{
public:

  /**
   * @brief Create an uninitialized handle.
   *
   * This can be initialized by calling HapticPlayer::Get().
   */
  HapticPlayer();

  /**
   * @brief Create an initialized handle to the HapticPlayer.
   *
   * @return A handle to a newly allocated Dali resource.
   */
  static HapticPlayer Get();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~HapticPlayer();

  /**
   * @brief Plays a monotone vibration.
   * @param[in]  duration  The duration of the vibration.
   */
  void PlayMonotone(unsigned int duration);

  /**
   * @brief Plays vibration in predefined patterns.
   * @param[in] filePath Path to the file containing the effect.
   */
  void PlayFile(const std::string filePath);

  /**
   * @brief Stops the currently playing vibration effects.
   */
  void Stop();

public: // Not intended for application developers

  /**
   * @brief This constructor is used by HapticPlayer::Get().
   * @param[in] hapticPlayer A pointer to the haptic player.
   */
  HapticPlayer( Internal::Adaptor::HapticPlayer* hapticPlayer );
};

} // namespace Dali

/**
 * @}
 */
#endif // __DALI_HAPTIC_PLAYER_H__
