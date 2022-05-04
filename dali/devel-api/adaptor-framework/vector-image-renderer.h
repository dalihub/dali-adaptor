#ifndef DALI_VECTOR_IMAGE_RENDERER_H
#define DALI_VECTOR_IMAGE_RENDERER_H

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
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal::Adaptor
{
class VectorImageRenderer;
} // namespace Internal::Adaptor

/**
 * @brief Used for rendering a vector image file (SVG)
 */
class DALI_ADAPTOR_API VectorImageRenderer : public BaseHandle
{
public:
  /**
   * @brief Creates an initialized handle to a new VectorImageRenderer.
   *
   * @return A handle to a newly allocated VectorImageRenderer
   */
  static VectorImageRenderer New();

  /**
   * @brief Creates an empty handle.
   * Use VectorImageRenderer::New() to create an initialized object.
   */
  VectorImageRenderer();

  /**
   * @brief Destructor.
   */
  ~VectorImageRenderer();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  VectorImageRenderer(const VectorImageRenderer& handle) = default;

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param[in] rhs A reference to the copied handle
   * @return A reference to this
   */
  VectorImageRenderer& operator=(const VectorImageRenderer& rhs) = default;

  /**
   * @brief Load vector image data directly.
   *
   * @param[in] data SVG image data to load.
   * @param[in] dpi The DPI of the screen.
   * @return True if the load success, false otherwise.
   */
  bool Load(const Vector<uint8_t>& data, float dpi);

  /**
   * @brief Rasterizes the content to the pixel buffer synchronously.
   *
   * @param[in] width The pixel buffer width
   * @param[in] height The pixel buffer height
   * @return The handle to the rasterized PixelBuffer object or an empty handle in case failed.
   */
  Dali::Devel::PixelBuffer Rasterize(uint32_t width, uint32_t height);

  /**
   * @brief Gets the default size of the file.
   *
   * @param[out] width The default width of the file
   * @param[out] height The default height of the file
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const;

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated VectorImageRenderer
   */
  explicit DALI_INTERNAL VectorImageRenderer(Internal::Adaptor::VectorImageRenderer* internal);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_VECTOR_IMAGE_RENDERER_H
