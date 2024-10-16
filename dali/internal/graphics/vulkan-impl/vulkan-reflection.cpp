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
#include <dali/internal/graphics/vulkan-impl/vulkan-reflection.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-shader-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-shader.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/public-api/common/vector-wrapper.h>

// EXTERNAL INCLUDES
#include <third-party/SPIRV-Reflect/spirv_reflect.h>
#include <vulkan/vulkan.hpp>

namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gGraphicsReflectionLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_GRAPHICS_REFLECTION");
#endif

} // namespace
namespace Dali::Graphics::Vulkan
{
inline VertexInputAttributeFormat ToVertexInputAttributeFormat(SpvReflectFormat spvFormat)
{
  // only supported types (we need to verify)
  switch(spvFormat)
  {
    case SPV_REFLECT_FORMAT_R32_SINT:
    {
      return VertexInputAttributeFormat::INTEGER;
    }
    case SPV_REFLECT_FORMAT_R32_SFLOAT:
    {
      return VertexInputAttributeFormat::FLOAT;
    }
    //case SPV_REFLECT_FORMAT_R32G32_SINT:{
    //  return VertexInputAttributeFormat::VEC2
    //}
    case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
    {
      return VertexInputAttributeFormat::VEC2;
    }
    //case SPV_REFLECT_FORMAT_R32G32B32_SINT:{}
    case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
    {
      return VertexInputAttributeFormat::VEC3;
    }
    //case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:{};
    case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
    {
      return VertexInputAttributeFormat::VEC4;
    }
    default:
    {
      return VertexInputAttributeFormat::UNDEFINED;
    }
  }
}

Reflection::Reflection(Vulkan::ProgramImpl& program, VulkanGraphicsController& controller)
: Graphics::Reflection(),
  mController(controller),
  mProgram(program)
{
  BuildReflection();
}

Reflection::~Reflection()
{
  auto  vkDevice  = mController.GetGraphicsDevice().GetLogicalDevice();
  auto& allocator = mController.GetGraphicsDevice().GetAllocator();

  vkDevice.destroyPipelineLayout(mVkPipelineLayout, allocator);
  for(auto& dsLayout : mVkDescriptorSetLayoutList)
  {
    vkDevice.destroyDescriptorSetLayout(dsLayout, allocator);
  }
}

template<typename FN, typename OUT>
void SPIRVEnumerate(FN& proc, SpvReflectShaderModule* module, std::vector<OUT*>& out)
{
  uint32_t var_count = 0;
  proc(module, &var_count, nullptr);
  out.resize(var_count);
  auto ptr = out.data();
  proc(module, &var_count, reinterpret_cast<OUT**>(ptr));
}

void Reflection::BuildReflection()
{
  // LIMITATION: only one shader module per stage is allowed. Vulkan supports linking multiple
  // modules per stage but at this moment we won't support it (because we don't have a use case)
  bool vertexShaderDone   = false;
  bool fragmentShaderDone = false;
  enum class StageCheckResult
  {
    INVALID      = -1,
    ALREADY_DONE = 1,
    OK           = 0
  };

  // initialize uniform block data
  mUniformBlocks.clear();
  mUniformBlocks.resize(1); // index 0 is standalone block which isn't in use by Vulkan but must be there

  // initialize list of samplers
  // NOTE: We support only COMBINED_IMAGE_SAMPLER type currently (regular sampler on the GLES side)
  std::vector<UniformInfo> samplers;

  // build descriptor set layout (currently, we support only one set!)
  std::vector<vk::DescriptorSetLayoutCreateInfo> dsLayoutInfos;

  for(auto& state : (*mProgram.GetCreateInfo().shaderState))
  {
    auto                   impl        = static_cast<const Vulkan::Shader*>(state.shader)->GetImplementation();
    auto                   spirvBinary = impl->GetCreateInfo().sourceData;
    auto                   spirvLength = impl->GetCreateInfo().sourceSize;
    auto                   stage       = state.pipelineStage;
    SpvReflectShaderModule module;
    SpvReflectResult       result = spvReflectCreateShaderModule(spirvLength, spirvBinary, &module);

    if(result != SPV_REFLECT_RESULT_SUCCESS)
    {
      DALI_LOG_ERROR("Can't reflect SPIRV module! err = %d\n", int(result));
      continue;
    }

    // helper lambda if we need to check more types of pipeline stages in the future
    auto CheckStageIfDone = [stage](auto expectedStage, auto& variable, const char* stageName) -> StageCheckResult {
      if(stage == expectedStage)
      {
        if(!variable)
        {
          variable = true;
        }
        else
        {
          DALI_LOG_ERROR("Can't reflect SPIRV module! Only one module per %s is allowed!\n", stageName);
          return StageCheckResult::ALREADY_DONE; // return true if stage already done
        }
      }
      else
      {
        return StageCheckResult::INVALID; // not stage
      }
      return StageCheckResult::OK;
    };

    // Process only one module per stage
    if(CheckStageIfDone(PipelineStage::VERTEX_SHADER, vertexShaderDone, "VERTEX_STAGE") == StageCheckResult::ALREADY_DONE)
    {
      continue;
    }
    if(CheckStageIfDone(PipelineStage::FRAGMENT_SHADER, fragmentShaderDone, "FRAGMENT_STAGE") == StageCheckResult::ALREADY_DONE)
    {
      continue;
    }

    // Process vertex shader attributes
    if(stage == PipelineStage::VERTEX_SHADER)
    {
      BuildVertexAttributeReflection(&module);

      // build vertex input layouts for Vulkan
    }

    // Prepare descriptor set layout allocation
    std::vector<SpvReflectDescriptorSet*> dsSets;
    SPIRVEnumerate(spvReflectEnumerateDescriptorSets, &module, dsSets);

    // Per each descriptor set
    for(auto& dsSet : dsSets)
    {
      if(dsSet->set >= mVkDescriptorSetLayoutCreateInfoList.size())
      {
        mVkDescriptorSetLayoutCreateInfoList.resize(dsSet->set + 1);
        mVkDescriptorSetLayoutBindingList.resize(dsSet->set + 1);
      }

      // per each set preallocate bindings
      std::vector<vk::DescriptorSetLayoutBinding> bindings;
      bindings.resize(dsSet->binding_count);
      for(auto i = 0u; i < dsSet->binding_count; ++i)
      {
        auto& bind = bindings[i];
        auto& ref  = *dsSet->bindings[i];
        bind.setBinding(ref.binding);
        bind.setDescriptorCount(ref.count);
        bind.setDescriptorType(vk::DescriptorType(ref.descriptor_type));
        bind.setStageFlags(
          stage == PipelineStage::VERTEX_SHADER ? vk::ShaderStageFlagBits::eVertex : vk::ShaderStageFlagBits::eFragment);
      }

      auto& dsSetCreateInfo = mVkDescriptorSetLayoutCreateInfoList[dsSet->set];

      auto& bindingList = mVkDescriptorSetLayoutBindingList[dsSet->set];
      bindingList.insert(bindingList.end(), bindings.begin(), bindings.end());
      dsSetCreateInfo.setBindings(bindingList);
      dsSetCreateInfo.setBindingCount(bindingList.size());
    }

    // process uniform buffer bindings and samplers (opaque uniforms)
    std::vector<SpvReflectDescriptorBinding*> dsBindings;
    SPIRVEnumerate(spvReflectEnumerateDescriptorBindings, &module, dsBindings);

    uint32_t blockIndex = 1u;
    for([[maybe_unused]] auto& binding : dsBindings)
    {
      // process uniform buffer
      if(binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      {
        auto memberCount = binding->block.member_count;
        auto members     = binding->block.members;
        mUniformBlocks.emplace_back();
        auto& block         = mUniformBlocks.back();
        block.binding       = binding->binding;
        block.name          = binding->name;
        block.descriptorSet = binding->set;
        block.size          = 0; // to be updated with members

        block.members.resize(memberCount);
        for(auto i = 0u; i < memberCount; ++i)
        {
          auto& out         = block.members[i];
          auto& memb        = members[i];
          out.name          = memb.name;
          out.location      = 0;
          out.offset        = memb.offset;
          out.elementStride = memb.array.dims_count ? memb.array.stride : 0;
          out.elementCount  = memb.array.dims[0]; // will be zero for non-array
          out.uniformClass  = UniformClass::UNIFORM_BUFFER;
          out.bufferIndex   = blockIndex++; // TODO: do we need this for Vulkan?
          block.size += memb.padded_size;
        }
      }
      else if(binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      {
        samplers.emplace_back();
        auto uniformInfo          = &samplers.back();
        uniformInfo->uniformClass = UniformClass::COMBINED_IMAGE_SAMPLER;
        uniformInfo->name         = binding->name;
        uniformInfo->offset       = 0;
        uniformInfo->location     = 0; // location is in the order of bidings
        uniformInfo->binding      = binding->binding;
      }
    }

    if(!samplers.empty())
    {
      mUniformOpaques.insert(mUniformOpaques.end(), samplers.begin(), samplers.end());
      // sort samplers by bindings
      std::sort(mUniformOpaques.begin(), mUniformOpaques.end(), [](auto& lhs, auto& rhs) { return lhs.binding < rhs.binding; });
      for(auto i = 0u; i < mUniformOpaques.size(); ++i)
      {
        mUniformOpaques[i].location = i;
      }
    }

    spvReflectDestroyShaderModule(&module);
  }

  auto vkDevice = mController.GetGraphicsDevice().GetLogicalDevice();

  // Create descriptor set layouts

  for(auto& dsLayoutCreateInfo : mVkDescriptorSetLayoutCreateInfoList)
  {
    auto dsLayout = vkDevice.createDescriptorSetLayout(dsLayoutCreateInfo,
                                                       mController.GetGraphicsDevice().GetAllocator());

    mVkDescriptorSetLayoutList.emplace_back(dsLayout.value);
  }

  // Create pipeline layout
  vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  pipelineLayoutCreateInfo.setSetLayouts(mVkDescriptorSetLayoutList);
  // TODO: support push-constants, for now pipeline layout ignores push-constant

  vk::PipelineLayout pipelineLayout;
  VkAssert(vkDevice.createPipelineLayout(&pipelineLayoutCreateInfo, &mController.GetGraphicsDevice().GetAllocator(), &pipelineLayout));
  mVkPipelineLayout = pipelineLayout;

  // Destroy descriptor set layouts
}

vk::PipelineLayout Reflection::GetVkPipelineLayout() const
{
  return mVkPipelineLayout;
}

void Reflection::BuildVertexAttributeReflection(SpvReflectShaderModule* spvModule)
{
  std::vector<SpvReflectInterfaceVariable*> attrs;
  SPIRVEnumerate(spvReflectEnumerateInputVariables, &*spvModule, attrs);
  mVertexInputAttributes.clear();
  mVertexInputAttributes.resize(attrs.size());
  int maximumLocation = int(attrs.size()) - 1;
  for(auto& attr : attrs)
  {
    // SPIRV contains builtin attributes that are added with locations
    // at the end of 32bit range. To skip it we assume some 'healthy' top range
    // for locations that we are unlikely to exceed. max of unsigned 16bit number (65535)
    // should be more than enough.
    // TODO: consider an usecase we may want to use built-in inputs?
    if(attr->location > std::numeric_limits<uint16_t>::max())
    {
      continue;
    }

    AttributeInfo attributeInfo;
    attributeInfo.location = attr->location;
    attributeInfo.name     = attr->name;
    attributeInfo.format   = ToVertexInputAttributeFormat(attr->format);

    if(attributeInfo.location >= 0)
    {
      if(maximumLocation < int(attributeInfo.location))
      {
        maximumLocation = int(attributeInfo.location);
        // Increase container size s.t. we can use maximumLocation as index.
        mVertexInputAttributes.resize(maximumLocation + 1u);
      }

      mVertexInputAttributes[attributeInfo.location] = std::move(attributeInfo);
    }
  }
}

uint32_t Reflection::GetVertexAttributeLocation(const std::string& name) const
{
  DALI_LOG_INFO(gGraphicsReflectionLogFilter, Debug::Verbose, "name : %s\n", name.c_str());
  for(auto&& attr : mVertexInputAttributes)
  {
    if(attr.name == name)
    {
      return attr.location;
    }
  }
  return ERROR_ATTRIBUTE_NOT_FOUND;
}

Dali::Graphics::VertexInputAttributeFormat Reflection::GetVertexAttributeFormat(uint32_t location) const
{
  DALI_LOG_INFO(gGraphicsReflectionLogFilter, Debug::Verbose, "location : %u\n", location);
  if(location >= mVertexInputAttributes.size())
  {
    return Dali::Graphics::VertexInputAttributeFormat::UNDEFINED;
  }

  return mVertexInputAttributes[location].format;
}

std::string Reflection::GetVertexAttributeName(uint32_t location) const
{
  DALI_LOG_INFO(gGraphicsReflectionLogFilter, Debug::Verbose, "location : %u\n", location);
  if(location >= mVertexInputAttributes.size())
  {
    return {};
  }

  return mVertexInputAttributes[location].name;
}

std::vector<uint32_t> Reflection::GetVertexAttributeLocations() const
{
  std::vector<uint32_t> locations;
  for(auto&& attr : mVertexInputAttributes)
  {
    if(attr.format != Dali::Graphics::VertexInputAttributeFormat::UNDEFINED)
    {
      locations.push_back(attr.location);
    }
  }

  return locations;
}

uint32_t Reflection::GetUniformBlockCount() const
{
  return mUniformBlocks.size();
}

uint32_t Reflection::GetUniformBlockBinding(uint32_t index) const
{
  return index < mUniformBlocks.size() ? mUniformBlocks[index].binding : 0u;
}

uint32_t Reflection::GetUniformBlockSize(uint32_t index) const
{
  return index < mUniformBlocks.size() ? mUniformBlocks[index].size : 0u;
}

bool Reflection::GetUniformBlock(uint32_t index, Dali::Graphics::UniformBlockInfo& out) const
{
  if(index >= mUniformBlocks.size())
  {
    return false;
  }

  const auto& block = mUniformBlocks[index];

  out.name          = block.name;
  out.binding       = block.binding;
  out.descriptorSet = block.descriptorSet;
  auto membersSize  = block.members.size();
  out.members.resize(membersSize);
  out.size = block.size;
  for(auto i = 0u; i < out.members.size(); ++i)
  {
    const auto& memberUniform    = block.members[i];
    out.members[i].name          = memberUniform.name;
    out.members[i].binding       = block.binding;
    out.members[i].uniformClass  = Graphics::UniformClass::UNIFORM;
    out.members[i].offset        = memberUniform.offset;
    out.members[i].location      = memberUniform.location;
    out.members[i].elementCount  = memberUniform.elementCount;
    out.members[i].elementStride = memberUniform.elementStride;
  }

  return true;
}

std::vector<uint32_t> Reflection::GetUniformBlockLocations() const
{
  std::vector<uint32_t> retval{};
  for(auto&& ubo : mUniformBlocks)
  {
    retval.emplace_back(ubo.binding);
  }
  return retval;
}

std::string Reflection::GetUniformBlockName(uint32_t blockIndex) const
{
  if(blockIndex < mUniformBlocks.size())
  {
    return mUniformBlocks[blockIndex].name;
  }
  else
  {
    return {};
  }
}

uint32_t Reflection::GetUniformBlockMemberCount(uint32_t blockIndex) const
{
  if(blockIndex < mUniformBlocks.size())
  {
    return static_cast<uint32_t>(mUniformBlocks[blockIndex].members.size());
  }
  else
  {
    return 0u;
  }
}

std::string Reflection::GetUniformBlockMemberName(uint32_t blockIndex, uint32_t memberLocation) const
{
  if(blockIndex < mUniformBlocks.size() && memberLocation < mUniformBlocks[blockIndex].members.size())
  {
    return mUniformBlocks[blockIndex].members[memberLocation].name;
  }
  else
  {
    return {};
  }
}

uint32_t Reflection::GetUniformBlockMemberOffset(uint32_t blockIndex, uint32_t memberLocation) const
{
  if(blockIndex < mUniformBlocks.size() && memberLocation < mUniformBlocks[blockIndex].members.size())
  {
    return mUniformBlocks[blockIndex].members[memberLocation].offset;
  }
  else
  {
    return 0u;
  }
}

bool Reflection::GetNamedUniform(const std::string& name, Dali::Graphics::UniformInfo& out) const
{
  auto index = 0u;
  for(auto&& ubo : mUniformBlocks)
  {
    for(auto&& member : ubo.members)
    {
      if(name == member.name || name == (ubo.name + "." + member.name))
      {
        out.name         = name;
        out.location     = member.location;
        out.binding      = ubo.binding;
        out.bufferIndex  = index;
        out.offset       = member.offset;
        out.uniformClass = Graphics::UniformClass::UNIFORM;
        return true;
      }
    }
    ++index;
  }

  // check samplers
  index = 0u;
  for(auto&& uniform : mUniformOpaques)
  {
    if(uniform.name == name)
    {
      out.uniformClass = Graphics::UniformClass::COMBINED_IMAGE_SAMPLER;
      out.binding      = 0;
      out.name         = name;
      out.offset       = index;            // lexical location in shader
      out.location     = uniform.location; // uniform location mapping
      return true;
    }
    ++index;
  }

  return false;
}

const std::vector<Dali::Graphics::UniformInfo>& Reflection::GetSamplers() const
{
  return mUniformOpaques;
}

Graphics::ShaderLanguage Reflection::GetLanguage() const
{
  return ShaderLanguage::SPIRV_1_0;
}

} // namespace Dali::Graphics::Vulkan
