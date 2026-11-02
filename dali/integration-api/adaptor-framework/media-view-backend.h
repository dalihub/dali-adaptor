#ifndef DALI_INTEGRATION_MEDIA_VIEW_BACKEND_H
#define DALI_INTEGRATION_MEDIA_VIEW_BACKEND_H

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
#include <dali/public-api/math/rect.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/rendering/texture.h>
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/media-frame-source-plugin.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace DALI_NAMESPACE
{
namespace Integration
{
/**
 * @brief Describes the frame and its presentation geometry for a media visual.
 *
 * An empty texture means that the view should unregister its content visual.
 * sourceCrop uses normalized, unrotated source coordinates. sourceTransform is
 * applied after cropping, followed by the user flips in display coordinates.
 * contentSize is the resulting cropped and rotated image size for aspect fitting.
 * Copying this data shares the texture; it does not extend producer frame
 * retention or guarantee GPU completion.
 */
struct MediaViewPresentation
{
  Texture             texture;                                        ///< The shared NativeImage texture, or an empty handle.
  Rect<float>         sourceCrop{0.0f, 0.0f, 1.0f, 1.0f};             ///< The crop in unrotated source coordinates.
  MediaFrameTransform sourceTransform{MediaFrameTransform::IDENTITY}; ///< The producer's source-to-display transform.
  Vector2             contentSize;                                    ///< The cropped and rotated image dimensions.
  bool                horizontalFlip{false};                          ///< Whether to reflect in display coordinates horizontally.
  bool                verticalFlip{false};                            ///< Whether to reflect in display coordinates vertically.
};

/**
 * @brief Provides the shared frame presentation backend for Toolkit and Ui MediaView.
 *
 * The view registers its own media visual from GetPresentation() and listens to
 * PresentationChangedSignal(). The backend owns NativeImage and frame storage,
 * and borrows the native source. It does not retain an owning Actor handle, avoiding a
 * view ownership cycle, and does not control producer playback or capture.
 *
 * Construct, destroy and access this object on the DALi event thread. Producer
 * callbacks are synchronized internally and schedule image updates on that thread.
 * Only the latest pending frame is kept; at most three frames applied to NativeImage
 * are retained. This retention does not check or guarantee GPU completion.
 */
class DALI_ADAPTOR_API MediaViewBackend
{
public:
  using PresentationChangedSignalType = Signal<void()>; ///< Event-thread presentation notification.

  /**
   * @brief Creates a backend with no image or source subscription.
   *
   * The backend initially treats the view as disconnected from the scene.
   */
  MediaViewBackend();

  /**
   * @brief Detaches the source, returns held frames and unloads the source plugin.
   *
   * No GPU completion wait is performed when frames are returned.
   */
  ~MediaViewBackend();

  /**
   * @brief Prevents copying frame storage and its source subscription.
   *
   * @param[in] rhs The backend that cannot be copied
   */
  MediaViewBackend(const MediaViewBackend& rhs) = delete;

  /**
   * @brief Prevents assigning frame storage and its source subscription by copy.
   *
   * @param[in] rhs The backend that cannot be copied
   */
  MediaViewBackend& operator=(const MediaViewBackend& rhs) = delete;

  /**
   * @brief Returns the current image and its presentation geometry.
   *
   * The texture is empty until a frame is applied, and after the image is cleared.
   * The owning view creates a media visual and manages its fitting and decorations.
   *
   * @return A snapshot of the event-thread presentation state
   */
  MediaViewPresentation GetPresentation() const;

  /**
   * @brief Notifies the owning view when the texture, content size or coordinates change.
   *
   * Emitted on the DALi event thread outside the producer mutex. Pixel updates to
   * the same texture do not emit this signal if presentation geometry is unchanged.
   * Disconnect before destroying the view. Source clearing and scene disconnection
   * can emit synchronously; producer RETURN_BUFFER schedules event-thread clearing.
   *
   * @return The presentation notification signal
   */
  PresentationChangedSignalType& PresentationChangedSignal();

  /**
   * @brief Replaces the current source subscription with a borrowed native source.
   *
   * Clears the previous source and held frames before attempting the connection.
   * Loads the optional source plugin on demand. Failure leaves the backend without
   * a source. Connecting does not start or stop the producer.
   *
   * @param[in] source A producer-owned native source wrapped in Any
   * @return True if the subscription succeeds; false for an empty, unsupported, unavailable or already subscribed source
   * @pre The producer must remain valid until ClearSource() or backend destruction finishes
   */
  bool SetSource(Any source);

  /**
   * @brief Returns the borrowed source of the current subscription.
   *
   * The returned Any does not transfer ownership of the native source.
   *
   * @return The connected native source wrapped in Any, or an empty Any if disconnected
   */
  Any GetSource() const;

  /**
   * @brief Detaches the source, returns all held frames and clears the presented image.
   *
   * Waits for active source callbacks to finish, but does not wait for GPU completion.
   * Calling this without a source is safe. Crop and flip settings are preserved.
   */
  void ClearSource();

  /**
   * @brief Updates whether the owning view is connected to the scene.
   *
   * Disconnection returns all held frames and clears the displayed image while
   * keeping the source subscription. Frames arriving while disconnected are
   * released immediately. Reconnection displays subsequent frames.
   *
   * @param[in] connected True on scene connection; false on scene disconnection
   */
  void SetSceneConnected(bool connected);

  /**
   * @brief Sets the normalized crop rectangle in unrotated source coordinates.
   *
   * The origin is the source's top-left. The default crop is the full frame,
   * (0, 0, 1, 1). Invalid values leave the previous crop unchanged.
   *
   * @param[in] crop A finite rectangle within [0, 1] with positive width and height
   * @return True if the crop was accepted; false if any coordinate is invalid or the rectangle exceeds the source
   */
  bool SetSourceCrop(Rect<float> crop);

  /**
   * @brief Returns the normalized crop rectangle in unrotated source coordinates.
   *
   * @return The source crop; initially (0, 0, 1, 1)
   */
  Rect<float> GetSourceCrop() const;

  /**
   * @brief Enables or disables horizontal reflection in display coordinates.
   *
   * The reflection applies to the cropped image after the producer's transform.
   *
   * @param[in] enabled True to reflect horizontally; false to disable reflection (the default)
   */
  void SetHorizontalFlipEnabled(bool enabled);

  /**
   * @brief Checks whether horizontal reflection in display coordinates is enabled.
   *
   * @return True if horizontal reflection is enabled; false otherwise
   */
  bool IsHorizontalFlipEnabled() const;

  /**
   * @brief Enables or disables vertical reflection in display coordinates.
   *
   * The reflection applies to the cropped image after the producer's transform.
   *
   * @param[in] enabled True to reflect vertically; false to disable reflection (the default)
   */
  void SetVerticalFlipEnabled(bool enabled);

  /**
   * @brief Checks whether vertical reflection in display coordinates is enabled.
   *
   * @return True if vertical reflection is enabled; false otherwise
   */
  bool IsVerticalFlipEnabled() const;

private:
  struct Impl;
  UniquePtr<Impl> mImpl;
};
} // namespace Integration
} // namespace DALI_NAMESPACE
#endif // DALI_INTEGRATION_MEDIA_VIEW_BACKEND_H
