#ifndef DALI_CANVAS_RENDERER_DRAWABLE_GROUP_H
#define DALI_CANVAS_RENDERER_DRAWABLE_GROUP_H

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
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class CanvasRenderer;
class DrawableGroup;
} // namespace Adaptor
} // namespace DALI_INTERNAL

/**
 * @brief A class that holds many Drawable object. As a whole they can be transformed, their transparency can be changed.
 */
class DALI_ADAPTOR_API CanvasRenderer::DrawableGroup : public CanvasRenderer::Drawable
{
public:
  /**
   * @brief Creates an initialized handle to a new CanvasRenderer::DrawableGroup.
   * @return A handle to a newly allocated DrawableGroup
   */
  static DrawableGroup New();

public:
  /**
   * @brief Creates an empty handle. Use CanvasRenderer::DrawableGroup::New() to create an initialized object.
   */
  DrawableGroup();

  /**
   * @brief Destructor.
   */
  ~DrawableGroup();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  DrawableGroup(const DrawableGroup& handle) = default;

public:
  /**
   * @brief Add drawable object to the DrawableGroup. This method is similar to registration.
   * @param[in] drawable The drawable object.
   * @return Returns True when it's successful. False otherwise.
   */
  bool AddDrawable(Drawable& drawable);

  /**
   * @brief Remove drawable object to the DrawableGroup.
   * This method is similar to deregistration. Freeing memory is not concerned for drawables being removed.
   * @param[in] drawable the drawable object.
   * @return Returns True when it's successful. False otherwise.
   */
  bool RemoveDrawable(Drawable drawable);

  /**
   * @brief Remove all drawable objects added to the DrawableGroup.
   * @return Returns True when it's successful. False otherwise.
   */
  bool RemoveAllDrawables();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated CanvasRenderer::DrawableGroup
   */
  explicit DALI_INTERNAL DrawableGroup(Internal::Adaptor::DrawableGroup* impl);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_CANVAS_RENDERER_DRAWABLE_GROUP_H
