#ifndef DALI_INTERNAL_GRADIENT_IMPL_H
#define DALI_INTERNAL_GRADIENT_IMPL_H

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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-gradient.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal Gradient.
 */
class Gradient : public Dali::BaseObject
{
public:
  /**
   * @brief Constructor
   */
  Gradient();

  /**
   * @brief Destructor.
   */
  virtual ~Gradient() override;

  /**
   * @brief Create factory item(implementation) object.
   */
  void Create();

  /**
   * @copydoc Dali::CanvasRenderer::Gradient::SetColorStops()
   */
  virtual bool SetColorStops(Dali::CanvasRenderer::Gradient::ColorStops& colorStops);

  /**
   * @copydoc Dali::CanvasRenderer::Gradient::GetColorStops()
   */
  virtual Dali::CanvasRenderer::Gradient::ColorStops GetColorStops() const;

  /**
   * @copydoc Dali::CanvasRenderer::Gradient::SetSpread()
   */
  virtual bool SetSpread(Dali::CanvasRenderer::Gradient::Spread spread);

  /**
   * @copydoc Dali::CanvasRenderer::Gradient::GetSpread()
   */
  virtual Dali::CanvasRenderer::Gradient::Spread GetSpread() const;

  /**
   * @brief Set a gradient object
   * @param[in] object gradient object
   */
  virtual void SetObject(const void* object);

  /**
   * @brief Returns a gradient object pointer.
   * @return Returns a gradient object pointer.
   */
  virtual void* GetObject() const;

  /**
   * @brief Set a changed state.
   * @param[in] changed The state of changed.
   */
  virtual void SetChanged(bool changed);

  /**
   * @brief Get a changed state.
   * @return Returns state of changed.
   */
  virtual bool GetChanged() const;

  /**
   * @brief Returns a gradient's implements object pointer.
   * @return Returns a gradient's implements object pointer.
   */
  Dali::Internal::Adaptor::Gradient* GetImplementation();

  Gradient(const Gradient&) = delete;
  Gradient& operator=(Gradient&) = delete;
  Gradient(Gradient&&)           = delete;
  Gradient& operator=(Gradient&&) = delete;

private:
  Dali::Internal::Adaptor::Gradient* mImpl = nullptr;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::Gradient& GetImplementation(Dali::CanvasRenderer::Gradient& gradient)
{
  DALI_ASSERT_ALWAYS(gradient && "Gradient handle is empty.");

  BaseObject& handle = gradient.GetBaseObject();

  return static_cast<Internal::Adaptor::Gradient&>(handle);
}

inline static const Internal::Adaptor::Gradient& GetImplementation(const Dali::CanvasRenderer::Gradient& gradient)
{
  DALI_ASSERT_ALWAYS(gradient && "Gradient handle is empty.");

  const BaseObject& handle = gradient.GetBaseObject();

  return static_cast<const Internal::Adaptor::Gradient&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_GRADIENT_IMPL_H
