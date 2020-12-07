#ifndef DALI_VECTOR_IMAGE_RENDERER_PLUGIN_H
#define DALI_VECTOR_IMAGE_RENDERER_PLUGIN_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

namespace Dali
{

/**
 * VectorImageRendererPlugin is an abstract interface, used by dali-adaptor to render a vector image(SVG).
 * A concrete implementation must be created for each platform and provided as a dynamic library which
 * will be loaded at run time by the adaptor.
 */
class VectorImageRendererPlugin
{
public:
  /**
   * @brief Constructor
   */
  VectorImageRendererPlugin() {}

  /**
   * @brief Destructor
   */
  virtual ~VectorImageRendererPlugin() {}

  /**
   * @brief Load vector image data directly.
   *
   * @param[in] data The memory data of vector image
   * @return True if the load success, false otherwise.
   */
  virtual bool Load(const Vector<uint8_t>& data) = 0;

  /**
   * @brief Rasterizes the content to the target buffer synchronously.
   *
   * @param[in] buffer The target buffer
   * @return True if the rendering succeeds, false otherwise.
   */
  virtual bool Rasterize(Dali::Devel::PixelBuffer &buffer) = 0;

  /**
   * @brief Gets the default size of the file.
   *
   * @param[out] width The default width of the file
   * @param[out] height The default height of the file
   */
  virtual void GetDefaultSize( uint32_t& width, uint32_t& height ) const = 0;

  /**
   * @brief Function pointer called in adaptor to create a plugin instance.
   */
  using CreateVectorImageRendererFunction = VectorImageRendererPlugin* (*)();
};

} // namespace Dali

#endif // DALI_VECTOR_IMAGE_RENDERER_PLUGIN_H
