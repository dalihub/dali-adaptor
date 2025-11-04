#ifndef DALI_VIDEO_PLAYER_IMPL_H
#define DALI_VIDEO_PLAYER_IMPL_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/video-sync-mode.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/video-player-plugin.h>
#include <dali/devel-api/adaptor-framework/video-player.h>

namespace Dali
{
class Any;

namespace Internal
{
namespace Adaptor
{
class VideoPlayer;

typedef IntrusivePtr<VideoPlayer> VideoPlayerPtr;

/**
 * @brief VideoPlayer class is used for video playback.
 * @SINCE_1_1.24
 */
class VideoPlayer : public Dali::BaseObject
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
  void SetUrl(const std::string& url);

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
  void SetMute(bool mute);

  /**
   * @copydoc Dali::VideoPlayer::IsMuted()
   */
  bool IsMuted();

  /**
   * @copydoc Dali::VideoPlayer::SetVolume()
   */
  void SetVolume(float left, float right);

  /**
   * @copydoc Dali::VideoPlayer::GetVolume()
   */
  void GetVolume(float& left, float& right);

  /**
   * @copydoc Dali::VideoPlayer::SetRenderingTarget()
   */
  void SetRenderingTarget(Dali::Any target);

  /**
   * @copydoc Dali::VideoPlayer::SetPlayPosition()
   */
  void SetPlayPosition(int millisecond);

  /**
   * @copydoc Dali::VideoPlayer::GetPlayPosition()
   */
  int GetPlayPosition();

  /**
   * @copydoc Dali::VideoPlayer::SetDisplayArea()
   */
  void SetDisplayArea(DisplayArea area);

  /**
   * @copydoc Dali::VideoPlayer::SetSetDisplayRotation()
   */
  void SetDisplayRotation(Dali::VideoPlayerPlugin::DisplayRotation rotation);

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
  void Initialize(Dali::Actor actor, VideoSyncMode syncMode);

  /**
   * @brief Dali::VideoPlayer::Forward()
   */
  void Forward(int millisecond);

  /**
   * @brief Dali::VideoPlayer::Backward()
   */
  void Backward(int millisecond);

  /**
   * @brief Dali::VideoPlayer::IsVideoTextureSupported()
   */
  bool IsVideoTextureSupported();

  /**
   * @brief Dali::VideoPlayer::SetCodecType()
   */
  void SetCodecType(Dali::VideoPlayerPlugin::CodecType type);

  /**
   * @brief Dali::VideoPlayer::GetCodecType()
   */
  Dali::VideoPlayerPlugin::CodecType GetCodecType() const;

  /**
   * @copydoc Dali::VideoPlayer::SetDisplayMode()
   */
  void SetDisplayMode(Dali::VideoPlayerPlugin::DisplayMode::Type mode);

  /**
   * @brief Dali::VideoPlayer::GetDisplayMode()
   */
  Dali::VideoPlayerPlugin::DisplayMode::Type GetDisplayMode() const;

  /**
   * @brief Dali::VideoPlayer::GetMediaPlayer()
   */
  Any GetMediaPlayer();

  /**
   * @brief Dali::VideoPlayer::StartSynchronization()
   */
  void StartSynchronization();

  /**
   * @copydoc Dali::VideoPlayer::FinishSynchronization()
   */
  void FinishSynchronization();

  /**
   * @copydoc Dali::VideoPlayer::RaiseAbove()
   */
  void RaiseAbove(Dali::VideoPlayer target);

  /**
   * @copydoc Dali::VideoPlayer::LowerBelow()
   */
  void LowerBelow(Dali::VideoPlayer target);

  /**
   * @copydoc Dali::VideoPlayer::RaiseToTop()
   */
  void RaiseToTop();

  /**
   * @copydoc Dali::VideoPlayer::LowerToBottom()
   */
  void LowerToBottom();

  /**
   * @copydoc Dali::VideoPlayer::SetAutoRotationEnabled()
   */
  void SetAutoRotationEnabled(bool enable);

