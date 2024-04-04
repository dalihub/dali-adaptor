#ifndef __DALI_TTS_PLAYER_H__
#define __DALI_TTS_PLAYER_H__

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class TtsPlayer;
}
}

/**
 * @brief The Text-to-speech (TTS) Player.
 * @SINCE_1_0.0
 */
class DALI_ADAPTOR_API TtsPlayer : public BaseHandle
{
public: // ENUMs

  /**
   * @brief Enumeration for the instance of TTS mode.
   * @SINCE_1_0.0
   */
  enum Mode
  {
    DEFAULT = 0,  ///< Default mode for normal application @SINCE_1_0.0
    NOTIFICATION, ///< Notification mode, such as playing utterance is started or completed @SINCE_1_0.0
    SCREEN_READER, ///< Screen reader mode. To help visually impaired users interact with their devices, screen reader reads text or graphic elements on the screen using the TTS engine. @SINCE_1_0.0
    MODE_NUM
  };

  /**
   * @brief Enumeration for the instance of TTS state.
   * @SINCE_1_0.0
   */
  enum State
  {
    UNAVAILABLE = 0,    ///< Player is not available @SINCE_1_0.0
    READY,              ///< Player is ready to play @SINCE_1_0.0
    PLAYING,            ///< Player is playing @SINCE_1_0.0
    PAUSED              ///< Player is paused @SINCE_1_0.0
  };

public: // Typedefs

  /**
   * @brief Type of signal emitted when the TTS state changes.
   * @SINCE_1_0.0
   */
  typedef Signal< void ( const Dali::TtsPlayer::State, const Dali::TtsPlayer::State ) > StateChangedSignalType;

public: // API

  /**
   * @brief Creates an uninitialized handle.
   *
   * This can be initialized by calling TtsPlayer::Get().
   * @SINCE_1_0.0
   */
  TtsPlayer();

  /**
   * @brief Gets the singleton of the TtsPlayer for the given mode.
   *
   * @SINCE_1_0.0
   * @param mode The mode of tts-player
   * @return A handle of the Ttsplayer for the given mode
   */
  static TtsPlayer Get(Dali::TtsPlayer::Mode mode = Dali::TtsPlayer::DEFAULT);

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @SINCE_1_0.0
   */
  ~TtsPlayer();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @SINCE_1_0.0
   * @param [in] handle A reference to the copied handle
   */
  TtsPlayer(const TtsPlayer& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @SINCE_1_0.0
   * @param [in] rhs A reference to the copied handle
   * @return A reference to this
   */
  TtsPlayer& operator=(const TtsPlayer& rhs);

  /**
   * @brief Starts playing the audio data synthesized from the specified text.
   *
   * @SINCE_1_0.0
   * @param[in] text The text to play
   * @pre The TtsPlayer needs to be initialized.
   */
  void Play(const std::string& text);

  /**
   * @brief Stops playing the utterance.
   * @SINCE_1_0.0
   * @pre The TtsPlayer needs to be initialized.
   */
  void Stop();

  /**
   * @brief Pauses the currently playing utterance.
   * @SINCE_1_0.0
   * @pre The TtsPlayer needs to be initialized.
   */
  void Pause();

  /**
   * @brief Resumes the previously paused utterance.
   * @SINCE_1_0.0
   * @pre The TtsPlayer needs to be initialized.
   */
  void Resume();

  /**
   * @brief Gets the current state of the player.
   * @SINCE_1_0.0
   * @return The current TTS state
   * @pre The TtsPlayer needs to be initialized.
   */
  State GetState();

  /**
   * @brief Allows connection TTS state change signal.
   * @SINCE_1_0.0
   * @return A reference to the signal for connection
   * @note Only supported by some adaptor types.
   */
  Dali::TtsPlayer::StateChangedSignalType& StateChangedSignal();

public: // Not intended for application developers

  /// @cond internal
  /**
   * @brief This constructor is used by TtsPlayer::Get().
   * @SINCE_1_0.0
   * @param[in] ttsPlayer A pointer to the TTS player
   */
  explicit DALI_INTERNAL TtsPlayer( Internal::Adaptor::TtsPlayer* ttsPlayer );
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_TTS_PLAYER_H__
