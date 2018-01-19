/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/internal/accessibility/common/tts-player-impl.h>
#include <dali/internal/accessibility/common/tts-player-factory.h>
namespace Dali
{

namespace Internal
{

namespace Adaptor
{

Dali::TtsPlayer TtsPlayer::New(Dali::TtsPlayer::Mode mode)
{
  return Dali::TtsPlayer(TtsPlayerFactory::New(mode).release());
}

void TtsPlayer::Play(const std::string &text)
{

}

void TtsPlayer::Stop()
{

}

void TtsPlayer::Pause()
{

}

void TtsPlayer::Resume()
{

}

Dali::TtsPlayer::State TtsPlayer::GetState()
{
  return Dali::TtsPlayer::State();
}

Dali::TtsPlayer::StateChangedSignalType& TtsPlayer::StateChangedSignal()
{
  static Dali::TtsPlayer::StateChangedSignalType signal;
  return signal;
}

}
}
}