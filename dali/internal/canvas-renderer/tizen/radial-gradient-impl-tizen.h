#ifndef DALI_INTERNAL_TIZEN_RADIAL_GRADIENT_IMPL_TIZEN_H
#define DALI_INTERNAL_TIZEN_RADIAL_GRADIENT_IMPL_TIZEN_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-radial-gradient.h>
#include <dali/internal/canvas-renderer/common/radial-gradient-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal RadialGradient.
 */
class RadialGradientTizen : public Dali::Internal::Adaptor::RadialGradient
{
public:
  /**
   * @brief Creates a RadialGradient object.
   * @return A pointer to a newly allocated drawablegroup
   */
  static RadialGradientTizen* New();

  /**
   * @copydoc Dali::CanvasRenderer::RadialGradient::SetBounds()
   */
  bool SetBounds(Vector2 centerPoint, float radius) override;

  /**
   * @copydoc Dali::CanvasRenderer::RadialGradient::SetBounds()
   */
  bool GetBounds(Vector2& centerPoint, float& radius) const override;

private:
  RadialGradientTizen(const RadialGradientTizen&) = delete;
  RadialGradientTizen& operator=(RadialGradientTizen&) = delete;
  RadialGradientTizen(RadialGradientTizen&&)           = delete;
  RadialGradientTizen& operator=(RadialGradientTizen&&) = delete;

  /**
   * @brief Constructor
   */
  RadialGradientTizen();

  /**
   * @brief Destructor.
   */
  virtual ~RadialGradientTizen() override;

private:
  /**
   * @brief Initializes member data.
   */
  void Initialize();

private:
#ifdef THORVG_SUPPORT
  tvg::RadialGradient* mTvgRadialGradient;
#endif
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TIZEN_RADIAL_GRADIENT_IMPL_TIZEN_H
