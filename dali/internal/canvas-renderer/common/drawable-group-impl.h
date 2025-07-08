#ifndef DALI_INTERNAL_DRAWABLE_GROUP_IMPL_H
#define DALI_INTERNAL_DRAWABLE_GROUP_IMPL_H

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
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable-group.h>
#include <dali/internal/canvas-renderer/common/drawable-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class DrawableGroup;
typedef IntrusivePtr<DrawableGroup> DrawableGroupPtr;

/**
 * Dali internal DrawableGroup.
 */
class DrawableGroup : public Internal::Adaptor::Drawable
{
public:
  /**
   * @brief List of drawables.
   */
  using DrawableVector = std::vector<Dali::CanvasRenderer::Drawable>;

  /**
   * @brief Creates a DrawableGroup object.
   * @return A pointer to a newly allocated drawablegroup
   */
  static DrawableGroupPtr New();

  /**
   * @copydoc Dali::CanvasRenderer::DrawableGroup::AddDrawable()
   */
  bool AddDrawable(Dali::CanvasRenderer::Drawable& drawable);

  /**
   * @copydoc Dali::CanvasRenderer::DrawableGroup::RemoveDrawable()
   */
  bool RemoveDrawable(Dali::CanvasRenderer::Drawable drawable);

  /**
   * @copydoc Dali::CanvasRenderer::DrawableGroup::RemoveAllDrawables()
   */
  bool RemoveAllDrawables();

  /**
   * @brief Get list of drawables that added this group.
   * @return Returns list of drawables.
   */
  DrawableVector GetDrawables() const;

private:
  DrawableGroup(const DrawableGroup&)       = delete;
  DrawableGroup& operator=(DrawableGroup&)  = delete;
  DrawableGroup(DrawableGroup&&)            = delete;
  DrawableGroup& operator=(DrawableGroup&&) = delete;

  /**
   * @brief Constructor
   */
  DrawableGroup();

  /**
   * @brief Destructor.
   */
  virtual ~DrawableGroup() override;

private:
  /**
   * @brief Initializes member data.
   */
  void Initialize();

private:
#ifdef THORVG_SUPPORT
  tvg::Scene* mTvgScene;
#endif
  DrawableVector mDrawables;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::DrawableGroup& GetImplementation(Dali::CanvasRenderer::DrawableGroup& drawablegroup)
{
  DALI_ASSERT_ALWAYS(drawablegroup && "DrawableGroup handle is empty.");

  BaseObject& handle = drawablegroup.GetBaseObject();

  return static_cast<Internal::Adaptor::DrawableGroup&>(handle);
}

inline static const Internal::Adaptor::DrawableGroup& GetImplementation(const Dali::CanvasRenderer::DrawableGroup& drawablegroup)
{
  DALI_ASSERT_ALWAYS(drawablegroup && "DrawableGroup handle is empty.");

  const BaseObject& handle = drawablegroup.GetBaseObject();

  return static_cast<const Internal::Adaptor::DrawableGroup&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_DRAWABLE_GROUP_IMPL_H
