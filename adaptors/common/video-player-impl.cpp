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
#include <video-player-impl.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/public-api/object/any.h>

// INTERNAL INCLUDES
#include <native-image-source.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace // unnamed namespace
{
const char* VIDEO_PLUGIN_SO( "libdali-video-player-plugin.so" );

Dali::BaseHandle Create()
{
  return Dali::VideoPlayer::New();
}

Dali::TypeRegistration type( typeid( Dali::VideoPlayer ), typeid( Dali::BaseHandle ), Create );

} // unnamed namespace

VideoPlayerPtr VideoPlayer::New()
{
  VideoPlayerPtr player = new VideoPlayer();
  return player;
}

VideoPlayer::VideoPlayer()
: mPlugin( NULL ),
  mHandle( NULL ),
  mCreateVideoPlayerPtr( NULL ),
  mDestroyVideoPlayerPtr( NULL )
{
}

VideoPlayer::~VideoPlayer()
{
  if( mHandle != NULL )
  {
    if( mDestroyVideoPlayerPtr != NULL )
    {
      mDestroyVideoPlayerPtr( mPlugin );
    }

    dlclose( mHandle );
  }
}

void VideoPlayer::Initialize()
{
  char* error = NULL;

  mHandle = dlopen( VIDEO_PLUGIN_SO, RTLD_LAZY );

  error = dlerror();
  if( mHandle == NULL || error != NULL )
  {
    DALI_LOG_ERROR( "VideoPlayer::Initialize(), dlopen error: %s\n", error );
    return;
  }

  mCreateVideoPlayerPtr = reinterpret_cast< CreateVideoPlayerFunction >( dlsym( mHandle, "CreateVideoPlayerPlugin" ) );
  if( mCreateVideoPlayerPtr == NULL )
  {
    DALI_LOG_ERROR( "Can't load symbol CreateVideoPlayerPlugin(), error: %s\n", error );
    return;
  }

  mPlugin = mCreateVideoPlayerPtr();

  if( mPlugin == NULL )
  {
    DALI_LOG_ERROR( "Can't create the VideoPlayerPlugin object\n" );
    return;
  }

  mDestroyVideoPlayerPtr = reinterpret_cast< DestroyVideoPlayerFunction >( dlsym( mHandle, "DestroyVideoPlayerPlugin" ) );
  if( mDestroyVideoPlayerPtr == NULL )
  {
    DALI_LOG_ERROR( "Can't load symbol DestroyVideoPlayerPlugin(), error: %s\n", error );
    return;
  }

}

void VideoPlayer::SetUrl( const std::string& url )
{
  if( mPlugin != NULL )
  {
    mPlugin->SetUrl( url );
  }
}

std::string VideoPlayer::GetUrl()
{
  if( mPlugin != NULL )
  {
    return mPlugin->GetUrl();
  }

  return std::string( NULL );
}

void VideoPlayer::SetLooping(bool looping)
{
  if( mPlugin != NULL )
  {
    mPlugin->SetLooping( looping );
  }
}

bool VideoPlayer::IsLooping()
{
  if( mPlugin != NULL )
  {
    return mPlugin->IsLooping();
  }

  return false;
}

void VideoPlayer::Play()
{
  if( mPlugin != NULL )
  {
    mPlugin->Play();
  }
}

void VideoPlayer::Pause()
{
  if( mPlugin != NULL )
  {
    mPlugin->Pause();
  }
}

void VideoPlayer::Stop()
{
  if( mPlugin != NULL )
  {
    mPlugin->Stop();
  }
}

void VideoPlayer::SetMute( bool mute )
{
  if( mPlugin != NULL )
  {
    mPlugin->SetMute( mute );
  }
}

bool VideoPlayer::IsMuted()
{
  if( mPlugin != NULL )
  {
    return mPlugin->IsMuted();
  }

  return false;
}

void VideoPlayer::SetVolume( float left, float right )
{
  if( mPlugin != NULL )
  {
    mPlugin->SetVolume( left, right );
  }
}

void VideoPlayer::GetVolume( float& left, float& right )
{
  if( mPlugin != NULL )
  {
    mPlugin->GetVolume( left, right );
  }
}

void VideoPlayer::SetRenderingTarget( Dali::Any target )
{
  if( mPlugin != NULL )
  {
    mPlugin->SetRenderingTarget( target );
  }
}

void VideoPlayer::SetPlayPosition( int millisecond )
{
  if( mPlugin != NULL )
  {
    mPlugin->SetPlayPosition( millisecond );
  }
}

int VideoPlayer::GetPlayPosition()
{
  if( mPlugin != NULL )
  {
    return mPlugin->GetPlayPosition();
  }
  return 0;
}

void VideoPlayer::SetDisplayRotation( Dali::VideoPlayerPlugin::DisplayRotation rotation )
{
  if( mPlugin != NULL )
  {
    mPlugin->SetDisplayRotation( rotation );
  }
}

Dali::VideoPlayerPlugin::DisplayRotation VideoPlayer::GetDisplayRotation()
{
  if( mPlugin != NULL )
  {
    return mPlugin->GetDisplayRotation();
  }

  return Dali::VideoPlayerPlugin::ROTATION_NONE;
}

Dali::VideoPlayerPlugin::VideoPlayerSignalType& VideoPlayer::FinishedSignal()
{
  if( mPlugin != NULL )
  {
    return mPlugin->FinishedSignal();
  }

  return mFinishedSignal;
}

} // namespace Adaptor;
} // namespace Internal;
} // namespace Dali;

