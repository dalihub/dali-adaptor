#ifndef DALI_INTERNAL_TIZEN_CANVAS_RENDERER_IMPL_TIZEN_H
#define DALI_INTERNAL_TIZEN_CANVAS_RENDERER_IMPL_TIZEN_H

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
#ifdef THORVG_SUPPORT
#include <thorvg.h>
#endif
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/object/weak-handle.h>
#include <dali/public-api/rendering/texture.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>
#include <dali/internal/canvas-renderer/common/drawable-group-impl.h>

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
   * @copydoc Dali::CanvasRenderer::GetRasterizedTexture()
   */
  Dali::Texture GetRasterizedTexture() override;

  /**
   * @copydoc Dali::CanvasRenderer::AddDrawable()
   */
  bool AddDrawable(Dali::CanvasRenderer::Drawable& drawable) override;

  /**
   * @copydoc Dali::CanvasRenderer::IsCanvasChanged()
   */
  bool IsCanvasChanged() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Rasterize()
   */
  bool Rasterize() override;

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

#ifdef THORVG_SUPPORT
  /**
   * @brief Get drawables changed status.
   * If drawable is a type that can have child drawables, it is called recursively.
   * @param[in] drawable The drawable object.
   * @return Returns whether drawables have changed.
   */
  bool HaveDrawablesChanged(const Dali::CanvasRenderer::Drawable& drawable) const;

  /**
   * @brief Update drawables changed status.
   * If drawable is a type that can have child drawables, it is called recursively.
   * @param[in] drawable The drawable object.
   * @param[in] changed The state of changed.
   */
  void UpdateDrawablesChanged(Dali::CanvasRenderer::Drawable& drawable, bool changed);

  /**
   * @brief Push drawable object to parent.
   * If drawable is a type that can have child drawables, it is called recursively.
   * @param[in] drawable The drawable object.
   * @param[in] group The scene object of tvg that can be drawable group.
   */
  void PushDrawableToGroup(Dali::CanvasRenderer::Drawable& drawable, tvg::Scene* group);
#endif

private:
#ifdef THORVG_SUPPORT
  Dali::Texture                   mRasterizedTexture;
  Dali::Mutex                     mMutex;
  std::unique_ptr<tvg::SwCanvas>  mTvgCanvas;
  tvg::Scene*                     mTvgRoot;
  Dali::NativeImageSourceQueuePtr mNativeImageQueue;
#endif
  DrawableGroup::DrawableVector mDrawables;

  Vector2 mSize;
  Vector2 mViewBox;
  bool    mChanged;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TIZEN_CANVAS_RENDERER_IMPL_TIZEN_H
