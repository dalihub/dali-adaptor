#ifndef DALI_INTERNAL_GENERIC_CANVAS_RENDERER_IMPL_GENERIC_H
#define DALI_INTERNAL_GENERIC_CANVAS_RENDERER_IMPL_GENERIC_H

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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>
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
  static CanvasRendererGeneric* New(const Vector2& viewBox);

  /**
   * @copydoc Dali::CanvasRenderer::Commit()
   */
  bool Commit() override;

  /**
   * @copydoc Dali::CanvasRenderer::GetRasterizedTexture()
   */
  virtual Dali::Texture GetRasterizedTexture() override;

  /**
   * @copydoc Dali::CanvasRenderer::AddDrawable()
   */
  bool AddDrawable(Dali::CanvasRenderer::Drawable& drawable) override;

  /**
   * @copydoc Dali::CanvasRenderer::IsCanvasChanged()
   */
  virtual bool IsCanvasChanged() const;

  /**
   * @copydoc Dali::CanvasRenderer::Rasterize()
   */
  virtual bool Rasterize();

  /**
   * @copydoc Dali::CanvasRenderer::RemoveDrawable()
   */
  bool RemoveDrawable(Dali::CanvasRenderer::Drawable& drawable) override;

  /**
   * @copydoc Dali::CanvasRenderer::RemoveAllDrawables()
   */
  bool RemoveAllDrawables() override;

  /**
   * @copydoc Dali::CanvasRenderer::SetSize()
   */
  bool SetSize(Vector2 size) override;

  /**
   * @copydoc Dali::CanvasRenderer::GetSize()
   */
  Vector2 GetSize() const override;

  /**
   * @copydoc Dali::CanvasRenderer::SetViewBox()
   */
  bool SetViewBox(const Vector2& viewBox) override;

  /**
   * @copydoc Dali::CanvasRenderer::GetViewBox()
   */
  const Vector2& GetViewBox() override;

private:
  CanvasRendererGeneric()                             = delete;
  CanvasRendererGeneric(const CanvasRendererGeneric&) = delete;
  CanvasRendererGeneric& operator=(CanvasRendererGeneric&) = delete;
  CanvasRendererGeneric(CanvasRendererGeneric&&)           = delete;
  CanvasRendererGeneric& operator=(CanvasRendererGeneric&&) = delete;

  /**
   * @brief Constructor
   */
  CanvasRendererGeneric(const Vector2& viewBox);

  /**
   * @brief Destructor.
   */
  virtual ~CanvasRendererGeneric() override;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GENERIC_CANVAS_RENDERER_IMPL_GENERIC_H
