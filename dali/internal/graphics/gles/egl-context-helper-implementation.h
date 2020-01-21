#ifndef DALI_INTERNAL_ADAPTOR_EGL_CONTEXT_HELPER_IMPLEMENTATION_H
#define DALI_INTERNAL_ADAPTOR_EGL_CONTEXT_HELPER_IMPLEMENTATION_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include <dali/internal/graphics/common/egl-include.h>
// EXTERNAL INCLUDES
#include <dali/integration-api/gl-context-helper-abstraction.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace Integration
{
class RenderSurface;
}

namespace Internal
{
namespace Adaptor
{

class EglImplementation;

/**
 * EglContextHelperImplementation is a concrete implementation for GlContextHelperAbstraction.
 * It provides helper functions to access EGL context APIs
 */
class EglContextHelperImplementation : public Integration::GlContextHelperAbstraction
{
public:
  /**
   * Constructor
   */
  EglContextHelperImplementation();

  /**
   * Destructor
   */
  virtual ~EglContextHelperImplementation() = default;

  /**
   * Initialize with the Egl implementation.
   * @param[in] impl The EGL implementation (to access the EGL context)
   */
  void Initialize( EglImplementation* impl );

  /**
   * @copydoc Dali::Integration::GlContextHelperAbstraction::MakeSurfacelessContextCurrent()
   */
  virtual void MakeSurfacelessContextCurrent() override;

  /**
   * @copydoc Dali::Integration::GlContextHelperAbstraction::MakeContextCurrent()
   */
  virtual void MakeContextCurrent( Integration::RenderSurface* surface ) override;

  /**
   * @copydoc Dali::Integration::GlContextHelperAbstraction::MakeContextNull()
   */
  virtual void MakeContextNull() override;

  /**
   * @copydoc Dali::Integration::GlContextHelperAbstraction::WaitClient()
   */
  virtual void WaitClient() override;

private:

  EglImplementation* mEglImplementation; ///< Egl implementation (to access the EGL context)
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_EGL_CONTEXT_HELPER_IMPLEMENTATION_H
