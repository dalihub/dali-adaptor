#ifndef __DALI_TTS_PLAYER_H__
#define __DALI_TTS_PLAYER_H__

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

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class TtsPlayer;
}
}

/**
 * @brief The Text-to-speech Player.
 */
class DALI_IMPORT_API TtsPlayer : public BaseHandle
{
public: // ENUMs

  /**
   * @brief Enumeration of TTS mode.
   */
  enum Mode
  {
    DEFAULT = 0,  ///< Default mode for normal application
    NOTIFICATION, ///< Notification mode
    SCREEN_READER, ///< Screen reader mode
    MODE_NUM
  };

public: // API

  /**
   * @brief Create an uninitialized handle.
   *
   * This can be initialized by calling TtsPlayer::Get().
   */
  TtsPlayer();

  /**
   * @brief Gets the singleton of the TtsPlayer for each mode.
   *
   * Internally, each tts player handles (singleton instance) are managed for each mode.
   * @param mode the mode of tts-player
   * @return A handle of the Ttsplayer for given mode.
   */
  static TtsPlayer Get(Dali::TtsPlayer::Mode mode = Dali::TtsPlayer::DEFAULT);

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~TtsPlayer();

  /**
   * @brief Start playing the audio data synthesized from the specified text.
   *
   * @pre The TtsPlayer needs to be initialized.
   * @param[in] text to play.
   */
  void Play(const std::string& text);

  /**
   * @brief Stops playing the utterance.
   * @pre The TtsPlayer needs to be initialized.
   */
  void Stop();

  /**
   * @brief Pauses the currently playing utterance.
   * @pre The TtsPlayer needs to be initialized.
   */
  void Pause();

  /**
   * @brief Resumes the previously paused utterance.
   * @pre The TtsPlayer needs to be initialized.
   */
  void Resume();

public: // Not intended for application developers

  /**
   * @brief This constructor is used by TtsPlayer::Get().
   * @param[in] ttsPlayer A pointer to the TTS player.
   */
  explicit DALI_INTERNAL TtsPlayer( Internal::Adaptor::TtsPlayer* ttsPlayer );
};

} // namespace Dali

#endif // __DALI_TTS_PLAYER_H__
