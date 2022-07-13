#ifndef DALI_INTERNAL_TIZEN_GRADIENT_IMPL_TIZEN_H
#define DALI_INTERNAL_TIZEN_GRADIENT_IMPL_TIZEN_H

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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-gradient.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>
#include <dali/internal/canvas-renderer/common/gradient-impl.h>
#include <dali/internal/canvas-renderer/tizen/canvas-renderer-impl-tizen.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal Gradient.
 */
class GradientTizen : public Dali::Internal::Adaptor::Gradient
{
public:
  /**
   * @brief Creates a Gradient object.
   * @return A pointer to a newly allocated Gradient
   */
  static GradientTizen* New();

  /**
   * @copydoc Dali::CanvasRenderer::Gradient::SetColorStops()
   */
  bool SetColorStops(Dali::CanvasRenderer::Gradient::ColorStops& colorStops) override;

  /**
   * @copydoc Dali::CanvasRenderer::Gradient::GetColorStops()
   */
  Dali::CanvasRenderer::Gradient::ColorStops GetColorStops() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Gradient::SetSpread()
   */
  bool SetSpread(Dali::CanvasRenderer::Gradient::Spread spread) override;

  /**
   * @copydoc Dali::CanvasRenderer::Gradient::GetSpread()
   */
  Dali::CanvasRenderer::Gradient::Spread GetSpread() const override;

  /**
   * @copydoc Internal::Adaptor::Gradient::GetObject()
   */
  void SetObject(const void* object) override;

  /**
   * @copydoc Internal::Adaptor::Gradient::SetObject()
   */
  void* GetObject() const override;

  /**
   * @copydoc Internal::Adaptor::Gradient::SetChanged()
   */
  void SetChanged(bool changed) override;

  /**
   * @copydoc Internal::Adaptor::Gradient::GetChanged()
   */
  bool GetChanged() const override;

  GradientTizen(const Gradient&) = delete;
  GradientTizen& operator=(Gradient&) = delete;
  GradientTizen(Gradient&&)           = delete;
  GradientTizen& operator=(Gradient&&) = delete;

protected:
  /**
   * @brief Constructor
   */
  GradientTizen();

  /**
   * @brief Destructor.
   */
  virtual ~GradientTizen() override;

private:
  bool mChanged;
#ifdef THORVG_SUPPORT
  tvg::Fill* mTvgFill;
#endif
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TIZEN_GRADIENT_IMPL_TIZEN_H
