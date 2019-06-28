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
#include <dali/internal/accessibility/generic/tts-player-impl-generic.h>

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
Debug::Filter* TtsPlayerGeneric::gLogFilter = Debug::Filter::New(Debug::Concise, false, "LOG_TTS_PLAYER");
#endif

std::unique_ptr<TtsPlayerGeneric> TtsPlayerGeneric::New(Dali::TtsPlayer::Mode mode)
{
  return std::unique_ptr<TtsPlayerGeneric>(new TtsPlayerGeneric(mode));
}

TtsPlayerGeneric::TtsPlayerGeneric(Dali::TtsPlayer::Mode mode)
: mStateChangedSignal()
{
  DALI_LOG_ERROR("TTS is not implemented in GENERIC profile.\n");
}

TtsPlayerGeneric::~TtsPlayerGeneric()
{
}

void TtsPlayerGeneric::Play(const std::string& text)
{
}

void TtsPlayerGeneric::Stop()
{
}

void TtsPlayerGeneric::Pause()
{
}

void TtsPlayerGeneric::Resume()
{
}

Dali::TtsPlayer::State TtsPlayerGeneric::GetState()
{
  return Dali::TtsPlayer::UNAVAILABLE;
}

Dali::TtsPlayer::StateChangedSignalType& TtsPlayerGeneric::StateChangedSignal()
{
  return mStateChangedSignal;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