  /**
   * @copydoc Dali::VideoPlayer::IsAutoRotationEnabled() const
   */
  bool IsAutoRotationEnabled() const;

  /**
   * @copydoc Dali::VideoPlayer::SetLetterBoxEnabled()
   */
  void SetLetterBoxEnabled(bool enable);

  /**
   * @copydoc Dali::VideoPlayer::IsLetterBoxEnabled() const
   */
  bool IsLetterBoxEnabled() const;

  /**
   * @brief Sets the frame interpolation interval for smooth video playback.
   *
   * The interpolation factor will progress from 0.0 to 1.0 over this duration.
   * This interval is applied after the next call to SetVideoFrameBuffer.
   *
   * @param[in] intervalSeconds The interpolation interval in seconds
   */
  void SetFrameInterpolationInterval(float intervalSeconds);

  /**
   * @brief Enables or disables offscreen frame rendering for video interpolation.
   *
   * When enabled, the video player will use offscreen rendering for frame interpolation,
   * which can improve visual quality for certain video content.
   *
   * @param[in] useOffScreenFrame True to enable offscreen frame rendering, false to disable
   * @param[in] previousFrameBufferNativeImageSourcePtr Native image source for previous frame buffer
   * @param[in] currentFrameBufferNativeImageSourcePtr Native image source for current frame buffer
   */
  void EnableOffscreenFrameRendering(bool useOffScreenFrame, Dali::NativeImageSourcePtr previousFrameBufferNativeImageSourcePtr, Dali::NativeImageSourcePtr currentFrameBufferNativeImageSourcePtr);

  /**
   * @brief Sets the video frame buffer for rendering.
   *
   * This method sets the native image source that will be used as the frame buffer
   * for video rendering. The frame buffer contains the surface data for video playback.
   *
   * @param[in] source The video frame buffer source containing surface data
   */
  void SetVideoFrameBuffer(Dali::NativeImageSourcePtr source);

  /**
   * @brief Gets Video Player plugin
   * @SINCE_2_0.14
   * @return VideoPlayerPlugin pointer
   */
  VideoPlayerPlugin* GetVideoPlayerPlugin();

  /**
   * @copydoc Dali::VideoPlayer::SceneConnection()
   */
  void SceneConnection();

  /**
   * @copydoc Dali::VideoPlayer::SceneDisconnection()
   */
  void SceneDisconnection();

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
  VideoPlayer(const VideoPlayer& player);

  // Undefined assignment operator
  VideoPlayer& operator=(const VideoPlayer& player);

private:
  Dali::VideoPlayerPlugin* mPlugin; ///< Videoplayer plugin handle
  void*                    mHandle; ///< Handle for the loaded library

  typedef Dali::VideoPlayerPlugin* (*CreateVideoPlayerFunction)(Dali::Actor actor, Dali::VideoSyncMode syncMode);
  typedef void (*DestroyVideoPlayerFunction)(Dali::VideoPlayerPlugin* plugin);

  CreateVideoPlayerFunction  mCreateVideoPlayerPtr;
  DestroyVideoPlayerFunction mDestroyVideoPlayerPtr;

  Dali::VideoPlayerPlugin::VideoPlayerSignalType mFinishedSignal;
};

} // namespace Adaptor
} // namespace Internal

inline static Internal::Adaptor::VideoPlayer& GetImplementation(Dali::VideoPlayer& player)
{
  DALI_ASSERT_ALWAYS(player && "VideoPlayer handle is empty.");

  BaseObject& handle = player.GetBaseObject();

  return static_cast<Internal::Adaptor::VideoPlayer&>(handle);
}

inline static const Internal::Adaptor::VideoPlayer& GetImplementation(const Dali::VideoPlayer& player)
{
  DALI_ASSERT_ALWAYS(player && "VideoPlayer handle is empty.");

  const BaseObject& handle = player.GetBaseObject();

  return static_cast<const Internal::Adaptor::VideoPlayer&>(handle);
}

} // namespace Dali

#endif // DALI_VIDEO_PLAYER_IMPL_H
