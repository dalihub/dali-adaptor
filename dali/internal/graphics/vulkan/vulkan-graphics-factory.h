#ifndef DALI_GRAPHICS_VULKAN_GRAPHICS_FACTORY_H
#define DALI_GRAPHICS_VULKAN_GRAPHICS_FACTORY_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

#include <dali/internal/graphics/common/graphics-factory-interface.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali::Internal::Adaptor
{
class DALI_ADAPTOR_API GraphicsFactory : public GraphicsFactoryInterface
{
public:
  /**
   * Constructor
   */
  explicit GraphicsFactory(EnvironmentOptions& environmentOptions);

  /**
   * Destructor
   */
  ~GraphicsFactory() override;

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

#endif // DALI_GRAPHICS_VULKAN_GRAPHICS_FACTORY_H
