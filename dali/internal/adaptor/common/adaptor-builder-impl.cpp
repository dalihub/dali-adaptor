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
#include <dali/internal/adaptor/common/adaptor-builder-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-factory.h>

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
  // Construct Graphics Factory
  mGraphicsFactory = CreateGraphicsFactory(environmentOptions);
}

GraphicsFactoryInterface& AdaptorBuilder::GetGraphicsFactory() const
{
  return *mGraphicsFactory;
}

} // namespace Dali::Internal::Adaptor
