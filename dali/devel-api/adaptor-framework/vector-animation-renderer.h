#ifndef DALI_VECTOR_ANIMATION_RENDERER_H
#define DALI_VECTOR_ANIMATION_RENDERER_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/rendering/renderer.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class VectorAnimationRenderer;
}
} // namespace DALI_INTERNAL

/**
 * @brief Used for rendering a vector animation file
 */
class DALI_ADAPTOR_API VectorAnimationRenderer : public BaseHandle
{
public:
  enum class VectorProperty
  {
    FILL_COLOR,         ///< Fill color of the object, Type Property::VECTOR3
    FILL_OPACITY,       ///< Fill opacity of the object, Type Property::FLOAT
    STROKE_COLOR,       ///< Stroke color of the object, Type Property::VECTOR3
    STROKE_OPACITY,     ///< Stroke opacity of the object, Type Property::FLOAT
    STROKE_WIDTH,       ///< Stroke width of the object, Type Property::FLOAT
    TRANSFORM_ANCHOR,   ///< Transform anchor of the Layer and Group object, Type Property::VECTOR2
    TRANSFORM_POSITION, ///< Transform position of the Layer and Group object, Type Property::VECTOR2
    TRANSFORM_SCALE,    ///< Transform scale of the Layer and Group object, Type Property::VECTOR2 [0..100]
    TRANSFORM_ROTATION, ///< Transform rotation of the Layer and Group object, Type Property::FLOAT [0..360] in degrees
    TRANSFORM_OPACITY,  ///< Transform opacity of the Layer and Group object, Type Property::FLOAT
    TRIM_START,         ///< Trim Start property of Shape object , value type is float [ 0 .. 100] */
    TRIM_END            ///< Trim End property of Shape object , value type Property::VECTOR2 [0..100] */
  };

  /// @brief UploadCompleted signal type.
  using UploadCompletedSignalType = Signal<void()>;

  /**
   * @brief Creates an initialized handle to a new VectorAnimationRenderer.
   *
   * @return A handle to a newly allocated VectorAnimationRenderer
   */
  static VectorAnimationRenderer New();

  /**
   * @brief Creates an empty handle.
   * Use VectorAnimationRenderer::New() to create an initialized object.
   */
  VectorAnimationRenderer();

  /**
   * @brief Destructor.
   */
  ~VectorAnimationRenderer();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  VectorAnimationRenderer(const VectorAnimationRenderer& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param[in] rhs A reference to the copied handle
   * @return A reference to this
   */
  VectorAnimationRenderer& operator=(const VectorAnimationRenderer& rhs);

  /**
   * @brief Finalizes the renderer.
   */
  void Finalize();

  /**
   * @brief Loads the animation file.
   *
   * @param[in] url The url of the vector animation file
   * @return True if loading success, false otherwise.
   */
  bool Load(const std::string& url);

  /**
   * @brief Loads the animation file by buffer.
   *
   * @param[in] data The raw buffer of the vector animation file
   * @return True if loading success, false otherwise.
   */
  bool Load(const Dali::Vector<uint8_t>& data);

  /**
   * @brief Sets the renderer used to display the result image.
   *
   * @param[in] renderer The renderer used to display the result image
   */
  void SetRenderer(Renderer renderer);

  /**
   * @brief Sets the target image size.
   *
   * @param[in] width The target image width
   * @param[in] height The target image height
   */
  void SetSize(uint32_t width, uint32_t height);

  /**
   * @brief Renders the content to the target buffer synchronously.
   *
   * @param[in] frameNumber The frame number to be rendered
   * @return True if the rendering success, false otherwise.
   */
  bool Render(uint32_t frameNumber);

  /**
   * @brief Notify the Renderer that rendering is stopped.
   */
  void RenderStopped();

  /**
   * @brief Gets the total number of frames of the file
   *
   * @return The total number of frames
   */
  uint32_t GetTotalFrameNumber() const;

  /**
   * @brief Gets the frame rate of the file.
   *
   * @return The frame rate of the file
   */
  float GetFrameRate() const;

  /**
   * @brief Gets the default size of the file.
   *
   * @param[out] width The default width of the file
   * @param[out] height The default height of the file
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const;

  /**
   * @brief Gets the layer information of all the child layers.
   *
   * @param[out] map The layer information
   */
  void GetLayerInfo(Property::Map& map) const;

  /**
   * @brief Gets the start frame and the end frame number of the composition marker.
   *
   * @param[in] marker The composition marker of the file
   * @param[out] startFrame The start frame number of the specified marker
   * @param[out] endFrame The end frame number of the specified marker
   * @return True if the marker is found in the file, false otherwise.
   *
   * @note https://helpx.adobe.com/after-effects/using/layer-markers-composition-markers.html
   * Markers exported from AfterEffect are used to describe a segment of an animation {comment/tag , startFrame, endFrame}
   * Marker can be use to devide a resource in to separate animations by tagging the segment with comment string,
   * start frame and duration of that segment.
   */
  bool GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const;

  /**
   * @brief Gets the all composition marker informations.
   *
   * @param[out] map The marker information
   */
  void GetMarkerInfo(Property::Map& map) const;

  /**
   * @brief Invalidates the rendered buffer.
   * @note The upload completed signal will be emitted again.
   */
  void InvalidateBuffer();

  /**
   * @brief Sets property value for the specified keyPath. This keyPath can resolve to multiple contents.
   * In that case, the callback's value will apply to all of them.
   *
   * @param[in] keyPath The key path used to target a specific content or a set of contents that will be updated.
   * @param[in] property The property to set.
   * @param[in] callback The callback that gets called every time the animation is rendered.
   * @param[in] id The Id to specify the callback. It should be unique and will be passed when the callback is called.
   *
   * @note A callback of the following type may be used:
   * id  The id to specify the callback.
   * property The property that represent what you are trying to change.
   * frameNumber The current frame number.
   * It returns a Property::Value to set according to the property type.
   *
   * @code
   *   Property::Value MyFunction(int32_t id, VectorProperty property, uint32_t frameNumber);
   * @endcode
   *
   * The keypath should contain object names separated by (.) and can handle globe(**) or wildchar(*).
   * Ownership of the callback is passed onto this class.
   */
  void AddPropertyValueCallback(const std::string& keyPath, VectorProperty property, CallbackBase* callback, int32_t id);

  /**
   * @brief Keep the rasterized buffer to use the frame cache.
   *
   * By default, only as much as needed is kept in the rasterized buffer and not kept after use.
   * tis API is efficient in terms of memory, but has the disadvantage of having to load the necessary buffer each time.
   * therefore, if there are cases in which you want to improve the performance even if the app sacrifices a lot of memory, this API is useful.
   */
  void KeepRasterizedBuffer();

public: // Signals
  /**
   * @brief Connect to this signal to be notified when the texture upload is completed.
   *
   * @return The signal to connect to.
   */
  UploadCompletedSignalType& UploadCompletedSignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated VectorAnimationRenderer
   */
  explicit DALI_INTERNAL VectorAnimationRenderer(Internal::Adaptor::VectorAnimationRenderer* internal);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_VECTOR_ANIMATION_RENDERER_H
