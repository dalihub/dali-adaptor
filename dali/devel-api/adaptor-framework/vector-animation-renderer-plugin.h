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
   * @brief Creates a renderer to render an vector animation file.
   *
   * @param[in] url The url of an animation file
   * @param[in] renderer The renderer used to render the image
   * @param[in] width The target image width
   * @param[in] height The target image height
   * @return True if the renderer is successfully created, false otherwise
   */
  virtual bool CreateRenderer( const std::string& url, Renderer renderer, uint32_t width, uint32_t height ) = 0;

  /**
   * @brief Starts the rendering.
   *
   * @return True if the renderer is successfully started, false otherwise
   */
  virtual bool StartRender() = 0;

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
   * @brief Gets the total number of frames of the file
   *
   * @return The total number of frames
   */
  virtual uint32_t GetTotalFrameNumber() = 0;

  /**
   * @brief Function pointer called in adaptor to create a plugin instance.
   */
  using CreateVectorAnimationRendererFunction = VectorAnimationRendererPlugin* (*)();
};

} // namespace Dali

#endif // DALI_VECTOR_ANIMATION_RENDERER_PLUGIN_H
