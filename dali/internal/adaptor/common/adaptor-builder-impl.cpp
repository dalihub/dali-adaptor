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

// CLASS HEADER
#include <dali/internal/adaptor/common/adaptor-builder-impl.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/internal/window-system/common/display-utils.h>

#if defined(VULKAN_ENABLED)
#include <dali/graphics/vulkan/vulkan-graphics-factory.h>
#else
#include <dali/internal/graphics/gles/egl-graphics-factory.h>
#endif

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

AdaptorBuilder* gAdaptorBuilder=nullptr;

AdaptorBuilder& AdaptorBuilder::Get(EnvironmentOptions& environmentOptions)
{
  if( gAdaptorBuilder == nullptr )
  {
    gAdaptorBuilder = new AdaptorBuilder( environmentOptions );
  }
  return *gAdaptorBuilder;
}

AdaptorBuilder::AdaptorBuilder(EnvironmentOptions& environmentOptions)
: mGraphicsFactory(nullptr),
  mEnvironmentOptions( environmentOptions )
{
  // Construct Graphics Factory
  mGraphicsFactory = Utils::MakeUnique< GraphicsFactory >(environmentOptions);
}

GraphicsFactory& AdaptorBuilder::GetGraphicsFactory() const
{
  return *mGraphicsFactory;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
