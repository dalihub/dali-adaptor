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
#include <dali/internal/graphics/vulkan-impl/vulkan-shader-impl.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-spirv.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

namespace Dali::Graphics::Vulkan
{
struct ShaderImpl::Impl
{
  explicit Impl(Graphics::Vulkan::VulkanGraphicsController& _controller, const Graphics::ShaderCreateInfo& _createInfo)
  : controller(_controller)
  {
    createInfo.pipelineStage  = _createInfo.pipelineStage;
    createInfo.shaderlanguage = _createInfo.shaderlanguage;
    createInfo.sourceMode     = _createInfo.sourceMode;
    createInfo.shaderVersion  = _createInfo.shaderVersion;

    if(createInfo.sourceMode == ShaderSourceMode::TEXT)
    {
      if(createInfo.shaderVersion > 0)
      {
        // Make a copy of source code. if code is meant to be used
        // by modern parser, skip the prefix part
        size_t dataStartIndex = 0;
        size_t dataSize;

        ShaderImpl::StripLegacyCodeIfNeeded(_createInfo, dataStartIndex, glslVersion, dataSize);

        source.resize(dataSize);
        std::copy(reinterpret_cast<const uint8_t*>(_createInfo.sourceData) + dataStartIndex,
                  reinterpret_cast<const uint8_t*>(_createInfo.sourceData) + dataStartIndex + dataSize,
                  source.data());

        // Substitute pointer
        createInfo.sourceData = source.data();
        createInfo.sourceSize = dataSize;
      }
    }
    else // if binary format, we expect ready to use SPIRV shader module
    {
      // Make a copy of spirv data
      source.resize(_createInfo.sourceSize);
      std::copy(reinterpret_cast<const uint8_t*>(_createInfo.sourceData),
                reinterpret_cast<const uint8_t*>(_createInfo.sourceData) + _createInfo.sourceSize,
                source.data());
      createInfo.sourceData = source.data();
      createInfo.sourceSize = _createInfo.sourceSize;
    }
  }

  ~Impl() = default;

  bool Compile()
  {
    bool success = true;
    if(createInfo.sourceMode == ShaderSourceMode::TEXT)
    {
      const auto src = !sourcePreprocessed.empty() ? reinterpret_cast<const char*>(sourcePreprocessed.data()) : reinterpret_cast<const char*>(createInfo.sourceData);

      // null-terminated char already included. So we should remove last character (null terminator) from size.
      int32_t size = static_cast<int32_t>(!sourcePreprocessed.empty() ? static_cast<uint32_t>(sourcePreprocessed.size()) : createInfo.sourceSize) - 1;

      if(src != nullptr && size >= 0)
      {
        SPIRVGeneratorInfo info;
        info.pipelineStage = createInfo.pipelineStage;
        auto shaderCode    = std::string_view(src, size);
        info.shaderCode    = shaderCode;

        spirv = std::make_unique<SPIRVGenerator>(info);

        spirv->Generate();
        if(spirv->IsValid())
        {
          // substitute data and size with compiled code
          createInfo.sourceSize = spirv->Get().size() * 4u;
          createInfo.sourceData = spirv->Get().data();
        }
        else
        {
          success = false;
        }
      }
      else
      {
        success = false;
      }
    }
    if(!success)
    {
      return false;
    }

    // Create Vulkan shader module
    auto&                      gfxDevice = controller.GetGraphicsDevice();
    auto                       vkDevice  = gfxDevice.GetLogicalDevice();
    vk::ShaderModuleCreateInfo info;
    info.pCode    = reinterpret_cast<const uint32_t*>(createInfo.sourceData);
    info.codeSize = size_t(createInfo.sourceSize);
    VkAssert(vkDevice.createShaderModule(&info, &gfxDevice.GetAllocator(), &shaderModule));

    return true;
  }

  void Destroy()
  {
    auto vkDevice = controller.GetGraphicsDevice().GetLogicalDevice();
    vkDevice.destroyShaderModule(shaderModule, controller.GetGraphicsDevice().GetAllocator());
  }

