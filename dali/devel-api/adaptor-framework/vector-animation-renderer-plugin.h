#ifndef DALI_VECTOR_ANIMATION_RENDERER_PLUGIN_H
#define DALI_VECTOR_ANIMATION_RENDERER_PLUGIN_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/rendering/renderer.h>
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/vector-animation-renderer.h>

namespace Dali
{
/**
 * VectorAnimationRendererPlugin is an abstract interface, used by dali-adaptor to render a vector animation.
 * A concrete implementation must be created for each platform and provided as a dynamic library which
 * will be loaded at run time by the adaptor.
 */
class VectorAnimationRendererPlugin
{
public:
  using UploadCompletedSignalType = Dali::VectorAnimationRenderer::UploadCompletedSignalType;
  using VectorProperty            = Dali::VectorAnimationRenderer::VectorProperty;

  /**
   * @brief Constructor
   */
  VectorAnimationRendererPlugin()
  {
  }

  /**
   * @brief Destructor
   */
  virtual ~VectorAnimationRendererPlugin()
  {
  }

  /**
   * @brief Finalizes the renderer. It will be called in the main thread.
   */
  virtual void Finalize() = 0;

  /**
   * @brief Loads the animation file.
   *
   * @param[in] url The url of the vector animation file
   * @return True if loading success, false otherwise.
   */
  virtual bool Load(const std::string& url) = 0;

  /**
   * @brief Sets the renderer used to display the result image.
   *
   * @param[in] renderer The renderer used to display the result image
   */
  virtual void SetRenderer(Renderer renderer) = 0;

  /**
   * @brief Sets the target image size.
   *
   * @param[in] width The target image width
   * @param[in] height The target image height
   */
  virtual void SetSize(uint32_t width, uint32_t height) = 0;

  /**
   * @brief Renders the content to the target buffer synchronously.
   *
   * @param[in] frameNumber The frame number to be rendered
   * @return True if the rendering success, false otherwise.
   */
  virtual bool Render(uint32_t frameNumber) = 0;

  /**
   * @brief Notify the Renderer that rendering is stopped.
   */
  virtual void RenderStopped() = 0;

  /**
   * @brief Gets the total number of frames of the file.
   *
   * @return The total number of frames
   */
  virtual uint32_t GetTotalFrameNumber() const = 0;

  /**
   * @brief Gets the frame rate of the file.
   *
   * @return The frame rate of the file
   */
  virtual float GetFrameRate() const = 0;

  /**
   * @brief Gets the default size of the file.
   *
   * @param[out] width The default width of the file
   * @param[out] height The default height of the file
   */
  virtual void GetDefaultSize(uint32_t& width, uint32_t& height) const = 0;

  /**
   * @brief Gets the layer information of all the child layers.
   *
   * @param[out] map The layer information
   */
  virtual void GetLayerInfo(Property::Map& map) const = 0;

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
  virtual bool GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const = 0;

  /**
   * @brief Invalidates the rendered buffer.
   * @note The upload completed signal will be emitted again.
   */
  virtual void InvalidateBuffer() = 0;

  /**
   * @brief Sets property value for the specified keyPath. This keyPath can resolve to multiple contents.
   * In that case, the callback's value will apply to all of them.
   *
   * @param[in] keyPath The key path used to target a specific content or a set of contents that will be updated.
   * @param[in] property The property to set.
   * @param[in] callback The callback what gets called every time the animation is rendered.
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
   * The keypath should conatin object names separated by (.) and can handle globe(**) or wildchar(*).
   * Ownership of the callback is passed onto this class.
   */
  virtual void AddPropertyValueCallback(const std::string& keyPath, VectorProperty property, CallbackBase* callback, int32_t id) = 0;

  /**
   * @brief Connect to this signal to be notified when the texture upload is completed.
   *
   * @return The signal to connect to.
   */
  virtual UploadCompletedSignalType& UploadCompletedSignal() = 0;

  /**
   * @brief Function pointer called in adaptor to create a plugin instance.
   */
  using CreateVectorAnimationRendererFunction = VectorAnimationRendererPlugin* (*)();
};

} // namespace Dali

#endif // DALI_VECTOR_ANIMATION_RENDERER_PLUGIN_H
