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
#include <dali/internal/graphics/vulkan-impl/vulkan-spirv.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// EXTERNAL EXCLUDES
#include <utility>

namespace Dali::Graphics::Vulkan
{
struct SPIRVGeneratorExtraInfo
{
  glslang_stage_t stage;
  bool            valid;
};

SPIRVGenerator::SPIRVGenerator(Dali::Graphics::Vulkan::SPIRVGeneratorInfo generatorInfo)
{
  mGeneratorInfo = std::move(generatorInfo);

  // Using new to inject internal data visible only within implementation
  mGeneratorInfo.extraInfo = new SPIRVGeneratorExtraInfo;

  mGeneratorInfo.extraInfo->valid = false;

  switch(mGeneratorInfo.pipelineStage)
  {
    case PipelineStage::VERTEX_SHADER:
    {
      mGeneratorInfo.extraInfo->stage = GLSLANG_STAGE_VERTEX;
      break;
    }
    case PipelineStage::FRAGMENT_SHADER:
    {
      mGeneratorInfo.extraInfo->stage = GLSLANG_STAGE_FRAGMENT;
      break;
    }
    default:
    {
      DALI_LOG_ERROR("SPIRVGenerator: Unsupported stage used!\n");
      mGeneratorInfo.extraInfo->valid = false;
      return;
    }
  }
}

SPIRVGenerator::~SPIRVGenerator()
{
  delete mGeneratorInfo.extraInfo;
}

glslang_shader_t* SPIRVGenerator::CompileSource(glslang_stage_t stage, std::string shaderSource)
{
  const char*           cstr  = shaderSource.c_str();
  const glslang_input_t input = {
    .language                          = GLSLANG_SOURCE_GLSL,
    .stage                             = stage,
    .client                            = GLSLANG_CLIENT_VULKAN,
    .client_version                    = GLSLANG_TARGET_VULKAN_1_0,
    .target_language                   = GLSLANG_TARGET_SPV,
    .target_language_version           = GLSLANG_TARGET_SPV_1_0,
    .code                              = cstr,
    .default_version                   = 100,
    .default_profile                   = GLSLANG_NO_PROFILE,
    .force_default_version_and_profile = false,
    .forward_compatible                = false,
    .messages                          = GLSLANG_MSG_DEFAULT_BIT,
    .resource                          = glslang_default_resource(),
  };

  glslang_shader_t* shader = glslang_shader_create(&input);

  if(!glslang_shader_preprocess(shader, &input))
  {
    DALI_LOG_ERROR("%s\n", glslang_shader_get_info_log(shader));
    DALI_LOG_ERROR("%s\n", glslang_shader_get_info_debug_log(shader));
    DALI_LOG_ERROR("%s\n", input.code);
    glslang_shader_delete(shader);
    return nullptr;
  }

  if(!glslang_shader_parse(shader, &input))
  {
    DALI_LOG_ERROR("%s\n", glslang_shader_get_info_log(shader));
    DALI_LOG_ERROR("%s\n", glslang_shader_get_info_debug_log(shader));
    DALI_LOG_ERROR("%s\n", glslang_shader_get_preprocessed_code(shader));
    glslang_shader_delete(shader);
    return nullptr;
  }

  return shader;
}

void SPIRVGenerator::Generate()
{
  glslang_initialize_process();
  auto  stage  = mGeneratorInfo.extraInfo->stage;
  auto* shader = CompileSource(stage, mGeneratorInfo.shaderCode);

  if(!shader)
  {
    return;
  }

  glslang_program_t* program = glslang_program_create();
  glslang_program_add_shader(program, shader);

  if(!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
  {
    DALI_LOG_ERROR("%s\n", glslang_program_get_info_log(program));
    DALI_LOG_ERROR("%s\n", glslang_program_get_info_debug_log(program));
    glslang_program_delete(program);
    return;
  }

  glslang_program_SPIRV_generate(program, stage);

  auto size = glslang_program_SPIRV_get_size(program);
  mBinary.resize(size);
  glslang_program_SPIRV_get(program, mBinary.data());

  mGeneratorInfo.extraInfo->valid = true;

  const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
  if(spirv_messages)
  {
    DALI_LOG_ERROR("%s\b", spirv_messages);
    mGeneratorInfo.extraInfo->valid = false;
  }

  glslang_program_delete(program);
  glslang_finalize_process();
}

bool SPIRVGenerator::IsValid() const
{
  return mGeneratorInfo.extraInfo->valid;
}

const std::vector<uint32_t>& SPIRVGenerator::Get()
{
  if(!mGeneratorInfo.extraInfo->valid)
  {
    mBinary.clear(); // return 0-size array
  }

  return mBinary;
}

} // namespace Dali::Graphics::Vulkan