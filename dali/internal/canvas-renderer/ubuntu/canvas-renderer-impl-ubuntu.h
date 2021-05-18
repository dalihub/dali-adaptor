#ifndef DALI_INTERNAL_UBUNTU_CANVAS_RENDERER_IMPL_UBUNTU_H
#define DALI_INTERNAL_UBUNTU_CANVAS_RENDERER_IMPL_UBUNTU_H

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
#ifdef THORVG_SUPPORT
#include <thorvg.h>
#endif
#include <dali/public-api/object/weak-handle.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer-drawable.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
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
class CanvasRendererUbuntu : public Dali::Internal::Adaptor::CanvasRenderer
{
public:
  /**
   * @brief Creates a CanvasRenderer object.
   * @param[in] viewBox The viewBox of canvas.
   * @return A pointer to a newly allocated CanvasRenderer
   */
  static CanvasRendererUbuntu* New(const Vector2& viewBox);

  /**
   * @copydoc Dali::CanvasRenderer::Commit()
   */
  bool Commit() override;

  /**
   * @copydoc Dali::CanvasRenderer::GetPixelBuffer()
   */
  Devel::PixelBuffer GetPixelBuffer() override;

  /**
   * @copydoc Dali::CanvasRenderer::AddDrawable()
   */
  bool AddDrawable(Dali::CanvasRenderer::Drawable& drawable) override;

  /**
   * @copydoc Dali::CanvasRenderer::SetSize()
   */
  bool SetSize(const Vector2& size) override;

  /**
   * @copydoc Dali::CanvasRenderer::GetSize()
   */
  const Vector2& GetSize() override;

private:
  CanvasRendererUbuntu()                            = delete;
  CanvasRendererUbuntu(const CanvasRendererUbuntu&) = delete;
  CanvasRendererUbuntu& operator=(CanvasRendererUbuntu&) = delete;
  CanvasRendererUbuntu(CanvasRendererUbuntu&&)           = delete;
  CanvasRendererUbuntu& operator=(CanvasRendererUbuntu&&) = delete;

  /**
   * @brief Constructor
   */
  CanvasRendererUbuntu(const Vector2& viewBox);

  /**
   * @brief Destructor.
   */
  virtual ~CanvasRendererUbuntu() override;

  /**
   * @brief Initializes member data.
   * @param[in] viewBox The viewBox of canvas.
   */
  void Initialize(const Vector2& viewBox);

  /**
   * @brief Create and set target buffer
   * @param[in] viewBox The size of buffer.
   */
  void MakeTargetBuffer(const Vector2& size);

#ifdef THORVG_SUPPORT
  /**
   * @brief Push drawable object to parent.
   * If drawable is a type that can have child drawables, it is called recursively.
   * @param[in] drawable The drawable object.
   * @param[in] parent The scene object of tvg that can be parent.
   */
  void PushDrawableToParent(Dali::CanvasRenderer::Drawable& drawable, tvg::Scene* parent);
#endif

private:
  Devel::PixelBuffer mPixelBuffer;

#ifdef THORVG_SUPPORT
  std::unique_ptr<tvg::SwCanvas> mTvgCanvas;
  tvg::Scene*                    mTvgRoot;
#endif
  using DrawableVector = std::vector<Dali::CanvasRenderer::Drawable>;
  DrawableVector mDrawables;

  Vector2 mSize;
  Vector2 mViewBox;
  bool    mChanged;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_UBUNTU_CANVAS_RENDERER_IMPL_UBUNTU_H
