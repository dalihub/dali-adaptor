#ifndef DALI_INTERNAL_CANVAS_RENDERER_IMPL_H
#define DALI_INTERNAL_CANVAS_RENDERER_IMPL_H

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
#ifdef THORVG_SUPPORT
#include <thorvg.h>
#endif
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/weak-handle.h>
#include <dali/public-api/rendering/texture.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>
#include <dali/internal/canvas-renderer/common/drawable-group-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class CanvasRenderer;
typedef IntrusivePtr<CanvasRenderer> CanvasRendererPtr;

/**
 * Dali internal CanvasRenderer.
 */
class CanvasRenderer : public Dali::BaseObject
{
public:
  /**
   * @copydoc Dali::CanvasRenderer::Commit()
   */
  bool Commit();

  /**
   * @copydoc Dali::CanvasRenderer::GetRasterizedTexture()
   */
  Dali::Texture GetRasterizedTexture();

  /**
   * @copydoc Dali::CanvasRenderer::AddDrawable()
   */
  bool AddDrawable(Dali::CanvasRenderer::Drawable& drawable);

  /**
   * @copydoc Dali::CanvasRenderer::IsCanvasChanged()
   */
  bool IsCanvasChanged() const;

  /**
   * @copydoc Dali::CanvasRenderer::Rasterize()
   */
  bool Rasterize();

  /**
   * @copydoc Dali::CanvasRenderer::RemoveDrawable()
   */
  bool RemoveDrawable(Dali::CanvasRenderer::Drawable& drawable);

  /**
   * @copydoc Dali::CanvasRenderer::RemoveAllDrawables()
   */
  bool RemoveAllDrawables();

  /**
   * @copydoc Dali::CanvasRenderer::SetSize()
   */
  bool SetSize(Vector2 size);

  /**
   * @copydoc Dali::CanvasRenderer::GetSize()
   */
  Vector2 GetSize() const;

  /**
   * @copydoc Dali::CanvasRenderer::SetViewBox()
   */
  bool SetViewBox(const Vector2& viewBox);

  /**
   * @copydoc Dali::CanvasRenderer::GetViewBox()
   */
  const Vector2& GetViewBox();

private:
  CanvasRenderer()                            = delete;
  CanvasRenderer(const CanvasRenderer&)       = delete;
  CanvasRenderer& operator=(CanvasRenderer&)  = delete;
  CanvasRenderer(CanvasRenderer&&)            = delete;
  CanvasRenderer& operator=(CanvasRenderer&&) = delete;

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

protected:
  /**
   * @brief Constructor.
   * @param[in] viewBox The viewBox of canvas.
   */
  CanvasRenderer(const Vector2& viewBox);

  /**
   * @brief Destructor.
   */
  virtual ~CanvasRenderer();

  /**
   * @brief Initializes member data.
   * @param[in] viewBox The viewBox of canvas.
   */
  void Initialize(const Vector2& viewBox);

protected: // Seperated by platforms
  /**
   * @copydoc Dali::CanvasRenderer::GetRasterizedTexture()
   */
  virtual Dali::Texture OnGetRasterizedTexture() = 0;

  /**
   * @copydoc Dali::CanvasRenderer::Rasterize()
   */
  virtual bool OnRasterize() = 0;

  /**
   * @copydoc CanvasRenderer::MakeTargetBuffer()
   */
  virtual void OnMakeTargetBuffer(const Vector2& size) = 0;

protected:
#ifdef THORVG_SUPPORT
  Dali::Texture                  mRasterizedTexture;
  Dali::Mutex                    mMutex;
#ifdef THORVG_VERSION_1
  tvg::SwCanvas*                 mTvgCanvas;
#else
  std::unique_ptr<tvg::SwCanvas> mTvgCanvas;
#endif
  tvg::Scene*                    mTvgRoot;
#endif
  DrawableGroup::DrawableVector mDrawables;

  Vector2 mSize;
  Vector2 mViewBox;
  bool    mChanged;
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
