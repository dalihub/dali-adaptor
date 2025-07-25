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
#include "vulkan-shader.h"

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include "vulkan-graphics-controller.h"
#include "vulkan-shader-impl.h"

namespace Dali::Graphics::Vulkan
{
Shader::Shader(ShaderHandle impl)
: mShader(impl)
{
}

Shader::Shader(const Graphics::ShaderCreateInfo& createInfo, VulkanGraphicsController& controller)
{
  mShader = ShaderHandle(new ShaderImpl(createInfo, controller));
}

Shader::~Shader()
{
}

bool Shader::TryRecycle(const Graphics::ShaderCreateInfo& createInfo, VulkanGraphicsController& controller)
{
  return false;
}

[[nodiscard]] const ShaderCreateInfo& Shader::GetCreateInfo() const
{
  return GetImplementation()->GetCreateInfo();
}

void Shader::DiscardResource()
{
}

uint32_t Shader::GetGLSLVersion() const
{
  return GetImplementation()->GetGLSLVersion();
}

[[nodiscard]] ShaderHandle Shader::GetImplementation() const
{
  return mShader;
}

} // namespace Dali::Graphics::Vulkan
