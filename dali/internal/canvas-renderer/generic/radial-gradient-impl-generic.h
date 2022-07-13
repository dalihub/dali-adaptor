#ifndef DALI_INTERNAL_GENERIC_RADIAL_GRADIENT_IMPL_GENERIC_H
#define DALI_INTERNAL_GENERIC_RADIAL_GRADIENT_IMPL_GENERIC_H

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
class RadialGradientGeneric : public Dali::Internal::Adaptor::RadialGradient
{
public:
  /**
   * @brief Creates a RadialGradient object.
   * @return A pointer to a newly allocated drawablegroup
   */
  static RadialGradientGeneric* New();

  /**
   * @copydoc Dali::CanvasRenderer::RadialGradient::SetBounds()
   */
  bool SetBounds(Vector2 centerPoint, float radius) override;

  /**
   * @copydoc Dali::CanvasRenderer::RadialGradient::SetBounds()
   */
  bool GetBounds(Vector2& centerPoint, float& radius) const override;

private:
  RadialGradientGeneric(const RadialGradientGeneric&) = delete;
  RadialGradientGeneric& operator=(RadialGradientGeneric&) = delete;
  RadialGradientGeneric(RadialGradientGeneric&&)           = delete;
  RadialGradientGeneric& operator=(RadialGradientGeneric&&) = delete;

  /**
   * @brief Constructor
   */
  RadialGradientGeneric();

  /**
   * @brief Destructor.
   */
  virtual ~RadialGradientGeneric() override;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GENERIC_RADIAL_GRADIENT_IMPL_GENERIC_H
