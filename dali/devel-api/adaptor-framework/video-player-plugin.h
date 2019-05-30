#ifndef DALI_VIDEO_PLAYER_PLUGIN_H
#define DALI_VIDEO_PLAYER_PLUGIN_H

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
#include <dali/public-api/signals/dali-signal.h>
#include <dali/public-api/math/rect.h>

namespace Dali
{

class Any;
typedef Dali::Rect< int > DisplayArea;

/**
 * @brief VideoPlayerPlugin is an abstract interface, used by dali-adaptor to access video player plugin.
 * A concrete implementation must be created for each platform and provided as dynamic library.
 * @SINCE_1_1.38
 */
class VideoPlayerPlugin
{
public:

  typedef Signal< void () > VideoPlayerSignalType;

  /**
   * @brief Video display rotation option
   * @SINCE_1_1.38
   * @remarks The option is needed only for window surface rendering target
   */
  enum DisplayRotation
  {
    ROTATION_NONE,   ///< Display isn't rotated. @SINCE_1_1.38
    ROTATION_90,     ///< Display is rotated 90 degree. @SINCE_1_1.38
    ROTATION_180,    ///< Display is rotated 180 degree. @SINCE_1_1.38
    ROTATION_270     ///< Display is rotated 270 degree. @SINCE_1_1.38
  };

  /**
   * @brief Enumeration for video codec type
   */
  enum class CodecType
  {
    DEFAULT,      ///< Codec which has higher priority as default. Platform selects it. Usually the H/W codec has higher priority than S/W codec if it exist.
    HW,           ///< H/W codec
    SW            ///< S/W codec
  };

  /**
   * @brief The values of this enum determine how the video should be display mode to the view
   */
  struct DisplayMode
  {
    enum Type
    {
      LETTER_BOX = 0,     /**< Letter box */
      ORIGIN_SIZE,        /**< Origin size */
      FULL_SCREEN,        /**< Full-screen */
      CROPPED_FULL,       /**< Cropped full-screen */
      ORIGIN_OR_LETTER,   /**< Origin size (if surface size is larger than video size(width/height)) or Letter box (if video size(width/height) is larger than surface size) */
      DST_ROI             /**< Region of Interest */
    };
  };

  /**
   * @brief Constructor.
   * @SINCE_1_1.38
   */
  VideoPlayerPlugin(){}

  /**
   * @brief Destructor.
   * @SINCE_1_1.38
   */
  virtual ~VideoPlayerPlugin(){}

  /**
   * @brief Sets a URL of the video file to play.
   *
   * @SINCE_1_1.38
   * @param [in] url The url of video file
   */
  virtual void SetUrl( const std::string& url ) = 0;

  /**
   * @brief Returns the URL of the video file.
   * @SINCE_1_1.38
   * @return Url of string type
   */
  virtual std::string GetUrl() = 0;

  /**
   * @brief Sets the player looping status.
   * @SINCE_1_1.38
   *
   * @param [in] looping The new looping status: true or false
   */
  virtual void SetLooping(bool looping) = 0;

  /**
   * @brief Returns the player looping status.
   * @SINCE_1_1.38
   *
   * @return True if player is looping, false otherwise.
   */
  virtual bool IsLooping() = 0;

  /**
   * @brief Starts the video playback.
   * @SINCE_1_1.38
   */
  virtual void Play() = 0;

  /**
   * @brief Pauses the video playback.
   * @SINCE_1_1.38
   */
  virtual void Pause() = 0;

  /**
   * @brief Stops the video playback.
   * @SINCE_1_1.38
   */
  virtual void Stop() = 0;

  /**
   * @brief Sets the player mute status.
   * @SINCE_1_1.38
   * @param[in] mute The new mute status, true is mute.
   */
  virtual void SetMute( bool mute ) = 0;

  /**
   * @brief Returns the player mute status.
   * @SINCE_1_1.38
   * @return True if player is mute.
   */
  virtual bool IsMuted() = 0;

  /**
   * @brief Sets the player volume.
   * @SINCE_1_1.38
   * @param[in] left The left volume scalar
   * @param[in] right The right volume scalar
   */
  virtual void SetVolume( float left, float right ) = 0;

  /**
   * @brief Gets current volume factor.
   * @SINCE_1_1.38
   * @param[out] left The current left volume scalar
   * @param[out] right The current right volume scalar
   */
  virtual void GetVolume( float& left, float& right ) = 0;

  /**
   * @brief Sets video rendering target.
   * @SINCE_1_1.38
   * @param[in] target The target for video rendering, window surface or native image source
   */
  virtual void SetRenderingTarget( Any target ) = 0;

  /**
   * @brief Sets the position for playback.
   * @SINCE_1_1.38
   *
   * @param[in] millisecond The position for playback
   */
  virtual void SetPlayPosition( int millisecond ) = 0;

  /**
   * @brief Returns the current position in milliseconds.
   * @SINCE_1_1.38
   *
   * @return The current position of playback
   */
  virtual int GetPlayPosition() = 0;

  /**
   * @brief Sets the area of video display.
   * @SINCE_1_2.46
   * param[in] area The left-top position and size of the video display area
   */
  virtual void SetDisplayArea( DisplayArea area ) = 0;

  /**
   * @brief Sets video display rotation
   * @SINCE_1_1.38
   * @param[in] rotation The rotation of display
   */
  virtual void SetDisplayRotation( Dali::VideoPlayerPlugin::DisplayRotation rotation ) = 0;

  /**
   * @brief Returns rotation of current video display
   * @SINCE_1_1.38
   * @return The rotation of current display
   */
  virtual Dali::VideoPlayerPlugin::DisplayRotation GetDisplayRotation() = 0;

  /**
   * @brief Connect to this signal to be notified when a video playback have finished.
   *
   * @SINCE_1_1.38
   * @return A signal object to connect with.
   */
  virtual VideoPlayerSignalType& FinishedSignal() = 0;

  /**
   * @brief Seeks forward by the specified number of milliseconds.
   *
   * @SINCE_1_2.46
   * @param[in] millisecond The position for forward playback
   */
  virtual void Forward( int millisecond ) = 0;

  /**
   * @brief Seeks backward by the specified number of milliseconds.
   *
   * @SINCE_1_2.46
   * @param[in] millisecond The position for backward playback
   */
  virtual void Backward( int millisecond ) = 0;

  /**
   * @brief Checks whether the video texture is supported
   * @return True if supported, otherwise false.
   */
  virtual bool IsVideoTextureSupported() = 0;

  /**
   * @brief Sets codec type
   * @param[in] type The CodecType
   */
  virtual void SetCodecType( VideoPlayerPlugin::CodecType type ) = 0;

  /**
   * @brief Gets codec type
   * @return CodecType
   */
  virtual VideoPlayerPlugin::CodecType GetCodecType() const = 0;

  /**
   * @brief Sets the display mode for playback.
   * @param[in] mode of playback
   */
  virtual void SetDisplayMode( VideoPlayerPlugin::DisplayMode::Type mode ) = 0;

  /**
   * @brief Returns the current display mode.
   * @return The current display mode of playback
   */
  virtual VideoPlayerPlugin::DisplayMode::Type GetDisplayMode() const = 0;

};

} // namespace Dali;

#endif // DALI_VIDEO_PLAYER_PLUGIN_H
