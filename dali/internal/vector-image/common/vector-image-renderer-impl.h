#ifndef DALI_INTERNAL_VECTOR_IMAGE_RENDERER_IMPL_H
#define DALI_INTERNAL_VECTOR_IMAGE_RENDERER_IMPL_H

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
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/devel-api/adaptor-framework/vector-image-renderer.h>
#include <dali/internal/vector-image/common/vector-image-renderer-plugin-proxy.h>

struct NSVGrasterizer;
struct NSVGimage;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class VectorImageRenderer;
using VectorImageRendererPtr = IntrusivePtr<VectorImageRenderer>;

/**
 * Dali internal VectorImageRenderer.
 */
class VectorImageRenderer : public BaseObject
{
public:
  /**
   * @brief Creates a VectorImageRenderer object.
   *
   * @return A handle to a newly allocated VectorImageRenderer
   */
  static VectorImageRendererPtr New();

  /**
   * @copydoc Dali::VectorImageRenderer::Load()
   */
  bool Load(const Vector<uint8_t>& data, float dpi);

  /**
   * @copydoc Dali::VectorImageRenderer::Rasterize()
   */
  bool Rasterize(Dali::Devel::PixelBuffer& buffer, float scale);

  /**
   * @copydoc Dali::VectorImageRenderer::GetDefaultSize()
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const;

  VectorImageRenderer(const VectorImageRenderer&) = delete;
  VectorImageRenderer& operator=(VectorImageRenderer&) = delete;
  VectorImageRenderer(VectorImageRenderer&&)           = delete;
  VectorImageRenderer& operator=(VectorImageRenderer&&) = delete;

private:
  /**
   * @brief Constructor
   */
  VectorImageRenderer();

  /**
   * @brief Destructor.
   */
  virtual ~VectorImageRenderer();

  /**
   * @brief Initializes member data.
   */
  void Initialize();

private:
  VectorImageRendererPluginProxy mPlugin;
  NSVGimage*                     mParsedImage;
  NSVGrasterizer*                mRasterizer;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::VectorImageRenderer& GetImplementation(Dali::VectorImageRenderer& renderer)
{
  DALI_ASSERT_ALWAYS(renderer && "VectorImageRenderer handle is empty.");

  BaseObject& handle = renderer.GetBaseObject();

  return static_cast<Internal::Adaptor::VectorImageRenderer&>(handle);
}

inline static const Internal::Adaptor::VectorImageRenderer& GetImplementation(const Dali::VectorImageRenderer& renderer)
{
  DALI_ASSERT_ALWAYS(renderer && "VectorImageRenderer handle is empty.");

  const BaseObject& handle = renderer.GetBaseObject();

  return static_cast<const Internal::Adaptor::VectorImageRenderer&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_VECTOR_IMAGE_RENDERER_IMPL_H
