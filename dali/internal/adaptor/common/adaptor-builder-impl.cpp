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

// CLASS HEADER
#include <dali/internal/adaptor/common/adaptor-builder-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/display-utils.h>
#include <dali/public-api/adaptor-framework/graphics-backend.h>

#if defined(VULKAN_ENABLED)
#include <dali/internal/graphics/vulkan/vulkan-graphics-factory.h>
#else
#include <dali/internal/graphics/gles/egl-graphics-factory.h>
#endif

namespace Dali::Internal::Adaptor
{
namespace
{
static AdaptorBuilder* gAdaptorBuilder = nullptr;
}
AdaptorBuilder& AdaptorBuilder::Get(EnvironmentOptions& environmentOptions)
{
  if(gAdaptorBuilder == nullptr)
  {
    gAdaptorBuilder = new AdaptorBuilder(environmentOptions);
  }
  return *gAdaptorBuilder;
}

void AdaptorBuilder::Finalize()
{
  delete gAdaptorBuilder;
  gAdaptorBuilder = nullptr;
}

AdaptorBuilder::AdaptorBuilder(EnvironmentOptions& environmentOptions)
: mEnvironmentOptions(environmentOptions)
{
  switch(Graphics::GetCurrentGraphicsBackend())
  {
    case Graphics::Backend::GLES:
    {
      DALI_LOG_RELEASE_INFO("DALi Graphics Backend: GLES\n");
      // TODO: Load GLES library
      break;
    }

    case Graphics::Backend::VULKAN:
    {
      DALI_LOG_RELEASE_INFO("DALi Graphics Backend: VULKAN\n");
      // TODO: Attempt to load Vulkan library
      break;
    }
  }

  // Construct Graphics Factory
  mGraphicsFactory = Utils::MakeUnique<GraphicsFactory>(environmentOptions);
}

GraphicsFactoryInterface& AdaptorBuilder::GetGraphicsFactory() const
{
  return *mGraphicsFactory;
}

} // namespace Dali::Internal::Adaptor
