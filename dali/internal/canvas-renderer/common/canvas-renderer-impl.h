#ifndef DALI_INTERNAL_CANVAS_RENDERER_IMPL_H
#define DALI_INTERNAL_CANVAS_RENDERER_IMPL_H

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
#include <dali/public-api/rendering/texture.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal CanvasRenderer.
 */
class CanvasRenderer : public Dali::BaseObject
{
public:
  /**
   * @brief Constructor
   */
  CanvasRenderer();

  /**
   * @brief Destructor.
   */
  ~CanvasRenderer() override;

  /**
   * @copydoc Dali::CanvasRenderer::Commit()
   */
  virtual bool Commit();

  /**
   * @copydoc Dali::CanvasRenderer::GetRasterizedTexture()
   */
  virtual Dali::Texture GetRasterizedTexture();

  /**
   * @copydoc Dali::CanvasRenderer::AddDrawable()
   */
  virtual bool AddDrawable(Dali::CanvasRenderer::Drawable& drawable);

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
  virtual bool RemoveDrawable(Dali::CanvasRenderer::Drawable& drawable);

  /**
   * @copydoc Dali::CanvasRenderer::RemoveAllDrawables()
   */
  virtual bool RemoveAllDrawables();

  /**
   * @copydoc Dali::CanvasRenderer::SetSize()
   */
  virtual bool SetSize(Vector2 size);

  /**
   * @copydoc Dali::CanvasRenderer::GetSize()
   */
  virtual Vector2 GetSize() const;

  /**
   * @copydoc Dali::CanvasRenderer::SetViewBox()
   */
  virtual bool SetViewBox(const Vector2& viewBox);

  /**
   * @copydoc Dali::CanvasRenderer::GetViewBox()
   */
  virtual const Vector2& GetViewBox();

  CanvasRenderer(const CanvasRenderer&) = delete;
  CanvasRenderer& operator=(CanvasRenderer&) = delete;
  CanvasRenderer(CanvasRenderer&&)           = delete;
  CanvasRenderer& operator=(CanvasRenderer&&) = delete;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::CanvasRenderer& GetImplementation(Dali::CanvasRenderer& renderer)
{
  DALI_ASSERT_ALWAYS(renderer && "CanvasRenderer handle is empty.");

  BaseObject& handle = renderer.GetBaseObject();

  return static_cast<Internal::Adaptor::CanvasRenderer&>(handle);
}

inline static const Internal::Adaptor::CanvasRenderer& GetImplementation(const Dali::CanvasRenderer& renderer)
{
  DALI_ASSERT_ALWAYS(renderer && "CanvasRenderer handle is empty.");

  const BaseObject& handle = renderer.GetBaseObject();

  return static_cast<const Internal::Adaptor::CanvasRenderer&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_CANVAS_RENDERER_IMPL_H
