#ifndef DALI_INTERNAL_ACCESSIBILITY_GENERIC_TTS_PLAYER_IMPL_GENERIC_H
#define DALI_INTERNAL_ACCESSIBILITY_GENERIC_TTS_PLAYER_IMPL_GENERIC_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <memory>

#include <dali/integration-api/debug.h>
#include <dali/public-api/object/base-object.h>

#include <dali/internal/accessibility/common/tts-player-impl.h>

// INTERNAL INCLUDES

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Text-to-speech player
 */
class TtsPlayerGeneric : public Dali::Internal::Adaptor::TtsPlayer
{

public:

  /**
   * Create a TtsPlayer with the given mode.
   * This should only be called once by the Adaptor class for each given mode.
   * @param mode the mode of tts-player
   * @return A newly created TtsPlayer.
   */
  static std::unique_ptr<TtsPlayerGeneric> New(Dali::TtsPlayer::Mode mode);

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
   * Private Constructor; see also TtsPlayer::New()
   * @param mode the mode of tts-player
   */
  TtsPlayerGeneric(Dali::TtsPlayer::Mode mode);

  /**
   * Destructor
   */
  virtual ~TtsPlayerGeneric();

private:

  Dali::TtsPlayer::StateChangedSignalType mStateChangedSignal; ///< Signal emitted when the TTS state changes (non-functional, for interface compatibility).

#if defined(DEBUG_ENABLED)
public:
  static Debug::Filter* gLogFilter;
#endif
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ACCESSIBILITY_GENERIC_TTS_PLAYER_IMPL_GENERIC_H
