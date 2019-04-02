#ifndef DALI_VECTOR_ANIMATION_RENDERER_PLUGIN_H
#define DALI_VECTOR_ANIMATION_RENDERER_PLUGIN_H

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
#include <dali/public-api/rendering/renderer.h>
#include <string>

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

  /**
   * @brief Constructor
   */
  VectorAnimationRendererPlugin() {}

  /**
   * @brief Destructor
   */
  virtual ~VectorAnimationRendererPlugin() {}

  /**
   * @brief Second-phase constructor.
   *
   * @param[in] url The url of the animation file
   */
  virtual bool Initialize( const std::string& url ) = 0;

  /**
   * @brief Sets the renderer used to display the result image.
   *
   * @param[in] renderer The renderer used to display the result image
   */
  virtual void SetRenderer( Renderer renderer ) = 0;

  /**
   * @brief Sets the target image size.
   *
   * @param[in] width The target image width
   * @param[in] height The target image height
   */
  virtual void SetSize( uint32_t width, uint32_t height ) = 0;

  /**
   * @brief Stops the rendering.
   */
  virtual void StopRender() = 0;

  /**
   * @brief Renders the content to the target buffer synchronously.
   *
   * @param[in] frameNumber The frame number to be rendered
   */
  virtual void Render( uint32_t frameNumber ) = 0;

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
   * @brief Gets the default size of the file,.
   *
   * @return The default size of the file
   */
  virtual void GetDefaultSize( uint32_t& width, uint32_t& height ) const = 0;

  /**
   * @brief Function pointer called in adaptor to create a plugin instance.
   */
  using CreateVectorAnimationRendererFunction = VectorAnimationRendererPlugin* (*)();
};

} // namespace Dali

#endif // DALI_VECTOR_ANIMATION_RENDERER_PLUGIN_H
