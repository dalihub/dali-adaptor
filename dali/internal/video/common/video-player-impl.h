#ifndef DALI_VIDEO_PLAYER_IMPL_H
#define DALI_VIDEO_PLAYER_IMPL_H

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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/video-player.h>
#include <dali/devel-api/adaptor-framework/video-player-plugin.h>

namespace Dali
{
class Any;

namespace Internal
{

namespace Adaptor
{

class VideoPlayer;

typedef IntrusivePtr< VideoPlayer > VideoPlayerPtr;

/**
 * @brief VideoPlayer class is used for video playback.
 * @SINCE_1_1.24
 */
class VideoPlayer: public Dali::BaseObject
{
public:

  /**
   * @brief Creates a new VideoPlayer handle
   * @SINCE_1_1.24
   * @return VideoPlayer pointer
   */
  static VideoPlayerPtr New();

  /**
   * @copydoc Dali::VideoPlayer::SetUrl()
   */
  void SetUrl( const std::string& url );

  /**
   * @copydoc Dali::VideoPlayer::GetUrl()
   */
  std::string GetUrl();

  /**
   * @copydoc Dali::VideoPlayer::SetLooping()
   */
  void SetLooping(bool looping);

  /**
   * @copydoc Dali::VideoPlayer::IsLooping()
   */
  bool IsLooping();

  /**
   * @copydoc Dali::VideoPlayer::Play()
   */
  void Play();

  /**
   * @copydoc Dali::VideoPlayer::Pause()
   */
  void Pause();

  /**
   * @copydoc Dali::VideoPlayer::Stop()
   */
  void Stop();

  /**
   * @copydoc Dali::VideoPlayer::SetMute()
   */
  void SetMute( bool mute );

  /**
   * @copydoc Dali::VideoPlayer::IsMuted()
   */
  bool IsMuted();

  /**
   * @copydoc Dali::VideoPlayer::SetVolume()
   */
  void SetVolume( float left, float right );

  /**
   * @copydoc Dali::VideoPlayer::GetVolume()
   */
  void GetVolume( float& left, float& right );

  /**
   * @copydoc Dali::VideoPlayer::SetRenderingTarget()
   */
  void SetRenderingTarget( Dali::Any target );

  /**
   * @copydoc Dali::VideoPlayer::SetPlayPosition()
   */
  void SetPlayPosition( int millisecond );

  /**
   * @copydoc Dali::VideoPlayer::GetPlayPosition()
   */
  int GetPlayPosition();

  /**
   * @copydoc Dali::VideoPlayer::SetDisplayArea()
   */
  void SetDisplayArea( DisplayArea area );

  /**
   * @copydoc Dali::VideoPlayer::SetSetDisplayRotation()
   */
  void SetDisplayRotation( Dali::VideoPlayerPlugin::DisplayRotation rotation );

  /**
   * @copydoc Dali::VideoPlayer::GetDisplayRotation()
   */
  Dali::VideoPlayerPlugin::DisplayRotation GetDisplayRotation();

  /**
   * @copydoc Dali::VideoPlayer::FinishedSignal()
   */
  Dali::VideoPlayerPlugin::VideoPlayerSignalType& FinishedSignal();

  /**
   * @brief Initializes member data.
   */
  void Initialize();

  /**
   * @brief Dali::VideoPlayer::Forward()
   */
  void Forward( int millisecond );

  /**
   * @brief Dali::VideoPlayer::Backward()
   */
  void Backward( int millisecond );

  /**
   * @brief Dali::VideoPlayer::IsVideoTextureSupported()
   */
  bool IsVideoTextureSupported();

  /**
   * @brief Dali::VideoPlayer::SetCodecType()
   */
  void SetCodecType( Dali::VideoPlayerPlugin::CodecType type );

  /**
   * @brief Dali::VideoPlayer::GetCodecType()
   */
  Dali::VideoPlayerPlugin::CodecType GetCodecType() const;

  /**
   * @copydoc Dali::VideoPlayer::SetDisplayMode()
   */
  void SetDisplayMode( Dali::VideoPlayerPlugin::DisplayMode::Type mode );

  /**
   * @brief Dali::VideoPlayer::GetDisplayMode()
   */
  Dali::VideoPlayerPlugin::DisplayMode::Type GetDisplayMode() const;

private:

  /**
   * @brief Constructor.
   * @SINCE_1_1.24
   */
  VideoPlayer();

  /**
   * @brief Destructor.
   * @SINCE_1_1.24
   */
  virtual ~VideoPlayer();

  // Undefined copy constructor
  VideoPlayer( const VideoPlayer& player );

  // Undefined assignment operator
  VideoPlayer& operator=( const VideoPlayer& player );

private:

  Dali::VideoPlayerPlugin* mPlugin; ///< Videoplayer plugin handle
  void* mHandle; ///< Handle for the loaded library

  typedef Dali::VideoPlayerPlugin* (*CreateVideoPlayerFunction)();
  typedef void (*DestroyVideoPlayerFunction)( Dali::VideoPlayerPlugin* plugin );

  CreateVideoPlayerFunction mCreateVideoPlayerPtr;
  DestroyVideoPlayerFunction mDestroyVideoPlayerPtr;

  Dali::VideoPlayerPlugin::VideoPlayerSignalType mFinishedSignal;
};

} // namespace Adaptor
} // namespace Internal

inline static Internal::Adaptor::VideoPlayer& GetImplementation( Dali::VideoPlayer& player )
{
  DALI_ASSERT_ALWAYS( player && "VideoPlayer handle is empty." );

  BaseObject& handle = player.GetBaseObject();

  return static_cast< Internal::Adaptor::VideoPlayer& >( handle );
}

inline static const Internal::Adaptor::VideoPlayer& GetImplementation( const Dali::VideoPlayer& player )
{
  DALI_ASSERT_ALWAYS( player && "VideoPlayer handle is empty." );

  const BaseObject& handle = player.GetBaseObject();

  return static_cast< const Internal::Adaptor::VideoPlayer& >( handle );
}

} // namespace Dali;

#endif // DALI_VIDEO_PLAYER_IMPL_H

