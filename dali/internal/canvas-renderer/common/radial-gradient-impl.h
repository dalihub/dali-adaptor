#ifndef DALI_INTERNAL_RADIAL_GRADIENT_IMPL_H
#define DALI_INTERNAL_RADIAL_GRADIENT_IMPL_H

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
#include <dali/internal/canvas-renderer/common/gradient-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal RadialGradient.
 */
class RadialGradient : public Internal::Adaptor::Gradient
{
public:
  /**
   * @brief Constructor
   */
  RadialGradient();

  /**
   * @brief Destructor.
   */
  ~RadialGradient() override;

  /**
   * @copydoc Dali::CanvasRenderer::RadialGradient::SetBounds()
   */
  virtual bool SetBounds(Vector2 centerPoint, float radius);

  /**
   * @copydoc Dali::CanvasRenderer::RadialGradient::SetBounds()
   */
  virtual bool GetBounds(Vector2& centerPoint, float& radius) const;

  RadialGradient(const RadialGradient&) = delete;
  RadialGradient& operator=(RadialGradient&) = delete;
  RadialGradient(RadialGradient&&)           = delete;
  RadialGradient& operator=(RadialGradient&&) = delete;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::RadialGradient& GetImplementation(Dali::CanvasRenderer::RadialGradient& radialGradient)
{
  DALI_ASSERT_ALWAYS(radialGradient && "RadialGradient handle is empty.");

  BaseObject& handle = radialGradient.GetBaseObject();

  return static_cast<Internal::Adaptor::RadialGradient&>(handle);
}

inline static const Internal::Adaptor::RadialGradient& GetImplementation(const Dali::CanvasRenderer::RadialGradient& radialGradient)
{
  DALI_ASSERT_ALWAYS(radialGradient && "RadialGradient handle is empty.");

  const BaseObject& handle = radialGradient.GetBaseObject();

  return static_cast<const Internal::Adaptor::RadialGradient&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_RADIAL_GRADIENT_IMPL_H
