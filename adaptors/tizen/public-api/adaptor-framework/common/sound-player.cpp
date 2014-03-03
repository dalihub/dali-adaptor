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
#include <dali/public-api/adaptor-framework/common/sound-player.h>

// INTERNAL INCLUDES
#include <internal/common/sound-player-impl.h>

namespace Dali
{

const char* const SoundPlayer::SIGNAL_SOUND_PLAY_FINISHED = "sound-play-finished";

SoundPlayer::SoundPlayer()
{
}

SoundPlayer SoundPlayer::Get()
{
  return Internal::Adaptor::SoundPlayer::Get();
}

SoundPlayer::~SoundPlayer()
{
}

int SoundPlayer::PlaySound(const std::string fileName)
{
  return GetImplementation(*this).PlaySound(fileName);
}

void SoundPlayer::Stop(int handle)
{
  GetImplementation(*this).Stop(handle);
}

SoundPlayer::SoundPlayFinishedSignalV2& SoundPlayer::SoundPlayFinishedSignal()
{
  return GetImplementation(*this).SoundPlayFinishedSignal();
}

SoundPlayer::SoundPlayer( Internal::Adaptor::SoundPlayer* player )
: BaseHandle( player )
{
}

} // namespace Dali
