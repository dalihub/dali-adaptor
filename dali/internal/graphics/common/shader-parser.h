#ifndef DALI_INTERNAL_GRAPHICS_SHADER_PARSER_H
#define DALI_INTERNAL_GRAPHICS_SHADER_PARSER_H

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
#include <dali/devel-api/common/map-wrapper.h>

#include <cstdint>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace Dali::Internal::ShaderParser
{
/**
 * Defines parser shader stages
 */
enum class ShaderStage
{
  VERTEX,
  FRAGMENT
};

/**
 * Defines output shader GLSL dialects
 */
enum class OutputLanguage
{
  GLSL_100_ES = 100,
  GLSL_3      = 300,
  GLSL_300_ES = GLSL_3,
  GLSL_310_ES = 310,
  GLSL_320_ES = 320,
  GLSL_3_MAX, // end of GLSL3xx list
  SPIRV_GLSL
};

using CodeTokenPair = std::pair<int, int>;
struct CodeLine
{
  std::vector<CodeTokenPair> tokens;
  std::vector<std::string>   replacement;
  std::string                line;
};

struct Shader
{
  std::vector<CodeLine> codeLines;
  std::string           output;
  int                   customOutputLineIndex;
  int                   mainLine;
};

struct Program
{
  Shader                     vertexShader;
  Shader                     fragmentShader;
  std::map<std::string, int> varyings;
  int                        uboBinding{0};
  int&                       samplerBinding{uboBinding}; // sampler bindings and ubo bindings are the same
  int                        attributeLocation{0};

  std::vector<std::pair<std::string, uint32_t>> uniformBlocks;
};

struct ShaderParserInfo
{
  const std::string* vertexShaderCode;
  const std::string* fragmentShaderCode;

  uint32_t vertexShaderLegacyVersion;
  uint32_t fragmentShaderLegacyVersion;

  OutputLanguage language;
  uint32_t       outputVersion;

  std::string vertexShaderPrefix;   // this code will be added right after #version
  std::string fragmentShaderPrefix; // this code will be added right after #version
};

/**
 * Parses given source code and returns requested variant of shader
 * @param[in] parseInfo Valid ShaderParserInfo structure
 * @param[out] output Output strings
 */
void Parse(const ShaderParserInfo& parseInfo, std::vector<std::string>& output);

} // namespace Dali::Internal::ShaderParser

#endif