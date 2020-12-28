#ifndef DALI_INTERNAL_TIZEN_CANVAS_RENDERER_IMPL_TIZEN_H
#define DALI_INTERNAL_TIZEN_CANVAS_RENDERER_IMPL_TIZEN_H

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
#include <dali/public-api/object/weak-handle.h>
#include <thorvg.h>

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
class CanvasRendererTizen : public Dali::Internal::Adaptor::CanvasRenderer
{
public:
  /**
   * @brief Creates a CanvasRenderer object.
   * @param[in] viewBox The viewBox of canvas.
   * @return A pointer to a newly allocated CanvasRenderer
   */
  static CanvasRendererTizen* New(const Vector2& viewBox);

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
  CanvasRendererTizen()                           = delete;
  CanvasRendererTizen(const CanvasRendererTizen&) = delete;
  CanvasRendererTizen& operator=(CanvasRendererTizen&) = delete;
  CanvasRendererTizen(CanvasRendererTizen&&)           = delete;
  CanvasRendererTizen& operator=(CanvasRendererTizen&&) = delete;

  /**
   * @brief Constructor
   */
  CanvasRendererTizen(const Vector2& viewBox);

  /**
   * @brief Destructor.
   */
  virtual ~CanvasRendererTizen() override;

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

private:
  Devel::PixelBuffer mPixelBuffer;

  std::unique_ptr<tvg::SwCanvas>                                   mTvgCanvas;
  tvg::Scene*                                                      mTvgRoot;
  typedef std::vector<WeakHandle<Dali::CanvasRenderer::Drawable> > DrawableVector;
  typedef DrawableVector::iterator                                 DrawableVectorIterator;
  DrawableVector                                                   mDrawables;

  Vector2 mSize;
  Vector2 mViewBox;
  bool    mChanged;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TIZEN_CANVAS_RENDERER_IMPL_TIZEN_H
