#ifndef DALI_VIDEO_SOURCE_DESCRIPTOR_H
#define DALI_VIDEO_SOURCE_DESCRIPTOR_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-string.h>
#include <dali/public-api/common/dali-string-view.h>
#include <dali/public-api/common/unique-ptr.h>
#include <dali/public-api/object/any.h>
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @brief Ownership policy for a video source's native session.
 *
 * @SINCE_2_5.32
 */
enum class VideoSourceOwnership : uint32_t
{
  EXTERNAL = 0, ///< The caller owns the native session. @SINCE_2_5.32
  SHARED   = 1, ///< The native session is shared or ref-counted. @SINCE_2_5.32
  TRANSFER = 2  ///< Ownership is transferred to the video plugin. @SINCE_2_5.32
};

/**
 * @brief How a video source is rendered into the scene.
 *
 * @SINCE_2_5.32
 */
enum class VideoRenderingMode : uint32_t
{
  UNDERLAY     = 0, ///< Platform-composited hole-punch; renders beneath the UI. @SINCE_2_5.32
  NATIVE_IMAGE = 1  ///< Decoded frames become a GPU texture; supports UI render effects. @SINCE_2_5.32
};

/**
 * @brief Describes an externally created native player session, used for
 * source-based video creation (e.g. VideoPlayer::New()).
 *
 * The adaptor forwards this to the video plugin, which interprets @p providerId
 * and @p nativeSession. Implemented with a private handle so future fields can be
 * added without breaking binary compatibility.
 *
 * @SINCE_2_5.32
 */
class DALI_ADAPTOR_API VideoSourceDescriptor
{
public:
  /**
   * @brief Creates an empty descriptor.
   *
   * @SINCE_2_5.32
   */
  VideoSourceDescriptor();

  /**
   * @brief Copy constructor.
   *
   * @SINCE_2_5.32
   * @param[in] rhs The descriptor to copy
   */
  VideoSourceDescriptor(const VideoSourceDescriptor& rhs);

  /**
   * @brief Move constructor.
   *
   * @SINCE_2_5.32
   * @param[in] rhs The descriptor to move
   */
  VideoSourceDescriptor(VideoSourceDescriptor&& rhs) noexcept;

  /**
   * @brief Copy assignment operator.
   *
   * @SINCE_2_5.32
   * @param[in] rhs The descriptor to copy
   * @return A reference to this object
   */
  VideoSourceDescriptor& operator=(const VideoSourceDescriptor& rhs);

  /**
   * @brief Move assignment operator.
   *
   * @SINCE_2_5.32
   * @param[in] rhs The descriptor to move
   * @return A reference to this object
   */
  VideoSourceDescriptor& operator=(VideoSourceDescriptor&& rhs) noexcept;

  /**
   * @brief Destructor.
   *
   * @SINCE_2_5.32
   */
  ~VideoSourceDescriptor();

  /**
   * @brief Sets the provider id, interpreted by the video plugin. The descriptor
   * copies the string, so the caller does not need to keep it alive.
   *
   * @SINCE_2_5.32
   * @param[in] providerId The provider id
   */
  void SetProviderId(Dali::StringView providerId);

  /**
   * @brief Gets the provider id.
   *
   * @SINCE_2_5.32
   * @return The provider id
   */
  Dali::String GetProviderId() const;

  /**
   * @brief Sets the native player/session handle.
   *
   * @SINCE_2_5.32
   * @param[in] nativeSession The native session handle
   */
  void SetNativeSession(Any nativeSession);

  /**
   * @brief Gets the native player/session handle.
   *
   * @SINCE_2_5.32
   * @return The native session handle
   */
  const Any& GetNativeSession() const;

  /**
   * @brief Sets the native session ownership policy.
   *
   * @SINCE_2_5.32
   * @param[in] ownership The ownership policy
   */
  void SetOwnership(VideoSourceOwnership ownership);

  /**
   * @brief Gets the native session ownership policy.
   *
   * @SINCE_2_5.32
   * @return The ownership policy
   */
  VideoSourceOwnership GetOwnership() const;

  /**
   * @brief Sets how the source is rendered into the scene.
   *
   * @SINCE_2_5.32
   * @param[in] renderingMode The rendering mode
   */
  void SetRenderingMode(VideoRenderingMode renderingMode);

  /**
   * @brief Gets how the source is rendered into the scene.
   *
   * @SINCE_2_5.32
   * @return The rendering mode
   */
  VideoRenderingMode GetRenderingMode() const;

  /**
   * @brief Gets the descriptor version.
   *
   * @SINCE_2_5.32
   * @return The descriptor version
   */
  uint32_t GetVersion() const;

private:
  struct Impl;
  UniquePtr<Impl> mImpl;
};

} // namespace Dali

#endif // DALI_VIDEO_SOURCE_DESCRIPTOR_H
