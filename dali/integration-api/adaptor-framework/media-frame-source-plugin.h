#ifndef DALI_INTEGRATION_MEDIA_FRAME_SOURCE_PLUGIN_H
#define DALI_INTEGRATION_MEDIA_FRAME_SOURCE_PLUGIN_H

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
#include <dali/public-api/common/unique-ptr.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/object/any.h>
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace DALI_NAMESPACE
{
namespace Integration
{
/**
 * @brief Describes the producer's transform from source pixels to the displayed image.
 *
 * Source coordinates start at the top-left, with x increasing to the right and y
 * increasing downwards. Rotation angles are clockwise. Each value describes one
 * transform; the values are not flags and cannot be combined.
 */
enum class MediaFrameTransform : uint32_t
{
  IDENTITY        = 0, ///< Displays source pixels without a transform.
  ROTATE_90       = 1, ///< Rotates the source image 90 degrees clockwise.
  ROTATE_180      = 2, ///< Rotates the source image 180 degrees.
  ROTATE_270      = 3, ///< Rotates the source image 270 degrees clockwise.
  HORIZONTAL_FLIP = 4, ///< Reflects the source image horizontally.
  VERTICAL_FLIP   = 5  ///< Reflects the source image vertically.
};

/**
 * @brief Owns a producer frame and its release callback through move-only semantics.
 *
 * The surface and metadata describe the image; the release callback returns the
 * underlying producer resource. Moving transfers that responsibility and leaves
 * the original frame empty. Destruction calls a non-null release callback exactly
 * once, including when a frame is discarded without being displayed.
 *
 * The callback and its data must remain valid until the frame is released. Release
 * can occur on a producer callback thread or the DALi event thread, so the callback
 * must support both. This class does not wait for GPU completion before release.
 */
class DALI_ADAPTOR_API MediaFrame
{
public:
  /**
   * @brief Returns a producer resource using the data supplied when its frame was created.
   *
   * The callback must not throw and must remain callable until the frame is released.
   */
  using ReleaseFunction = void (*)(void*);

  /**
   * @brief Creates an empty frame with no producer resource to release.
   */
  MediaFrame();

  /**
   * @brief Takes responsibility for returning a producer frame through its release callback.
   *
   * No additional producer reference is acquired. The producer resource must keep
   * the surface valid until release is called.
   *
   * @param[in] surface A native surface wrapped in Any; the Tizen plugin supplies a tbm_surface_h
   * @param[in] size The unrotated source width and height in pixels
   * @param[in] transform The producer's transform for this frame
   * @param[in] release The callback used to return the producer resource, or nullptr for no callback
   * @param[in] data The release callback's opaque argument; nullptr is passed through unchanged
   */
  MediaFrame(Any surface, Vector2 size, MediaFrameTransform transform, ReleaseFunction release, void* data);

  /**
   * @brief Calls the release callback for the owned frame, if one was supplied.
   */
  ~MediaFrame();

  /**
   * @brief Transfers frame ownership and leaves the original frame empty.
   *
   * @param[in,out] rhs The frame whose ownership is transferred
   */
  MediaFrame(MediaFrame&& rhs) noexcept;

  /**
   * @brief Releases the current frame and transfers ownership from another frame.
   *
   * @param[in,out] rhs The frame whose ownership is transferred, leaving it empty
   * @return A reference to this frame
   */
  MediaFrame& operator=(MediaFrame&& rhs) noexcept;

  /**
   * @brief Prevents copying producer-frame ownership.
   *
   * @param[in] rhs The frame that cannot be copied
   */
  MediaFrame(const MediaFrame& rhs) = delete;

  /**
   * @brief Prevents assigning producer-frame ownership by copy.
   *
   * @param[in] rhs The frame that cannot be copied
   */
  MediaFrame& operator=(const MediaFrame& rhs) = delete;

  /**
   * @brief Returns the native surface without transferring producer-frame ownership.
   *
   * The returned native handle borrows the lifetime of the producer resource.
   *
   * @return The surface wrapped in Any, or an empty Any for an empty frame
   */
  Any GetSurface() const;

  /**
   * @brief Returns the source dimensions before rotation or crop is applied.
   *
   * @return The width and height in pixels, or Vector2::ZERO for an empty frame
   */
  Vector2 GetSize() const;

  /**
   * @brief Returns the producer's transform for this frame.
   *
   * @return The frame transform, or MediaFrameTransform::IDENTITY for an empty frame
   */
  MediaFrameTransform GetTransform() const;

  /**
   * @brief Checks whether the frame contains a non-empty surface Any.
   *
   * This does not validate the native handle stored inside the Any.
   *
   * @return True if a surface Any is present; false for an empty or moved-from frame
   */
  explicit operator bool() const;

private:
  struct Impl;
  UniquePtr<Impl> mImpl;
};

/**
 * @brief Receives producer frames and buffer-return requests from a source plugin.
 *
 * Callbacks may arrive on producer threads, so implementations must synchronize
 * their frame storage. The consumer must remain alive until the source plugin's
 * Detach() returns. Callback handling must not depend on the DALi event thread;
 * renderer changes can be scheduled asynchronously on that thread.
 */
class MediaFrameConsumer
{
public:
  /**
   * @brief Provides polymorphic destruction after the source has been detached.
   */
  virtual ~MediaFrameConsumer() = default;

  /**
   * @brief Transfers a producer frame to the consumer for presentation or release.
   *
   * A consumer that cannot accept the frame must release it by allowing the frame
   * to be destroyed. The consumer is responsible for releasing frames it retains.
   *
   * @param[in] frame The producer frame whose ownership is transferred to the consumer
   */
  virtual void OnFrame(MediaFrame frame) = 0;

  /**
   * @brief Returns all pending and retained producer frames before the callback returns.
   *
   * This callback must not wait for the DALi event thread or GPU completion.
   * Clearing the displayed image can be scheduled separately on the event thread.
   */
  virtual void OnReturnBuffer() = 0;
};

/**
 * @brief Adapts one borrowed native source to the MediaFrameConsumer interface.
 *
 * Attach() and Detach() are called sequentially on the DALi event thread, while
 * frame and event callbacks may execute on producer threads. The producer owns
 * the source; the plugin owns only its callback subscription. Detach the plugin
 * before destroying the producer or the consumer.
 *
 * The source is wrapped in Any using the native type supported by the plugin.
 * A source already subscribed by another instance of the plugin is rejected.
 */
class MediaFrameSourcePlugin
{
public:
  /**
   * @brief Provides polymorphic destruction for the source plugin.
   *
   * The owner must detach the subscription before destroying the plugin.
   */
  virtual ~MediaFrameSourcePlugin() = default;

  /**
   * @brief Replaces the current subscription with callbacks for the supplied source.
   *
   * The previous subscription is detached before connection is attempted. Failure
   * leaves the plugin detached. The source and consumer are borrowed and must remain
   * valid until Detach() returns. Callbacks may begin before Attach() returns.
   *
   * @param[in] source A producer-owned native source wrapped in Any
   * @param[in] consumer The consumer retained by reference for this subscription
   * @return True if callbacks were registered; false if the source cannot be subscribed
   */
  virtual bool Attach(Any source, MediaFrameConsumer& consumer) = 0;

  /**
   * @brief Removes the subscription and waits for active producer callbacks to finish.
   *
   * No callbacks may access the consumer after this method returns. Calling it
   * without a subscription is safe. Frames already transferred to the consumer
   * remain its responsibility and are not returned by Detach() itself.
   */
  virtual void Detach() = 0;
};

/**
 * @brief Defines the plugin factory loaded from the CreateMediaFrameSourcePlugin C symbol.
 *
 * The returned instance is owned by the caller and must be destroyed with the
 * matching DestroyMediaFrameSourcePlugin function.
 */
using CreateMediaFrameSourcePlugin = MediaFrameSourcePlugin* (*)();

/**
 * @brief Defines the destructor loaded from the DestroyMediaFrameSourcePlugin C symbol.
 *
 * The instance must be detached before destruction. All frames returned by the
 * plugin must be released before its shared library is unloaded, because their
 * release callbacks may execute code from that library.
 */
using DestroyMediaFrameSourcePlugin = void (*)(MediaFrameSourcePlugin*);
} // namespace Integration
} // namespace DALI_NAMESPACE
#endif // DALI_INTEGRATION_MEDIA_FRAME_SOURCE_PLUGIN_H
