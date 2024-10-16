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
 */

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-program.h>

// INTERNAL HEADERS
#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>

// EXTERNAL HEADERS
#include <iostream>

#if defined(DEBUG_ENABLED)
Debug::Filter* gGraphicsProgramLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_GRAPHICS_PROGRAM");
#endif

namespace Dali::Graphics::Vulkan
{
Program::Program(const Graphics::ProgramCreateInfo& createInfo, VulkanGraphicsController& controller)
{
  mProgram = new ProgramImpl(createInfo, controller);
}

Program::~Program()
{
  // Destroy Vulkan resources of implementation. This should happen
  // only if there's no more pipelines using this program so
  // it is safe to do it in the destructor
  // TODO: implement it via cache!
  if(!mProgram->Release())
  {
    mProgram->Destroy();
    delete mProgram;
  }
}

const Vulkan::Reflection& Program::GetReflection() const
{
  return mProgram->GetReflection();
}

VulkanGraphicsController& Program::GetController() const
{
  return GetImplementation()->GetController();
}

const ProgramCreateInfo& Program::GetCreateInfo() const
{
  return GetImplementation()->GetCreateInfo();
}

bool Program::InitializeResource()
{
  return true;
}

void Program::DiscardResource()
{
  GetController().DiscardResource(this);
}

void Program::DestroyResource()
{
  // nothing to do here
}

}; // namespace Dali::Graphics::Vulkan
