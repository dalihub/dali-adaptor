#ifndef DALI_GRAPHICS_VULKAN_SPIRV_H
#define DALI_GRAPHICS_VULKAN_SPIRV_H

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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-shader-create-info.h>
#include <dali/graphics-api/graphics-shader.h>

// INTERNAL INCLUDES
#include <vector>
#include "vulkan-graphics-resource.h"

// EXTERNAL INCLUDES
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

namespace Dali::Graphics::Vulkan
{
class ShaderImpl;

/**
 * Info structure for generating SPIRV bytecode
 */
struct SPIRVGeneratorInfo
{
  std::string   shaderCode;
  PipelineStage pipelineStage;

  struct SPIRVGeneratorExtraInfo* extraInfo; ///< Reserved
};

/**
 * Class generates SPIRV from GLSL
 */
class SPIRVGenerator
{
public:
  /**
   * @brief Constructor
   * @param[in] generatorInfo valid SPIRVGeneratorInfo structure
   */
  explicit SPIRVGenerator(SPIRVGeneratorInfo generatorInfo);

  /**
   * @brief Destructor
   */
  ~SPIRVGenerator();

  /**
   * @brief Triggers compilation to SPIRV
   */
  void Generate();

  /**
   * @brief Checks whether generator is valid
   * @return False if invalid
   */
  [[nodiscard]] bool IsValid() const;

  /**
   * @brief Returns SPIRV buffer
   * @return Array of 32-bit words
   */
  const std::vector<uint32_t>& Get();

private:
  /**
   * Compiles GLSL source to SPIRV
   */
  glslang_shader_t* CompileSource(glslang_stage_t stage, std::string shaderSource);

  std::vector<uint32_t> mBinary;
  SPIRVGeneratorInfo    mGeneratorInfo;
};

} // namespace Dali::Graphics::Vulkan
#endif //DALI_GRAPHICS_VULKAN_SPIRV_H