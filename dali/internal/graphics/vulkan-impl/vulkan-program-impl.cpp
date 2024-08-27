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
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>

// INTERNAL HEADERS
#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/common/shader-parser.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-reflection.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-shader-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-shader.h>

// EXTERNAL HEADERS
#include <iostream>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gGraphicsProgramLogFilter;
#endif

namespace Dali::Graphics::Vulkan
{
struct ProgramImpl::Impl
{
  explicit Impl(VulkanGraphicsController& _controller, const ProgramCreateInfo& info)
  : controller(_controller)
  {
    createInfo = info;
    if(info.shaderState)
    {
      createInfo.shaderState = new std::vector<ShaderState>(*info.shaderState);
    }

    // Create new reference of std::string_view.
    name            = std::string(info.name);
    createInfo.name = name;
  }

  ~Impl()
  {
    delete createInfo.shaderState;
  }

  VulkanGraphicsController& controller;
  ProgramCreateInfo         createInfo;
  std::string               name;
  uint32_t                  refCount{0u};

  std::unique_ptr<Vulkan::Reflection> reflection{nullptr};
};

ProgramImpl::ProgramImpl(const Graphics::ProgramCreateInfo& createInfo, VulkanGraphicsController& controller)
{
  // Create implementation
  mImpl = std::make_unique<Impl>(controller, createInfo);

  // Preprocess source code, if successful, compile shaders
  bool success = true;
  if(Preprocess())
  {
    for(const auto& state : *createInfo.shaderState)
    {
      auto shader     = static_cast<const Vulkan::Shader*>(state.shader);
      auto shaderImpl = shader->GetImplementation();
      if(!shaderImpl->Compile())
      {
        DALI_LOG_ERROR("SPIRV Compilation failed!\n");
        success = false;
      }
    }
  }

  if(success)
  {
    // Build reflection
    mImpl->reflection = std::make_unique<Vulkan::Reflection>(*this, controller);
  }
}

ProgramImpl::~ProgramImpl() = default;

bool ProgramImpl::Destroy()
{
  return false;
}

bool ProgramImpl::Preprocess()
{
  // For now only Vertex and Fragment shader stages supported
  // and one per stage
  std::string  vertexString;
  std::string  fragmentString;
  std::string* currentString = nullptr;

  const Vulkan::Shader* vsh = nullptr;
  const Vulkan::Shader* fsh = nullptr;

  const auto& info = mImpl->createInfo;

  for(const auto& state : *info.shaderState)
  {
    const auto* shader = static_cast<const Vulkan::Shader*>(state.shader);
    if(state.pipelineStage == PipelineStage::VERTEX_SHADER)
    {
      // Only TEXT source mode can be processed
      currentString = &vertexString;
      vsh           = shader;
    }
    else if(state.pipelineStage == PipelineStage::FRAGMENT_SHADER)
    {
      // Only TEXT source mode can be processed
      currentString = &fragmentString;
      fsh           = shader;
    }
    else
    {
      // no valid stream to push
      currentString = nullptr;
      DALI_LOG_ERROR("Shader state contains invalid shader source (most likely binary)! Can't process!");
    }

    // Check if stream valid
    if(currentString && currentString->empty() && shader->GetCreateInfo().sourceMode == ShaderSourceMode::TEXT)
    {
      *currentString = std::string(reinterpret_cast<const char*>(shader->GetCreateInfo().sourceData),
                                   shader->GetCreateInfo().sourceSize);
    }
    else
    {
      DALI_LOG_ERROR("Preprocessing of binary shaders isn't allowed!");
    }
  }

  // if we have both streams ready
  if(!vertexString.empty() && !fragmentString.empty())
  {
    // In case we have one modern shader and one legacy counterpart we need to enforce
    // output language.
    Internal::ShaderParser::ShaderParserInfo parseInfo{};
    parseInfo.vertexShaderCode            = &vertexString;
    parseInfo.fragmentShaderCode          = &fragmentString;
    parseInfo.vertexShaderLegacyVersion   = vsh->GetGLSLVersion();
    parseInfo.fragmentShaderLegacyVersion = fsh->GetGLSLVersion();
    parseInfo.language                    = Internal::ShaderParser::OutputLanguage::SPIRV_GLSL;
    parseInfo.outputVersion               = std::max(vsh->GetGLSLVersion(), fsh->GetGLSLVersion());

    std::vector<std::string> newShaders;

    Internal::ShaderParser::Parse(parseInfo, newShaders);

    // substitute shader code
    vsh->GetImplementation()->SetPreprocessedCode(newShaders[0].data(), newShaders[0].size());
    fsh->GetImplementation()->SetPreprocessedCode(newShaders[1].data(), newShaders[1].size());
    return true;
  }
  else
  {
    DALI_LOG_ERROR("Preprocessing shader code failed!");
    return false;
  }
}

bool ProgramImpl::Create()
{
  // TODO: redirect to the reflection builder
  return true;
}

uint32_t ProgramImpl::GetSPIRVProgram() const
{
  return {};
}

uint32_t ProgramImpl::Retain()
{
  return ++mImpl->refCount;
}

uint32_t ProgramImpl::Release()
{
  return --mImpl->refCount;
}

uint32_t ProgramImpl::GetRefCount() const
{
  return mImpl->refCount;
}

const Vulkan::Reflection& ProgramImpl::GetReflection() const
{
  return *mImpl->reflection;
}

bool ProgramImpl::GetParameter(uint32_t parameterId, void* out)
{
  return false;
}

VulkanGraphicsController& ProgramImpl::GetController() const
{
  return mImpl->controller;
}

const ProgramCreateInfo& ProgramImpl::GetCreateInfo() const
{
  return mImpl->createInfo;
}

}; // namespace Dali::Graphics::Vulkan