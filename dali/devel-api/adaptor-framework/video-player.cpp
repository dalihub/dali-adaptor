/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/video-player.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>

// INTERNAL INCLUDES
#include <dali/internal/video/common/video-player-impl.h>

namespace Dali
{

VideoPlayer::VideoPlayer()
{
}

VideoPlayer::VideoPlayer( Internal::Adaptor::VideoPlayer* internal )
: BaseHandle( internal )
{
}

VideoPlayer::~VideoPlayer()
{
}

VideoPlayer VideoPlayer::New()
{
  Internal::Adaptor::VideoPlayerPtr player = Internal::Adaptor::VideoPlayer::New();

  if( player )
  {
    player->Initialize();
  }

  return VideoPlayer( player.Get() );
}

VideoPlayer::VideoPlayer( const VideoPlayer& player )
: BaseHandle( player )
{
}

VideoPlayer& VideoPlayer::operator=( const VideoPlayer& player )
{
  if( *this != player )
  {
    BaseHandle::operator=( player );
  }
  return *this;
}

VideoPlayer VideoPlayer::DownCast( BaseHandle handle )
{
  return VideoPlayer( dynamic_cast< Internal::Adaptor::VideoPlayer* >( handle.GetObjectPtr() ) );
}

void VideoPlayer::SetUrl( const std::string& url )
{
  GetImplementation( *this ).SetUrl( url );
}

std::string VideoPlayer::GetUrl()
{
  return GetImplementation( *this ).GetUrl();
}

void VideoPlayer::SetLooping(bool looping)
{
  GetImplementation( *this ).SetLooping( looping );
}

bool VideoPlayer::IsLooping()
{
  return GetImplementation( *this ).IsLooping();
}

void VideoPlayer::Play()
{
  GetImplementation( *this ).Play();
}

void VideoPlayer::Pause()
{
  GetImplementation( *this ).Pause();
}

void VideoPlayer::Stop()
{
  GetImplementation( *this ).Stop();
}

void VideoPlayer::SetMute( bool mute )
{
  GetImplementation( *this ).SetMute( mute );
}

bool VideoPlayer::IsMuted()
{
  return GetImplementation( *this ).IsMuted();
}

void VideoPlayer::SetVolume( float left, float right )
{
  GetImplementation( *this ).SetVolume( left, right );
}

void VideoPlayer::GetVolume( float& left, float& right )
{
  GetImplementation( *this ).GetVolume( left, right );
}

void VideoPlayer::SetRenderingTarget( Any target )
{
  GetImplementation( *this ).SetRenderingTarget( target );
}

void VideoPlayer::SetPlayPosition( int millisecond )
{
  GetImplementation( *this ).SetPlayPosition( millisecond );
}

int VideoPlayer::GetPlayPosition()
{
  return GetImplementation( *this ).GetPlayPosition();
}

void VideoPlayer::SetDisplayArea( DisplayArea area )
{
  GetImplementation( *this ).SetDisplayArea( area );
}

void VideoPlayer::SetDisplayRotation( Dali::VideoPlayerPlugin::DisplayRotation rotation )
{
  GetImplementation( *this ).SetDisplayRotation( rotation );
}

Dali::VideoPlayerPlugin::DisplayRotation VideoPlayer::GetDisplayRotation()
{
  return GetImplementation( *this ).GetDisplayRotation();
}

Dali::VideoPlayerPlugin::VideoPlayerSignalType& VideoPlayer::FinishedSignal()
{
  return GetImplementation( *this ).FinishedSignal();
}

void VideoPlayer::Forward( int millisecond )
{
  GetImplementation( *this ).Forward( millisecond );
}

void VideoPlayer::Backward( int millisecond )
{
  GetImplementation( *this ).Backward( millisecond );
}

bool VideoPlayer::IsVideoTextureSupported()
{
  return GetImplementation( *this ).IsVideoTextureSupported();
}

void VideoPlayer::SetCodecType( Dali::VideoPlayerPlugin::CodecType type )
{
  GetImplementation( *this ).SetCodecType( type );
}

Dali::VideoPlayerPlugin::CodecType VideoPlayer::GetCodecType() const
{
  return GetImplementation( *this ).GetCodecType();
}

void VideoPlayer::SetDisplayMode( Dali::VideoPlayerPlugin::DisplayMode::Type mode )
{
  GetImplementation( *this ).SetDisplayMode( mode );
}

Dali::VideoPlayerPlugin::DisplayMode::Type VideoPlayer::GetDisplayMode() const
{
  return GetImplementation( *this ).GetDisplayMode();
}

} // namespace Dali;

