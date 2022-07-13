#ifndef DALI_INTERNAL_LINEAR_GRADIENT_IMPL_H
#define DALI_INTERNAL_LINEAR_GRADIENT_IMPL_H

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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-linear-gradient.h>
#include <dali/internal/canvas-renderer/common/gradient-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal LinearGradient.
 */
class LinearGradient : public Internal::Adaptor::Gradient
{
public:
  /**
   * @brief Constructor
   */
  LinearGradient();

  /**
   * @brief Destructor.
   */
  ~LinearGradient() override;

  /**
   * @copydoc Dali::CanvasRenderer::LinearGradient::SetBounds()
   */
  virtual bool SetBounds(Vector2 firstPoint, Vector2 SecondPoint);

  /**
   * @copydoc Dali::CanvasRenderer::LinearGradient::SetBounds()
   */
  virtual bool GetBounds(Vector2& firstPoint, Vector2& SecondPoint) const;

  LinearGradient(const LinearGradient&) = delete;
  LinearGradient& operator=(LinearGradient&) = delete;
  LinearGradient(LinearGradient&&)           = delete;
  LinearGradient& operator=(LinearGradient&&) = delete;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::LinearGradient& GetImplementation(Dali::CanvasRenderer::LinearGradient& linearGradient)
{
  DALI_ASSERT_ALWAYS(linearGradient && "LinearGradient handle is empty.");

  BaseObject& handle = linearGradient.GetBaseObject();

  return static_cast<Internal::Adaptor::LinearGradient&>(handle);
}

inline static const Internal::Adaptor::LinearGradient& GetImplementation(const Dali::CanvasRenderer::LinearGradient& linearGradient)
{
  DALI_ASSERT_ALWAYS(linearGradient && "LinearGradient handle is empty.");

  const BaseObject& handle = linearGradient.GetBaseObject();

  return static_cast<const Internal::Adaptor::LinearGradient&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_LINEAR_GRADIENT_IMPL_H
