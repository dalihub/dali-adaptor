#ifndef DALI_INTERNAL_ACCESSIBILITY_TIZEN_TTS_PLAYER_IMPL_TIZEN_H
#define DALI_INTERNAL_ACCESSIBILITY_TIZEN_TTS_PLAYER_IMPL_TIZEN_H

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
#include <tts.h>
#include <string>
#include <memory>

#include <dali/integration-api/debug.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/tts-player.h>
#include <dali/internal/accessibility/common/tts-player-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Text-to-speech player
 */
class TtsPlayerTizen : public Dali::Internal::Adaptor::TtsPlayer
{

public:

  /**
   * Create a TtsPlayer with the given mode.
   * This should only be called once by the Adaptor class for each given mode.
   * @param mode the mode of tts-player
   * @return A newly created TtsPlayer.
   */
  static std::unique_ptr<TtsPlayerTizen> New(Dali::TtsPlayer::Mode mode);

  /**
   * @copydoc TtsPlayer::Play()
   */
  void Play(const std::string& text) override;

  /**
   * @copydoc TtsPlayer::Stop()
   */
  void Stop() override;

  /**
   * @copydoc TtsPlayer::Pause()
   */
  void Pause() override;

  /**
   * @copydoc TtsPlayer::Resume()
   */
  void Resume() override;

  /**
   * @copydoc TtsPlayer::GetState()
   */
  Dali::TtsPlayer::State GetState() override;

  /**
   * @copydoc TtsPlayer::StateChangedSignal()
   */
  Dali::TtsPlayer::StateChangedSignalType& StateChangedSignal() override;

  /**
   * Destructor
   */
  ~TtsPlayerTizen() override;

private:

  /**
   * Private Constructor; see also TtsPlayer::New()
   * @param mode the mode of tts-player
   */
  TtsPlayerTizen(Dali::TtsPlayer::Mode mode);

  /**
   * Initializes the player.
   */
  void Initialize();

  /**
   * Logs the error code.
   * @param[in] reason The error code
   */
  void LogErrorCode(tts_error_e reason);

  /**
   * Used to emit the state changed signal from outside the object (EG. A static function).
   * @param[in] previous The previous state
   * @param[in] current The current state
   */
  void EmitStateChangedSignal( tts_state_e previous, tts_state_e current );

  /**
   * Called when the state of TTS is changed.
   *
   * @param[in] tts The handle for TTS
   * @param[in] previous A previous state
   * @param[in] current A current state
   * @param[in] userData The user data passed from the callback registration function.
   */
  static void StateChangedCallback(tts_h tts, tts_state_e previous, tts_state_e current, void *userData);

  /**
   * Called when the utterance read by TTS has been completed.
   *
   * @param[in] tts The handle for TTS
   * @param[in] ID of the completed utterance
   * @param[in] userData The user data passed from the callback registration function.
   */
  static void UtteranceCompletedCallback(tts_h tts, int utteranceId, void *userData);

  // Undefined
  TtsPlayerTizen(const TtsPlayerTizen&);

  // Undefined
  TtsPlayerTizen& operator=(TtsPlayerTizen&);

private:

  Dali::TtsPlayer::StateChangedSignalType mStateChangedSignal; ///< Signal emitted when the TTS state changes
  bool mInitialized; ///< Whether the TTS player is initialised successfully or not
  std::string mUnplayedString; ///< The text that can not be played because tts engine is not yet initialized
  tts_h mTtsHandle;  ///< The handle of TTS
  int mUtteranceId;  ///< The utterance ID

  Dali::TtsPlayer::Mode mTtsMode; ///< The current mode of tts engine

#if defined(DEBUG_ENABLED)
public:
  static Debug::Filter* gLogFilter;
#endif
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ACCESSIBILITY_TIZEN_TTS_PLAYER_IMPL_TIZEN_H
