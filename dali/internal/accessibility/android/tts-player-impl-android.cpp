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

// CLASS HEADER
#include <dali/internal/accessibility/android/tts-player-impl-android.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>

#include <memory>
namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#if defined(DEBUG_ENABLED)
Debug::Filter* TtsPlayerAndroid::gLogFilter = Debug::Filter::New(Debug::Concise, false, "LOG_TTS_PLAYER");
#endif

std::unique_ptr<TtsPlayerAndroid> TtsPlayerAndroid::New(Dali::TtsPlayer::Mode mode)
{
  return std::unique_ptr<TtsPlayerAndroid>(new TtsPlayerAndroid(mode));
}

TtsPlayerAndroid::TtsPlayerAndroid(Dali::TtsPlayer::Mode mode)
: mStateChangedSignal()
{
  DALI_LOG_ERROR("TTS is not implemented in UBUNTU profile.\n");
}

TtsPlayerAndroid::~TtsPlayerAndroid()
{
}

void TtsPlayerAndroid::Play(const std::string& text)
{
}

void TtsPlayerAndroid::Stop()
{
}

void TtsPlayerAndroid::Pause()
{
}

void TtsPlayerAndroid::Resume()
{
}

Dali::TtsPlayer::State TtsPlayerAndroid::GetState()
{
  return Dali::TtsPlayer::UNAVAILABLE;
}

Dali::TtsPlayer::StateChangedSignalType& TtsPlayerAndroid::StateChangedSignal()
{
  return mStateChangedSignal;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
