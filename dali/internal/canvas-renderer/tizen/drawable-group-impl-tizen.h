#ifndef DALI_INTERNAL_TIZEN_DRAWABLE_GROUP_IMPL_TIZEN_H
#define DALI_INTERNAL_TIZEN_DRAWABLE_GROUP_IMPL_TIZEN_H

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
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/weak-handle.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable-group.h>
#include <dali/internal/canvas-renderer/common/drawable-group-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal DrawableGroup.
 */
class DrawableGroupTizen : public Dali::Internal::Adaptor::DrawableGroup
{
public:
  /**
   * @brief Creates a DrawableGroup object.
   * @return A pointer to a newly allocated drawablegroup
   */
  static DrawableGroupTizen* New();

  /**
   * @copydoc Dali::CanvasRenderer::DrawableGroup::AddDrawable()
   */
  bool AddDrawable(Dali::CanvasRenderer::Drawable& drawable) override;

  /**
   * @copydoc Dali::CanvasRenderer::DrawableGroup::RemoveDrawable()
   */
  bool RemoveDrawable(Dali::CanvasRenderer::Drawable drawable) override;

  /**
   * @copydoc Dali::CanvasRenderer::DrawableGroup::RemoveAllDrawables()
   */
  bool RemoveAllDrawables() override;

  /**
   * @copydoc Dali::CanvasRenderer::DrawableGroup::GetDrawables()
   */
  DrawableVector GetDrawables() const override;

private:
  DrawableGroupTizen(const DrawableGroupTizen&) = delete;
  DrawableGroupTizen& operator=(DrawableGroupTizen&) = delete;
  DrawableGroupTizen(DrawableGroupTizen&&)           = delete;
  DrawableGroupTizen& operator=(DrawableGroupTizen&&) = delete;

  /**
   * @brief Constructor
   */
  DrawableGroupTizen();

  /**
   * @brief Destructor.
   */
  virtual ~DrawableGroupTizen() override;

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

} // namespace Dali

#endif // DALI_INTERNAL_TIZEN_DRAWABLE_GROUP_IMPL_TIZEN_H