  void SetPreprocessedCode(void* data, uint32_t size)
  {
    if(size == 0)
    {
      sourcePreprocessed.clear();
      return;
    }

    const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(data);

    if(*(dataPtr + size - 1) != '\0')
    {
      sourcePreprocessed.resize(size + 1 /* Include null-terminated char */);
      sourcePreprocessed[size] = '\0';
    }
    else
    {
      // null-terminated char already included.
      sourcePreprocessed.resize(size);
    }

    std::copy(dataPtr, dataPtr + size, sourcePreprocessed.data());
  }

  VulkanGraphicsController&       controller;
  ShaderCreateInfo                createInfo;
  std::vector<uint8_t>            source{};
  std::vector<uint8_t>            sourcePreprocessed{};
  std::unique_ptr<SPIRVGenerator> spirv;
  vk::ShaderModule                shaderModule;

  uint32_t flushCount{0u};  ///< Number of frames at refCount=0
  uint32_t glslVersion{0u}; ///< 0 - unknown, otherwise valid #version like 130, 300, etc.
};

ShaderImpl::ShaderImpl(const Graphics::ShaderCreateInfo& createInfo, Graphics::Vulkan::VulkanGraphicsController& controller)
{
  mImpl = std::make_unique<Impl>(controller, createInfo);
}

ShaderImpl::~ShaderImpl()
{
  DestroyShaderModule();
}

void ShaderImpl::DestroyShaderModule()
{
  mImpl->Destroy();
  mImpl.reset();
}

[[nodiscard]] uint32_t ShaderImpl::IncreaseFlushCount()
{
  return ++mImpl->flushCount;
}

[[nodiscard]] uint32_t ShaderImpl::GetFlushCount() const
{
  return mImpl->flushCount;
}

[[nodiscard]] uint32_t ShaderImpl::GetGLSLVersion() const
{
  return mImpl->glslVersion;
}

/**
 * @brief Compiles shader
 *
 * @return True on success
 */
[[nodiscard]] bool ShaderImpl::Compile() const
{
  return mImpl->Compile();
}

[[nodiscard]] vk::ShaderModule ShaderImpl::GetVkShaderModule() const
{
  return mImpl->shaderModule;
}

const ShaderCreateInfo& ShaderImpl::GetCreateInfo() const
{
  return mImpl->createInfo;
}

[[nodiscard]] VulkanGraphicsController& ShaderImpl::GetController() const
{
  return mImpl->controller;
}

void ShaderImpl::StripLegacyCodeIfNeeded(const ShaderCreateInfo& info, size_t& startIndex, uint32_t& glslVersion, size_t& finalDataSize)
{
  // Make a copy of source code. if code is meant to be used
  // by modern parser, skip the prefix part
  auto text   = reinterpret_cast<const char*>(info.sourceData);
  auto result = std::string_view(text).find("//@legacy-prefix-end");
  glslVersion = 0u;
  if(info.shaderVersion != 0)
  {
    if(result != 0 && result != std::string::npos)
    {
      DALI_LOG_ERROR("Shader processing: @legacy-prefix-end must be a very first statement!\n");
    }
    else if(result == 0)
    {
      char* end;
      startIndex = std::strtoul(reinterpret_cast<const char*>(info.sourceData) + 21, &end, 10);
    }
  }
  else
  {
    // For legacy shaders we need to make sure that the #version is a very first line
    // so need to strip //@legacy-prefix-end tag
    auto versionPos = std::string_view(text).find("#version", 0);
    if(versionPos == std::string::npos)
    {
      startIndex = 0; // not trimming anything

      // if there's no version yet it's a legacy shader we assign 100
      glslVersion = 100;
    }
    else
    {
      // save version of legacy shader
      char* end;
      glslVersion = uint32_t(std::strtol(std::string_view(text).data() + versionPos + 9, &end, 10));
      startIndex  = versionPos;
    }
  }
  finalDataSize = info.sourceSize - startIndex;
}

void ShaderImpl::SetPreprocessedCode(void* data, uint32_t size)
{
  mImpl->SetPreprocessedCode(data, size);
}

} // namespace Dali::Graphics::Vulkan
