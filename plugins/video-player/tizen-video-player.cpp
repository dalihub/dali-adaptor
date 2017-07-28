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
#include <tizen-video-player.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/stage.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES

// The plugin factories
extern "C" DALI_EXPORT_API Dali::VideoPlayerPlugin* CreateVideoPlayerPlugin( void )
{
  return new Dali::Plugin::TizenVideoPlayer;
}

extern "C" DALI_EXPORT_API void DestroyVideoPlayerPlugin( Dali::VideoPlayerPlugin* plugin )
{
  if( plugin != NULL )
  {
    delete plugin;
  }
}

namespace Dali
{

namespace Plugin
{

namespace
{

const int TIMER_INTERVAL( 20 );

static void MediaPacketVideoDecodedCb( media_packet_h packet, void* user_data )
{
  TizenVideoPlayer* player = static_cast< TizenVideoPlayer* >( user_data );

  if( player == NULL )
  {
    DALI_LOG_ERROR( "Decoded callback got Null pointer as user_data.\n" );
    return;
  }

  player->PushPacket( packet );
}

static void EmitPlaybackFinishedSignal( void* user_data )
{
  TizenVideoPlayer* player = static_cast< TizenVideoPlayer* >( user_data );
  DALI_LOG_ERROR( "EmitPlaybackFinishedSignal.\n" );

  if( player == NULL )
  {
    DALI_LOG_ERROR( "Decoded callback got Null pointer as user_data.\n" );
    return;
  }

  DALI_LOG_ERROR( "EmitPlaybackFinishedSignal.\n" );

  if( !player->mFinishedSignal.Empty() )
  {
    DALI_LOG_ERROR( "EmitPlaybackFinishedSignal.3\n" );
    player->mFinishedSignal.Emit();
  }

  player->Stop();
}

// ToDo: VD player_set_play_position() doesn't work when callback pointer is NULL.
// We should check whether this callback is needed in platform.
static void PlayerSeekCompletedCb( void* data )
{
}

void LogPlayerError( int error )
{
  if( error != PLAYER_ERROR_NONE )
  {
    switch( error )
    {
      case PLAYER_ERROR_OUT_OF_MEMORY:
      {
        DALI_LOG_ERROR( "Player error: Out of memory\n" );
        return;
      }
      case PLAYER_ERROR_INVALID_PARAMETER:
      {
        DALI_LOG_ERROR( "Player error: Invalid parameter\n" );
        return;
      }
      case PLAYER_ERROR_NO_SUCH_FILE:
      {
        DALI_LOG_ERROR( "Player error: No such file\n" );
        return;
      }
      case PLAYER_ERROR_INVALID_OPERATION:
      {
        DALI_LOG_ERROR( "Player error: Invalid operation\n" );
        return;
      }
      case PLAYER_ERROR_FILE_NO_SPACE_ON_DEVICE:
      {
        DALI_LOG_ERROR( "Player error: No space on device\n" );
        return;
      }
      case PLAYER_ERROR_FEATURE_NOT_SUPPORTED_ON_DEVICE:
      {
        DALI_LOG_ERROR( "Player error: Not supported feature on device\n" );
        return;
      }
      case PLAYER_ERROR_SEEK_FAILED:
      {
        DALI_LOG_ERROR( "Player error: Seek failed\n" );
        return;
      }
      case PLAYER_ERROR_INVALID_STATE:
      {
        DALI_LOG_ERROR( "Player error: Invalid state\n" );
        return;
      }
      case PLAYER_ERROR_NOT_SUPPORTED_FILE:
      {
        DALI_LOG_ERROR( "Player error: Not supported file\n" );
        return;
      }
      case PLAYER_ERROR_INVALID_URI:
      {
        DALI_LOG_ERROR( "Player error: Invalid uri\n" );
        return;
      }
      case PLAYER_ERROR_SOUND_POLICY:
      {
        DALI_LOG_ERROR( "Player error: Sound policy\n" );
        return;
      }
      case PLAYER_ERROR_CONNECTION_FAILED:
      {
        DALI_LOG_ERROR( "Player error: Connection failed\n" );
        return;
      }
      case PLAYER_ERROR_VIDEO_CAPTURE_FAILED:
      {
        DALI_LOG_ERROR( "Player error: Video capture failed\n" );
        return;
      }
      case PLAYER_ERROR_DRM_EXPIRED:
      {
        DALI_LOG_ERROR( "Player error: DRM expired\n" );
        return;
      }
      case PLAYER_ERROR_DRM_NO_LICENSE:
      {
        DALI_LOG_ERROR( "Player error: No license\n" );
        return;
      }
      case PLAYER_ERROR_DRM_FUTURE_USE:
      {
        DALI_LOG_ERROR( "Player error: License for future use\n" );
        return;
      }
      case PLAYER_ERROR_DRM_NOT_PERMITTED:
      {
        DALI_LOG_ERROR( "Player error: Format not permitted\n" );
        return;
      }
      case PLAYER_ERROR_RESOURCE_LIMIT:
      {
        DALI_LOG_ERROR( "Player error: Resource limit\n" );
        return;
      }
      case PLAYER_ERROR_PERMISSION_DENIED:
      {
        DALI_LOG_ERROR( "Player error: Permission denied\n" );
        return;
      }
      case PLAYER_ERROR_SERVICE_DISCONNECTED:
      {
        DALI_LOG_ERROR( "Player error: Service disconnected\n" );
        return;
      }
      case PLAYER_ERROR_BUFFER_SPACE:
      {
        DALI_LOG_ERROR( "Player error: Buffer space\n" );
        return;
      }
    }
  }
}

} // unnamed namespace

TizenVideoPlayer::TizenVideoPlayer()
: mPlayer( NULL ),
  mPlayerState( PLAYER_STATE_NONE ),
  mTbmSurface( NULL ),
  mPacket( NULL ),
  mTargetType( NativeImage ),
  mAlphaBitChanged( false )
{
}

TizenVideoPlayer::~TizenVideoPlayer()
{
}

void TizenVideoPlayer::GetPlayerState( player_state_e* state )
{
  if( mPlayer != NULL && player_get_state( mPlayer, state ) != PLAYER_ERROR_NONE )
  {
    DALI_LOG_ERROR( "player_get_state error: Invalid parameter\n" );
    *state = PLAYER_STATE_NONE;
  }
}

void TizenVideoPlayer::SetUrl( const std::string& url )
{
  if( mUrl != url )
  {
    mUrl = url;

    GetPlayerState( &mPlayerState );

    if( mPlayerState != PLAYER_STATE_NONE && mPlayerState != PLAYER_STATE_IDLE )
    {
      Stop();
      int error = player_unprepare( mPlayer );
      LogPlayerError( error );
    }

    if( mPlayerState == PLAYER_STATE_IDLE )
    {
      int error = player_set_uri( mPlayer, mUrl.c_str() );
      LogPlayerError( error );

      error = player_prepare( mPlayer );
      LogPlayerError( error );
    }
  }
}

std::string TizenVideoPlayer::GetUrl()
{
  return mUrl;
}

void TizenVideoPlayer::SetRenderingTarget( Any target )
{
  int error;
  if( mPlayerState != PLAYER_STATE_NONE )
  {
    GetPlayerState( &mPlayerState );

    if( mPlayerState != PLAYER_STATE_IDLE )
    {
      Stop();
      error = player_unprepare( mPlayer );
      LogPlayerError( error );
    }

    error = player_destroy( mPlayer );
    LogPlayerError( error );
    mPlayerState = PLAYER_STATE_NONE;
    mPlayer = NULL;
    mUrl = "";
  }

  mNativeImageSourcePtr = NULL;
  mEcoreWlWindow = NULL;

  if( target.GetType() == typeid( Dali::NativeImageSourcePtr ) )
  {
    mTargetType = TizenVideoPlayer::NativeImage;

    Dali::NativeImageSourcePtr nativeImageSourcePtr = AnyCast< Dali::NativeImageSourcePtr >( target );

    InitializeTextureStreamMode( nativeImageSourcePtr );
  }
  else if( target.GetType() == typeid( Ecore_Wl_Window* ) )
  {
    mTargetType = TizenVideoPlayer::WindowSurface;

    Ecore_Wl_Window* nativeWindow = Dali::AnyCast< Ecore_Wl_Window* >( target );
    InitializeUnderlayMode( nativeWindow );
  }
  else
  {
    DALI_LOG_ERROR( "Video rendering target is unknown\n" );
  }
}

void TizenVideoPlayer::SetLooping( bool looping )
{
  GetPlayerState( &mPlayerState );

  if( mPlayerState != PLAYER_STATE_NONE )
  {
    int error = player_set_looping( mPlayer, looping );
    LogPlayerError( error );
  }
}

bool TizenVideoPlayer::IsLooping()
{
  GetPlayerState( &mPlayerState );

  bool looping = false;
  if( mPlayerState != PLAYER_STATE_NONE )
  {
    int error = player_is_looping( mPlayer, &looping );
    LogPlayerError( error );
  }

  return looping;
}

void TizenVideoPlayer::Play()
{
  GetPlayerState( &mPlayerState );

  if( mPlayerState == PLAYER_STATE_READY || mPlayerState == PLAYER_STATE_PAUSED )
  {
    if( mNativeImageSourcePtr != NULL && mTimer )
    {
      mTimer.Start();
    }

    int error = player_start( mPlayer );
    LogPlayerError( error );
  }
}

void TizenVideoPlayer::Pause()
{
  GetPlayerState( &mPlayerState );

  if( mPlayerState == PLAYER_STATE_PLAYING )
  {
    int error = player_pause( mPlayer );
    LogPlayerError( error );

    if( mNativeImageSourcePtr != NULL && mTimer )
    {
      mTimer.Stop();
      DestroyPackets();
    }
  }
}

void TizenVideoPlayer::Stop()
{
  GetPlayerState( &mPlayerState );

  if( mPlayerState == PLAYER_STATE_PLAYING || mPlayerState == PLAYER_STATE_PAUSED )
  {
    int error = player_stop( mPlayer );
    LogPlayerError( error );
  }

  if( mNativeImageSourcePtr != NULL && mTimer )
  {
    mTimer.Stop();
    DestroyPackets();
  }
}

void TizenVideoPlayer::SetMute( bool muted )
{
  GetPlayerState( &mPlayerState );

  if( mPlayerState == PLAYER_STATE_IDLE ||
      mPlayerState == PLAYER_STATE_READY ||
      mPlayerState == PLAYER_STATE_PLAYING ||
      mPlayerState == PLAYER_STATE_PAUSED
    )
  {
    int error = player_set_mute( mPlayer, muted );
    LogPlayerError( error );
  }
}

bool TizenVideoPlayer::IsMuted()
{
  GetPlayerState( &mPlayerState );
  bool muted = false;

   if( mPlayerState == PLAYER_STATE_IDLE ||
      mPlayerState == PLAYER_STATE_READY ||
      mPlayerState == PLAYER_STATE_PLAYING ||
      mPlayerState == PLAYER_STATE_PAUSED
    )
  {
    int error = player_is_muted( mPlayer, &muted );
    LogPlayerError( error );
  }

  return muted;
}

void TizenVideoPlayer::SetVolume( float left, float right )
{
  GetPlayerState( &mPlayerState );

  int error = player_set_volume( mPlayer, left, right );
  LogPlayerError( error );
}

void TizenVideoPlayer::GetVolume( float& left, float& right )
{
  GetPlayerState( &mPlayerState );

  int error = player_get_volume( mPlayer, &left, &right );
  LogPlayerError( error );
}

void TizenVideoPlayer::SetPlayPosition( int millisecond )
{
  int error;

  GetPlayerState( &mPlayerState );

  if( mPlayerState == PLAYER_STATE_READY ||
      mPlayerState == PLAYER_STATE_PLAYING ||
      mPlayerState == PLAYER_STATE_PAUSED
  )
  {
    error = player_set_play_position( mPlayer, millisecond, false, PlayerSeekCompletedCb, NULL );
    LogPlayerError( error );
  }
}

int TizenVideoPlayer::GetPlayPosition()
{
  int error;
  int millisecond = 0;

  GetPlayerState( &mPlayerState );

  if( mPlayerState == PLAYER_STATE_IDLE ||
      mPlayerState == PLAYER_STATE_READY ||
      mPlayerState == PLAYER_STATE_PLAYING ||
      mPlayerState == PLAYER_STATE_PAUSED
  )
  {
    error = player_get_play_position( mPlayer, &millisecond );
    LogPlayerError( error );
  }

  return millisecond;
}

void TizenVideoPlayer::SetDisplayRotation( Dali::VideoPlayerPlugin::DisplayRotation rotation )
{
  if( mNativeImageSourcePtr != NULL )
  {
    DALI_LOG_ERROR( "SetDisplayRotation is only for window rendering target.\n" );
    return;
  }

  int error;
  if( mPlayerState != PLAYER_STATE_NONE )
  {
    error = player_set_display_rotation( mPlayer, static_cast< player_display_rotation_e >( rotation ) );
    LogPlayerError( error );
  }
}

Dali::VideoPlayerPlugin::DisplayRotation TizenVideoPlayer::GetDisplayRotation()
{
  if( mNativeImageSourcePtr != NULL )
  {
    DALI_LOG_ERROR( "GetDisplayRotation is only for window rendering target.\n" );
    return Dali::VideoPlayerPlugin::ROTATION_NONE;
  }

  int error;
  player_display_rotation_e rotation = PLAYER_DISPLAY_ROTATION_NONE;
  if( mPlayerState != PLAYER_STATE_NONE )
  {
    error = player_get_display_rotation( mPlayer, &rotation );
    LogPlayerError( error );
  }
  return static_cast< Dali::VideoPlayerPlugin::DisplayRotation >( rotation );
}

Dali::VideoPlayerPlugin::VideoPlayerSignalType& TizenVideoPlayer::FinishedSignal()
{
  return mFinishedSignal;
}

void TizenVideoPlayer::InitializeTextureStreamMode( Dali::NativeImageSourcePtr nativeImageSourcePtr )
{
  int error;

  mNativeImageSourcePtr = nativeImageSourcePtr;

  if( mAlphaBitChanged )
  {
    ecore_wl_window_alpha_set( mEcoreWlWindow, false );
    mAlphaBitChanged = false;
  }

  if( mPlayerState == PLAYER_STATE_NONE )
  {
    error = player_create( &mPlayer );
    LogPlayerError( error );
  }

  GetPlayerState( &mPlayerState );

  if( mPlayerState == PLAYER_STATE_IDLE )
  {
    error = player_set_completed_cb( mPlayer, EmitPlaybackFinishedSignal, this );
    LogPlayerError( error );

    error = player_set_media_packet_video_frame_decoded_cb( mPlayer, MediaPacketVideoDecodedCb, this );
    LogPlayerError( error );

    error = player_set_sound_type( mPlayer, SOUND_TYPE_MEDIA );
    LogPlayerError( error );

    error = player_set_display_mode( mPlayer, PLAYER_DISPLAY_MODE_FULL_SCREEN );
    LogPlayerError( error );

    error = player_set_display( mPlayer, PLAYER_DISPLAY_TYPE_NONE, NULL );
    LogPlayerError( error );

    error = player_set_display_visible( mPlayer, true );
    LogPlayerError( error );

    mTimer = Dali::Timer::New( TIMER_INTERVAL );
    mTimer.TickSignal().Connect( this, &TizenVideoPlayer::Update );
  }
}

void TizenVideoPlayer::InitializeUnderlayMode( Ecore_Wl_Window* ecoreWlWindow )
{
  int error;
  if( mPlayerState == PLAYER_STATE_NONE )
  {
    error = player_create( &mPlayer );
    LogPlayerError( error );
  }

  GetPlayerState( &mPlayerState );
  mEcoreWlWindow = ecoreWlWindow;

  if( mPlayerState == PLAYER_STATE_IDLE )
  {
    error = player_set_completed_cb( mPlayer, EmitPlaybackFinishedSignal, this );
    LogPlayerError( error );

    error = player_set_sound_type( mPlayer, SOUND_TYPE_MEDIA );
    LogPlayerError( error );

    error = player_set_display_mode( mPlayer, PLAYER_DISPLAY_MODE_DST_ROI );
    LogPlayerError( error );

    error = player_set_display_roi_area( mPlayer, 0, 0, 1, 1 );

    int width, height;
    mAlphaBitChanged = ( ecore_wl_window_alpha_get( mEcoreWlWindow ) )? false: true;
    ecore_wl_screen_size_get( &width, &height );

    if( mAlphaBitChanged )
    {
      ecore_wl_window_alpha_set( mEcoreWlWindow, true );
    }
    error = player_set_ecore_wl_display( mPlayer, PLAYER_DISPLAY_TYPE_OVERLAY, mEcoreWlWindow, 0, 0, width, height );
    LogPlayerError( error );

    error = player_set_display_visible( mPlayer, true );
    LogPlayerError( error );
  }
}

bool TizenVideoPlayer::Update()
{
  Dali::Mutex::ScopedLock lock( mPacketMutex );

  int error;

  if( mPacket != NULL )
  {
    error = media_packet_destroy( mPacket );
    if( error != MEDIA_PACKET_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Media packet destroy error: %d\n", error );
    }
    mPacket = NULL;
  }

  if( !mPacketVector.Empty() )
  {
    mPacket = static_cast< media_packet_h >( mPacketVector[0] );
    mPacketVector.Remove( mPacketVector.Begin() );
  }

  if( mPacket == NULL )
  {
    return true;
  }

  error = media_packet_get_tbm_surface( mPacket, &mTbmSurface );
  if( error != MEDIA_PACKET_ERROR_NONE )
  {
    media_packet_destroy( mPacket );
    mPacket = NULL;
    DALI_LOG_ERROR( " error: %d\n", error );
    return true;
  }

  Any source( mTbmSurface );
  mNativeImageSourcePtr->SetSource( source );
  Dali::Stage::GetCurrent().KeepRendering( 0.0f );

  return true;
}

void TizenVideoPlayer::DestroyPackets()
{
  int error;
  if( mPacket != NULL )
  {
    error = media_packet_destroy( mPacket );
    DALI_LOG_ERROR( "Media packet destroy error: %d\n", error );
    mPacket = NULL;
  }

  for(unsigned int i = 0; i < mPacketVector.Size(); ++i)
  {
    mPacket = static_cast< media_packet_h >( mPacketVector[i] );
    error = media_packet_destroy( mPacket );
    DALI_LOG_ERROR( "Media packet destroy error: %d\n", error );
    mPacket = NULL;
  }
  mPacketVector.Clear();
}

void TizenVideoPlayer::PushPacket( media_packet_h packet )
{
  Dali::Mutex::ScopedLock lock( mPacketMutex );
  mPacketVector.PushBack( packet );
}

void TizenVideoPlayer::SetDisplayArea( DisplayArea area )
{
  GetPlayerState( &mPlayerState );

  if( mNativeImageSourcePtr != NULL )
  {
    DALI_LOG_ERROR( "SetDisplayArea is only for window surface target.\n" );
    return;
  }

  if( mPlayerState == PLAYER_STATE_IDLE ||
      mPlayerState == PLAYER_STATE_READY ||
      mPlayerState == PLAYER_STATE_PLAYING ||
      mPlayerState == PLAYER_STATE_PAUSED

  )
  {
    int error = player_set_display_roi_area( mPlayer, area.x, area.y, area.width, area.height );
    LogPlayerError( error );
  }
}

void TizenVideoPlayer::Forward( int millisecond )
{
  int error;

  GetPlayerState( &mPlayerState );

  if( mPlayerState == PLAYER_STATE_READY ||
      mPlayerState == PLAYER_STATE_PLAYING ||
      mPlayerState == PLAYER_STATE_PAUSED
  )
  {
    int currentPosition = 0;
    int nextPosition = 0;

    error = player_get_play_position( mPlayer, &currentPosition );
    LogPlayerError( error );

    nextPosition = currentPosition + millisecond;

    error = player_set_play_position( mPlayer, nextPosition, false, PlayerSeekCompletedCb, NULL );
    LogPlayerError( error );
  }
}

void TizenVideoPlayer::Backward( int millisecond )
{
  int error;

  GetPlayerState( &mPlayerState );

  if( mPlayerState == PLAYER_STATE_READY ||
      mPlayerState == PLAYER_STATE_PLAYING ||
      mPlayerState == PLAYER_STATE_PAUSED
  )
  {
    int currentPosition = 0;
    int nextPosition = 0;

    error = player_get_play_position( mPlayer, &currentPosition );
    LogPlayerError( error );

    nextPosition = currentPosition - millisecond;
    nextPosition = ( nextPosition < 0 )? 0 : nextPosition;

    error = player_set_play_position( mPlayer, nextPosition, false, PlayerSeekCompletedCb, NULL );
    LogPlayerError( error );
  }
}

} // namespace Plugin
} // namespace Dali;
