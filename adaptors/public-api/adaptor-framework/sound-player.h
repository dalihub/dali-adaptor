#ifndef __DALI_SOUND_PLAYER_H__
#define __DALI_SOUND_PLAYER_H__

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
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal-v2.h>

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class SoundPlayer;
}
}

/**
 * @brief Plays sound effects.
 */
class DALI_IMPORT_API SoundPlayer : public BaseHandle
{
public:

  typedef SignalV2< void (SoundPlayer&) > SoundPlayFinishedSignalV2; ///< Sound play finished signal

  // Signal Names
  static const char* const SIGNAL_SOUND_PLAY_FINISHED; ///< name "sound-play-finished"

public:

  /**
   * @brief Create an uninitialized handle.
   *
   * This can be initialized by calling SoundPlayer::Get().
   */
  SoundPlayer();

  /**
   * @brief Create an initialized handle to the SoundPlayer.
   *
   * @return A handle to a newly allocated Dali resource.
   */
  static SoundPlayer Get();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~SoundPlayer();

  /**
   * @brief Plays a sound file.
   *
   * @pre The SoundPlayer needs to be initialized.
   * @param[in]  fileName Path to the sound file to play.
   * @return a handle to the currently playing sound file which can be used to stop.
   */
  int PlaySound(const std::string fileName);

  /**
   * @brief Stops the currently playing sound.
   *
   * @pre The SoundPlayer needs to be initialized.
   * @param[in] handle
   */
  void Stop(int handle);

  /**
   * @brief This signal will be emitted after a given sound file is completely played.
   *
   * @pre The SoundPlayer needs to be initialized.
   * @return The signal to connect to.
   */
  SoundPlayFinishedSignalV2& SoundPlayFinishedSignal();

public: // Not intended for application developers

  /**
   * @brief This constructor is used by SoundPlayer::Get().
   *
   * @param[in] soundPlayer A pointer to the sound player.
   */
  explicit DALI_INTERNAL SoundPlayer( Internal::Adaptor::SoundPlayer* soundPlayer );
};

} // namespace Dali

#endif // __DALI_SOUND_PLAYER_H__
