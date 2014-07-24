#ifndef __DALI_INTERNAL_HAPTIC_PLAYER_H__
#define __DALI_INTERNAL_HAPTIC_PLAYER_H__

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

// EXTERNAL INCLUDES
#include <string>
#include <dali/public-api/object/base-object.h>
#include <haptic-player.h>

// INTERNAL INCLUDES
#include <feedback/feedback-plugin-proxy.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class FeedbackPluginProxy;

/**
 * Plays haptic effects.
 */
class HapticPlayer : public Dali::BaseObject
{

public:

  /**
   * Create a HapticPlayer.
   * This should only be called once by the Adaptor class.
   * @return A newly created HapticPlayer.
   */
  static Dali::HapticPlayer New();

  /**
   * Retrieve a handle to the HapticPlayer. This creates an instance if none has been created.
   * @return A handle to the HapticPlayer.
   */
  static Dali::HapticPlayer Get();

  /**
   * @copydoc Dali::HapticPlayer::PlayMonotone()
   */
  void PlayMonotone(unsigned int duration);

  /**
   * @copydoc Dali::HapticPlayer::PlayFile()
   */
  void PlayFile( const std::string& filePath );

  /**
   * @copydoc Dali::HapticPlayer::Stop()
   */
  void Stop();

private:

  /**
   * Private Constructor; see also HapticPlayer::New()
   */
  HapticPlayer();

  /**
   * Virtual Destructor
   */
  virtual ~HapticPlayer();

  // Undefined
  HapticPlayer(const HapticPlayer&);

  // Undefined
  HapticPlayer& operator=(HapticPlayer&);

private:

  FeedbackPluginProxy mPlugin;
};

} // namespace Adaptor

} // namespace Internal

// Helpers for public-api forwarding methods

inline Internal::Adaptor::HapticPlayer& GetImplementation(Dali::HapticPlayer& player)
{
  DALI_ASSERT_ALWAYS( player && "HapticPlayer handle is empty" );

  BaseObject& handle = player.GetBaseObject();

  return static_cast<Internal::Adaptor::HapticPlayer&>(handle);
}

inline const Internal::Adaptor::HapticPlayer& GetImplementation(const Dali::HapticPlayer& player)
{
  DALI_ASSERT_ALWAYS( player && "HapticPlayer handle is empty" );

  const BaseObject& handle = player.GetBaseObject();

  return static_cast<const Internal::Adaptor::HapticPlayer&>(handle);
}

} // namespace Dali

#endif // __DALI_INTERNAL_HAPTIC_PLAYER_H__
