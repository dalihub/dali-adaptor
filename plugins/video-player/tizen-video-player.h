#ifndef __DALI_TIZEN_VIDEO_PLAYER_PLUGIN_H__
#define __DALI_TIZEN_VIDEO_PLAYER_PLUGIN_H__

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

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/mutex.h>
#include <player.h>
#include <string>

#ifndef HAVE_WAYLAND
#define HAVE_WAYLAND
#endif
#include <player_internal.h>

// INTERNAL INCLUDES
#include <native-image-source.h>
#include <video-player-plugin.h>
#include <timer.h>

namespace Dali
{

namespace Plugin
{

/**
 * @brief Implementation of the Tizen video player class which has Tizen platform dependency.
 * @SINCE_1_1.38
 */
class TizenVideoPlayer : public Dali::VideoPlayerPlugin, public Dali::ConnectionTracker
{
public:

  /**
   * @brief Video rendering target type
   * @SINCE_1_1.38
   */
  enum RenderingTargetType
  {
    WindowSurface, ///< HW underlay
    NativeImage ///< texture stream
  };

  /**
   * @brief Constructor.
   * @SINCE_1_1.38
   */
  TizenVideoPlayer();

  /**
   * @brief Destructor.
   * @SINCE_1_1.38
   */
  virtual ~TizenVideoPlayer();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetUrl()
   */
  virtual void SetUrl( const std::string& url );

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetUrl()
   */
  virtual std::string GetUrl();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetLooping()
   */
  virtual void SetLooping(bool looping);

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsLooping()
   */
  virtual bool IsLooping();

  /**
   * @copydoc Dali::VideoPlayerPlugin::Play()
   */
  virtual void Play();

  /**
   * @copydoc Dali::VideoPlayerPlugin::Pause()
   */
  virtual void Pause();

  /**
   * @copydoc Dali::VideoPlayerPlugin::Stop()
   */
  virtual void Stop();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetMute()
   */
  virtual void SetMute( bool mute );

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsMute()
   */
  virtual bool IsMuted();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetVolume()
   */
  virtual void SetVolume( float left, float right );

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetVolume()
   */
  virtual void GetVolume( float& left, float& right );

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetRenderingTarget()
   */
  void SetRenderingTarget( Any target );

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetPlayPosition()
   */
  virtual void SetPlayPosition( int millisecond );

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetPlayPosition()
   */
  virtual int GetPlayPosition();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetDisplayArea()
   */
  virtual void SetDisplayArea( DisplayArea area );

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetDisplayRotation()
   */
  virtual void SetDisplayRotation( Dali::VideoPlayerPlugin::DisplayRotation rotation );

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetDisplayRotation()
   */
  virtual Dali::VideoPlayerPlugin::DisplayRotation GetDisplayRotation();

  /**
   * @copydoc Dali::VideoPlayerPlugin::FinishedSignal()
   */
  virtual Dali::VideoPlayerPlugin::VideoPlayerSignalType& FinishedSignal();

  /**
   * @brief Push media packet with video frame image
   */
  void PushPacket( media_packet_h packet );

  /**
   * @brief Dali::VideoPlayer::Forward()
   */
  void Forward( int millisecond );

  /**
   * @brief Dali::VideoPlayer::Backward()
   */
  void Backward( int millisecond );

private:

  /**
   * @brief Updates video frame image by timer if rendering targe is native image source
   */
  bool Update();

  /**
   * @brief Gets current player state
   */
  void GetPlayerState( player_state_e* state );

  /**
   * @brief Destroy all packests, which this plugin stores
   */
  void DestroyPackets();

  /**
   * @brief Initializes player for video rendering using native image source
   */
  void InitializeTextureStreamMode( Dali::NativeImageSourcePtr nativeImageSourcePtr );

  /**
   * @brief Initializes player for video rendering using wayland window surface
   */
  void InitializeUnderlayMode( Ecore_Wl_Window* ecoreWlWindow );

private:

  std::string mUrl; ///< The video file path
  player_h mPlayer; ///< Tizen player handle
  player_state_e mPlayerState; ///< Tizen player state
  tbm_surface_h mTbmSurface; ///< tbm surface handle
  media_packet_h mPacket; ///< Media packet handle with tbm surface of current video frame image
  Dali::NativeImageSourcePtr mNativeImageSourcePtr; ///< native image source for video rendering
  Dali::Timer mTimer; ///< Timer for texture streaming rendering
  Dali::Vector4 mBackgroundColor; ///< Current background color, which texturestream mode needs.
  RenderingTargetType mTargetType; ///< Current rendering target type

  Dali::Mutex mPacketMutex;
  Dali::Vector< media_packet_h > mPacketVector; ///< Container for media packet handle from Tizen player callback

  Ecore_Wl_Window* mEcoreWlWindow;

  bool mAlphaBitChanged; ///< True if underlay rendering initialization changes window alpha

public:

  Dali::VideoPlayerPlugin::VideoPlayerSignalType mFinishedSignal;
};

} // namespace Plugin
} // namespace Dali;

#endif
