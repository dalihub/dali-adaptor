#ifndef DALI_INTERNAL_TIZEN_LINEAR_GRADIENT_IMPL_TIZEN_H
#define DALI_INTERNAL_TIZEN_LINEAR_GRADIENT_IMPL_TIZEN_H

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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-linear-gradient.h>
#include <dali/internal/canvas-renderer/common/linear-gradient-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal LinearGradient.
 */
class LinearGradientTizen : public Dali::Internal::Adaptor::LinearGradient
{
public:
  /**
   * @brief Creates a LinearGradient object.
   * @return A pointer to a newly allocated drawablegroup
   */
  static LinearGradientTizen* New();

  /**
   * @copydoc Dali::CanvasRenderer::LinearGradient::SetBounds()
   */
  bool SetBounds(Vector2 firstPoint, Vector2 secondPoint) override;

  /**
   * @copydoc Dali::CanvasRenderer::LinearGradient::SetBounds()
   */
  bool GetBounds(Vector2& firstPoint, Vector2& secondPoint) const override;

private:
  LinearGradientTizen(const LinearGradientTizen&) = delete;
  LinearGradientTizen& operator=(LinearGradientTizen&) = delete;
  LinearGradientTizen(LinearGradientTizen&&)           = delete;
  LinearGradientTizen& operator=(LinearGradientTizen&&) = delete;

  /**
   * @brief Constructor
   */
  LinearGradientTizen();

  /**
   * @brief Destructor.
   */
  virtual ~LinearGradientTizen() override;

private:
  /**
   * @brief Initializes member data.
   */
  void Initialize();

private:
#ifdef THORVG_SUPPORT
  tvg::LinearGradient* mTvgLinearGradient;
#endif
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TIZEN_LINEAR_GRADIENT_IMPL_TIZEN_H
