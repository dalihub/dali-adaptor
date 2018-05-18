#ifndef __DALI_VIDEO_PLAYER_H__
#define __DALI_VIDEO_PLAYER_H__

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-handle.h>

//INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/video-player-plugin.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

class Any;

namespace Internal
{

namespace Adaptor
{
  class VideoPlayer;
} // namespace Adaptor

} // namespace Internal

/**
 * @brief VideoPlayer class is used for video playback.
 * @SINCE_1_1.38
 */
class DALI_ADAPTOR_API VideoPlayer: public BaseHandle
{
public:

  /**
   * @brief Constructor.
   * @SINCE_1_1.38
   */
  VideoPlayer();

  /**
   * @brief Destructor.
   * @SINCE_1_1.38
   */
  ~VideoPlayer();

  /**
   * @brief Creates a new instance of a VideoPlayer.
   * @SINCE_1_1.38
   */
  static VideoPlayer New();

 /**
   * @brief Copy constructor.
   *
   * @SINCE_1_1.38
   * @param[in] player VideoPlayer to copy. The copied player will point at the same implementation
   */
  VideoPlayer( const VideoPlayer& player );

 /**
   * @brief Assignment operator.
   *
   * @SINCE_1_1.38
   * @param[in] player The VideoPlayer to assign from.
   * @return The updated VideoPlayer.
   */
  VideoPlayer& operator=( const VideoPlayer& player );

  /**
   * @brief Downcast a handle to VideoPlayer handle.
   *
   * If handle points to a VideoPlayer the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @SINCE_1_1.38
   * @param[in] handle Handle to an object
   * @return Handle to a VideoPlayer or an uninitialized handle
   */
  static VideoPlayer DownCast( BaseHandle handle );

  /**
   * @brief Sets a URL of the video file to play.
   *
   * @SINCE_1_1.38
   * @param [in] url The url of video file
   */
  void SetUrl( const std::string& url );

  /**
   * @brief Returns the URL of the video file.
   * @SINCE_1_1.38
   * @return Url of string type
   */
  std::string GetUrl();

  /**
   * @brief Sets the player looping status.
   * @SINCE_1_1.38
   *
   * @param [in] looping The new looping status: true or false
   */
  void SetLooping(bool looping);

  /**
   * @brief Returns the player looping status.
   * @SINCE_1_1.38
   *
   * @return True if player is looping, false otherwise.
   */
  bool IsLooping();

  /**
   * @brief Starts the video playback.
   * @SINCE_1_1.38
   */
  void Play();

  /**
   * @brief Pauses the video playback.
   * @SINCE_1_1.38
   */
  void Pause();

  /**
   * @brief Stops the video playback.
   * @SINCE_1_1.38
   */
  void Stop();

  /**
   * @brief Sets the player mute status.
   * @SINCE_1_1.38
   * @param[in] mute The new mute status, true is mute.
   */
  void SetMute( bool mute );

  /**
   * @brief Returns the player mute status.
   * @SINCE_1_1.38
   * @return True if player is mute.
   */
  bool IsMuted();

  /**
   * @brief Sets the player volume.
   * @SINCE_1_1.38
   * @param[in] left The left volume scalar
   * @param[in] right The right volume scalar
   */
  void SetVolume( float left, float right );

  /**
   * @brief Returns current volume factor.
   * @SINCE_1_1.38
   * @param[out] left The current left volume scalar
   * @param[out] right The current right volume scalar
   */
  void GetVolume( float& left, float& right );

  /**
   * @brief Sets video rendering target.
   * @SINCE_1_1.38
   * @param[in] target The target for video rendering, window surface or native image source
   */
  void SetRenderingTarget( Any target );

  /**
   * @brief Sets the position for playback.
   * @SINCE_1_1.38
   *
   * @param[in] millisecond The position for playback
   */
  void SetPlayPosition( int millisecond );

  /**
   * @brief Gets the current position in milliseconds.
   * @SINCE_1_1.38
   *
   * @return The current position of playback
   */
  int GetPlayPosition();

  /**
   * @brief Sets the area of video display.
   * @SINCE_1_2.46
   * param[in] area The left-top position and size of the video display area
   */
  void SetDisplayArea( DisplayArea area );

  /**
   * @brief Sets video display rotation
   * @SINCE_1_1.38
   * @param[in] rotation The rotation of display
   */
  void SetDisplayRotation( Dali::VideoPlayerPlugin::DisplayRotation rotation );

  /**
   * @brief Returns rotation of current video display
   * @SINCE_1_1.38
   * @return The rotation of current display
   */
  Dali::VideoPlayerPlugin::DisplayRotation GetDisplayRotation();

  /**
   * @brief Connect to this signal to be notified when a video playback have finished.
   *
   * @SINCE_1_1.38
   * @return A signal object to connect with.
   */
  Dali::VideoPlayerPlugin::VideoPlayerSignalType& FinishedSignal();

  /**
   * @brief Seeks forward by the specified number of milliseconds.
   *
   * @SINCE_1_2.46
   * @param[in] millisecond The position for forward playback
   */
  void Forward( int millisecond );

  /**
   * @brief Seeks backward by the specified number of milliseconds.
   *
   * @SINCE_1_2.46
   * @param[in] millisecond The position for backward playback
   */
  void Backward( int millisecond );

  /**
   * @brief Checks whether the video texture is supported
   * @return True if supported, otherwise false.
   */
  bool IsVideoTextureSupported();

  /**
   * @brief Sets codec type
   * @param[in] type The VideoCodec::Type
   */
  void SetCodecType( Dali::VideoPlayerPlugin::CodecType type );

  /**
   * @brief Gets codec type
   * @return VideoCodec::Type
   */
  Dali::VideoPlayerPlugin::CodecType GetCodecType() const;

  /**
   * @brief Sets the display mode for playback.
   * @param[in] mode of playback
   */
  void SetDisplayMode( Dali::VideoPlayerPlugin::DisplayMode::Type mode );

  /**
   * @brief Gets display mode
   * @return DisplayMode
   */
  Dali::VideoPlayerPlugin::DisplayMode::Type GetDisplayMode() const;

private: // Not intended for application developers

  /**
   * @brief Internal constructor
   * @SINCE_1_1.38
   */
  explicit DALI_INTERNAL VideoPlayer( Internal::Adaptor::VideoPlayer* internal );
};

} // namespace Dali;

#endif

