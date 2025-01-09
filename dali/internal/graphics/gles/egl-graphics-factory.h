#ifndef DALI_INTERNAL_EGL_GRAPHICS_FACTORY_H
#define DALI_INTERNAL_EGL_GRAPHICS_FACTORY_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/graphics/common/graphics-factory-interface.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/environment-options.h>

namespace Dali::Internal::Adaptor
{
// Needs exporting as it's called directly by the GlWindow library
class DALI_ADAPTOR_API EglGraphicsFactory : public GraphicsFactoryInterface
{
public:
  /**
   * Constructor
   */
  explicit EglGraphicsFactory(EnvironmentOptions& environmentOptions);

  /**
   * Destructor
   */
  ~EglGraphicsFactory() override;

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsFactoryInterface::Create()
   */
  Graphics::GraphicsInterface& Create() override;

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsFactoryInterface::Destroy()
   */
  void Destroy() override;

private:
  EnvironmentOptions& mEnvironmentOptions;
};

} // namespace Dali::Internal::Adaptor

#endif // DALI_INTERNAL_EGL_GRAPHICS_FACTORY_H
