//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include <dali/public-api/adaptor-framework/common/tts-player.h>

// INTERNAL INCLUDES
#include <internal/common/tts-player-impl.h>
#include <internal/common/adaptor-impl.h>

namespace Dali
{

TtsPlayer::TtsPlayer()
{
}

TtsPlayer TtsPlayer::Get(Dali::TtsPlayer::Mode mode)
{
  TtsPlayer ttsPlayer;

  if ( Adaptor::IsAvailable() )
  {
    ttsPlayer = Internal::Adaptor::Adaptor::GetImplementation( Adaptor::Get() ).GetTtsPlayer(mode);
  }

  return ttsPlayer;
}

TtsPlayer::~TtsPlayer()
{
}

void TtsPlayer::Play(const std::string& text)
{
  return GetImplementation(*this).Play(text);
}

void TtsPlayer::Stop()
{
  GetImplementation(*this).Stop();
}

void TtsPlayer::Pause()
{
  GetImplementation(*this).Pause();
}

void TtsPlayer::Resume()
{
  GetImplementation(*this).Resume();
}

TtsPlayer::TtsPlayer( Internal::Adaptor::TtsPlayer* player )
: BaseHandle( player )
{
}

} // namespace Dali
