#ifndef DALI_INTERNAL_GENERIC_CANVAS_RENDERER_IMPL_GENERIC_H
#define DALI_INTERNAL_GENERIC_CANVAS_RENDERER_IMPL_GENERIC_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal CanvasRenderer.
 */
class CanvasRendererGeneric : public Dali::Internal::Adaptor::CanvasRenderer
{
public:
  /**
   * @brief Creates a CanvasRenderer object.
   * @param[in] viewBox The viewBox of canvas.
   * @return A pointer to a newly allocated CanvasRenderer
   */
  static CanvasRendererPtr New(const Vector2& viewBox);

protected:
  /**
   * @copydoc Dali::CanvasRenderer::GetRasterizedTexture()
   */
  Dali::Texture OnGetRasterizedTexture() override;

  /**
   * @copydoc Dali::CanvasRenderer::Rasterize()
   */
  bool OnRasterize() override;

  /**
   * @copydoc CanvasRenderer::OnMakeTargetBuffer()
   */
  void OnMakeTargetBuffer(const Vector2& size) override;

private:
  /**
   * @brief Constructor.
   * @param[in] viewBox The viewBox of canvas.
   */
  CanvasRendererGeneric(const Vector2& viewBox);

  /**
   * @brief Destructor.
   */
  virtual ~CanvasRendererGeneric() = default;

private:
#ifdef THORVG_SUPPORT
  Devel::PixelBuffer mPixelBuffer;
#endif
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GENERIC_CANVAS_RENDERER_IMPL_GENERIC_H
