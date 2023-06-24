/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

#include "gles-graphics-reflection.h"

#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

#include <vector>
#include "egl-graphics-controller.h"

#include <GLES3/gl3.h>
#include <GLES3/gl31.h>

#include "gles-graphics-program.h"

#include <iostream>

namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gGraphicsReflectionLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_GRAPHICS_REFLECTION");
#endif

struct StringSize
{
  const char* const mString;
  const uint32_t    mLength;

  template<uint32_t kLength>
  constexpr StringSize(const char (&string)[kLength])
  : mString(string),
    mLength(kLength - 1) // remove terminating null; N.B. there should be no other null.
  {
  }

  operator const char*() const
  {
    return mString;
  }
};

bool operator==(const StringSize& lhs, const char* rhs)
{
  return strncmp(lhs.mString, rhs, lhs.mLength) == 0;
}

const char* const    DELIMITERS           = " \t\n";
const char* const    DELIMITERS_INC_INDEX = " \t\n[]";
constexpr StringSize UNIFORM{"uniform"};
constexpr StringSize SAMPLER_PREFIX{"sampler"};
constexpr StringSize SAMPLER_TYPES[]   = {"2D", "Cube", "ExternalOES"};
constexpr auto       END_SAMPLER_TYPES = SAMPLER_TYPES + std::extent<decltype(SAMPLER_TYPES)>::value;

Dali::Graphics::VertexInputAttributeFormat GetVertexAttributeTypeFormat(GLenum type)
{
  switch(type)
  {
    case GL_FLOAT:
      return Dali::Graphics::VertexInputAttributeFormat::FLOAT;
    case GL_FLOAT_VEC2:
      return Dali::Graphics::VertexInputAttributeFormat::VEC2;
    case GL_FLOAT_VEC3:
      return Dali::Graphics::VertexInputAttributeFormat::VEC3;
    case GL_FLOAT_VEC4:
      return Dali::Graphics::VertexInputAttributeFormat::VEC4;
    case GL_INT:
      return Dali::Graphics::VertexInputAttributeFormat::INTEGER;
    default:
      return Dali::Graphics::VertexInputAttributeFormat::UNDEFINED;
  }
}

uint32_t GetGLDataTypeSize(GLenum type)
{
  // There are many more types than what are covered here, but
  // they are not supported in dali.
  switch(type)
  {
    case GL_FLOAT: // "float", 1 float, 4 bytes
      return 4;
    case GL_FLOAT_VEC2: // "vec2", 2 floats, 8 bytes
      return 8;
    case GL_FLOAT_VEC3: // "vec3", 3 floats, 12 bytes
      return 12;
    case GL_FLOAT_VEC4: // "vec4", 4 floats, 16 bytes
      return 16;
    case GL_INT: // "int", 1 integer, 4 bytes
      return 4;
    case GL_FLOAT_MAT2: // "mat2", 4 floats, 16 bytes
      return 16;
    case GL_FLOAT_MAT3: // "mat3", 3 vec3, 36 bytes
      return 36;
    case GL_FLOAT_MAT4: // "mat4", 4 vec4, 64 bytes
      return 64;
    default:
      return 0;
  }
}

bool IsSampler(GLenum type)
{
  return type == GL_SAMPLER_2D || type == GL_SAMPLER_3D || type == GL_SAMPLER_CUBE || type == GL_SAMPLER_EXTERNAL_OES;
}

std::string GetShaderSource(Dali::Graphics::ShaderState shaderState)
{
  std::vector<uint8_t> data;
  auto*                shader           = static_cast<const Dali::Graphics::GLES::Shader*>(shaderState.shader);
  auto&                shaderCreateInfo = shader->GetCreateInfo();
  data.resize(shaderCreateInfo.sourceSize + 1);
  std::memcpy(&data[0], shaderCreateInfo.sourceData, shaderCreateInfo.sourceSize);
  data[shaderCreateInfo.sourceSize] = 0;

  return std::string(reinterpret_cast<char*>(&data[0]));
}

void ParseShaderSamplers(std::string shaderSource, std::vector<Dali::Graphics::UniformInfo>& uniformOpaques, int& samplerPosition, std::vector<int>& samplerPositions)
{
  if(!shaderSource.empty())
  {
    char* shaderStr = strdup(shaderSource.c_str());
    char* uniform   = strstr(shaderStr, UNIFORM);

    while(uniform)
    {
      // From "uniform" to ";", not ignoring comments.
      char* outerToken = strtok_r(uniform + UNIFORM.mLength, ";", &uniform);

      char* nextPtr = nullptr;
      char* token   = strtok_r(outerToken, DELIMITERS, &nextPtr);
      while(token)
      {
        // Ignore any token up to "sampler"
        if(SAMPLER_PREFIX == token)
        {
          token += SAMPLER_PREFIX.mLength;
          if(std::find(SAMPLER_TYPES, END_SAMPLER_TYPES, token) != END_SAMPLER_TYPES)
          {
            bool found(false);
            // We now are at next token after "samplerxxx" in outerToken token "stream"

            // Does it use array notation?
            int  arraySize = 0; // 0 = No array
            auto iter      = std::string(token).find("[", 0);
            if(iter != std::string::npos)
            {
              // Get Array size from source. (Warning, may be higher than GetActiveUniform suggests)
              iter++;
              arraySize = int(strtol(token + iter, nullptr, 0));
            }

            token = strtok_r(nullptr, DELIMITERS_INC_INDEX, &nextPtr); // " ", "\t", "\n", "[", "]"

            for(uint32_t i = 0; i < static_cast<uint32_t>(uniformOpaques.size()); ++i)
            {
              if(samplerPositions[i] == -1 &&
                 strncmp(token, uniformOpaques[i].name.c_str(), uniformOpaques[i].name.size()) == 0)
              {
                // We have found a matching name.
                samplerPositions[i] = uniformOpaques[i].offset = samplerPosition;
                if(arraySize == 0)
                {
                  ++samplerPosition;
                }
                else
                {
                  samplerPosition += arraySize;
                }
                found = true;
                break;
              }
            }

            if(!found)
            {
              DALI_LOG_INFO(gGraphicsReflectionLogFilter, Debug::General, "Sampler uniform %s declared but not used in the shader\n", token);
            }
            break;
          }
        }

        token = strtok_r(nullptr, DELIMITERS, &nextPtr);
      }

      uniform = strstr(uniform, UNIFORM);
    }
    free(shaderStr);
  }
}

} // anonymous namespace

namespace Dali::Graphics::GLES
{
Reflection::Reflection(GLES::ProgramImpl& program, Graphics::EglGraphicsController& controller)
: Graphics::Reflection(),
  mController(controller),
  mProgram(program)
{
}

Reflection::~Reflection() = default;

void Reflection::BuildVertexAttributeReflection()
{
  auto glProgram = mProgram.GetGlProgram();

  int    written, size, location, maxLength, nAttribs;
  GLenum type;
  char*  name;

  auto gl = mController.GetGL();
  if(!gl)
  {
    // Do nothing during shutdown
    return;
  }

  DALI_LOG_INFO(gGraphicsReflectionLogFilter, Debug::General, "Build vertex attribute reflection for glProgram : %u\n", glProgram);

  gl->GetProgramiv(glProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
  gl->GetProgramiv(glProgram, GL_ACTIVE_ATTRIBUTES, &nAttribs);

  mVertexInputAttributes.clear();
  mVertexInputAttributes.resize(nAttribs);

  int maximumLocation = nAttribs - 1;

  name = new GLchar[maxLength];
  for(int i = 0; i < nAttribs; i++)
  {
    gl->GetActiveAttrib(glProgram, i, maxLength, &written, &size, &type, name);
    location = gl->GetAttribLocation(glProgram, name);

    if(location >= 0)
    {
      if(maximumLocation < location)
      {
        maximumLocation = location;
        // Increate continer size s.t. we can use maximumLocation as index.
        mVertexInputAttributes.resize(maximumLocation + 1u);
      }

      AttributeInfo attributeInfo;
      attributeInfo.location           = location;
      attributeInfo.name               = name;
      attributeInfo.format             = GetVertexAttributeTypeFormat(type);
      mVertexInputAttributes[location] = std::move(attributeInfo);
    }
  }

  delete[] name;
}

void Reflection::BuildUniformBlockReflection()
{
  auto  gl        = mController.GetGL();
  auto  glProgram = mProgram.GetGlProgram();
  char* name;
  int   numUniformBlocks = 0;

  if(!gl)
  {
    // Do nothing during shutdown
    return;
  }

  DALI_LOG_INFO(gGraphicsReflectionLogFilter, Debug::General, "Build uniform block reflection for glProgram : %u\n", glProgram);

  int   maxUniformNameLength;
  GLint activeUniformCount = 0;
  gl->GetProgramiv(glProgram, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);
  gl->GetProgramiv(glProgram, GL_ACTIVE_UNIFORMS, &activeUniformCount);
  gl->GetProgramiv(glProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

  numUniformBlocks++; // add block 0 for standalone UBO block

  mUniformBlocks.resize(numUniformBlocks);
  mUniformOpaques.clear();

  mStandaloneUniformExtraInfos.clear();

  std::vector<GLuint> uniformIndices;
  uniformIndices.reserve(activeUniformCount);
  for(auto i = 0; i < activeUniformCount; ++i)
  {
    uniformIndices.emplace_back(i);
  }

  // Obtain all parameters for active uniforms
  auto getActiveUniformParams = [gl, glProgram, uniformIndices](GLenum param) {
    std::vector<GLint> params;
    params.resize(uniformIndices.size());
    gl->GetActiveUniformsiv(glProgram, uniformIndices.size(), uniformIndices.data(), param, params.data());
    return params;
  };

  auto activeUniformType       = getActiveUniformParams(GL_UNIFORM_TYPE);
  auto activeUniformSize       = getActiveUniformParams(GL_UNIFORM_SIZE);
  auto activeUniformNameLength = getActiveUniformParams(GL_UNIFORM_NAME_LENGTH);
  auto activeUniformBlockIndex = getActiveUniformParams(GL_UNIFORM_BLOCK_INDEX);
  auto activeUniformOffset     = getActiveUniformParams(GL_UNIFORM_OFFSET);

  // Extract only uniform blocks and collect data
  std::string uniformName;

  // collect samplers into separate array
  std::vector<UniformInfo> samplers;

  name = new char[maxUniformNameLength + 1];

  for(auto i = 0u; i < activeUniformBlockIndex.size(); ++i)
  {
    GLenum type;
    GLint  elementCount;
    GLint  written;
    gl->GetActiveUniform(glProgram, i, maxUniformNameLength, &written, &elementCount, &type, name);

    auto location = gl->GetUniformLocation(glProgram, name);
    uniformName   = name;

    UniformInfo* uniformInfo{nullptr};
    if(IsSampler(activeUniformType[i]))
    {
      samplers.emplace_back();
      uniformInfo               = &samplers.back();
      uniformInfo->uniformClass = UniformClass::COMBINED_IMAGE_SAMPLER;
    }
    else
    {
      auto  blockIndex = activeUniformBlockIndex[i] + 1;
      auto& members    = mUniformBlocks[blockIndex].members;
      members.emplace_back();
      uniformInfo               = &members.back();
      uniformInfo->uniformClass = UniformClass::UNIFORM;
      uniformInfo->binding      = 0;
      uniformInfo->bufferIndex  = blockIndex;
      uniformInfo->binding      = blockIndex == 0 ? i : 0; // this will be reset later
      uniformInfo->offset       = activeUniformOffset[i];
    }

    uniformInfo->location = location; // location must be set later and sorted by offset
    uniformInfo->name     = name;
    // Strip off array index from name, use element count instead
    if(elementCount > 1)
    {
      auto iter = std::string(uniformName).find('[', 0);
      if(iter != std::string::npos)
      {
        uniformInfo->name         = std::string(name).substr(0, iter);
        uniformInfo->elementCount = elementCount;
      }
    }
  }

  // Sort by offset
  uint32_t blockIndex = 0;
  for(auto& ubo : mUniformBlocks)
  {
    std::sort(ubo.members.begin(), ubo.members.end(), [](auto& lhs, auto& rhs) {
      return lhs.offset < rhs.offset;
    });

    if(blockIndex > 0)
    {
      GLint uboSize;
      GLint blockNameLength;
      gl->GetActiveUniformBlockiv(glProgram, blockIndex - 1, GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);
      gl->GetActiveUniformBlockiv(glProgram, blockIndex - 1, GL_UNIFORM_BLOCK_NAME_LENGTH, &blockNameLength);
      char* blockName = new char[blockNameLength];
      gl->GetActiveUniformBlockName(glProgram, blockIndex - 1, blockNameLength, nullptr, blockName);
      ubo.name = blockName;
      ubo.size = uboSize;
      delete[] blockName;
    }
    else
    {
      ubo.name = "";
      ubo.size = 0; // to compute later
    }
    ubo.binding       = 0;
    ubo.descriptorSet = 0;
    blockIndex++;
  }

  // count uniform size
  auto& defaultUniformBlock = mUniformBlocks[0]; // Standalone block
  defaultUniformBlock.size  = 0;

  mStandaloneUniformExtraInfos.reserve(defaultUniformBlock.members.size());
  for(auto& member : defaultUniformBlock.members)
  {
    auto& type         = activeUniformType[member.binding];
    auto  dataTypeSize = GetGLDataTypeSize(type);
    member.offset      = defaultUniformBlock.size;
    defaultUniformBlock.size += (dataTypeSize * activeUniformSize[member.binding]);

    UniformExtraInfo extraInfo{};
    extraInfo.location  = member.location;
    extraInfo.size      = (dataTypeSize);
    extraInfo.offset    = member.offset;
    extraInfo.arraySize = activeUniformSize[member.binding];
    extraInfo.type      = activeUniformType[member.binding];
    member.binding      = 0;
    mStandaloneUniformExtraInfos.emplace_back(extraInfo);
  }

  mUniformOpaques = samplers;
  SortOpaques();
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
    return std::string();
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
    const auto& memberUniform   = block.members[i];
    out.members[i].name         = memberUniform.name;
    out.members[i].binding      = block.binding;
    out.members[i].uniformClass = Graphics::UniformClass::UNIFORM;
    out.members[i].offset       = memberUniform.offset;
    out.members[i].location     = memberUniform.location;
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
    return std::string();
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
    return std::string();
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

std::vector<GLenum> Reflection::GetStandaloneUniformTypes() const
{
  std::vector<GLenum> retval{};
  for(auto&& uniform : mStandaloneUniformExtraInfos)
  {
    retval.emplace_back(uniform.type);
  }

  return retval;
}

const std::vector<Reflection::UniformExtraInfo>& Reflection::GetStandaloneUniformExtraInfo() const
{
  return mStandaloneUniformExtraInfos;
}

const std::vector<Dali::Graphics::UniformInfo>& Reflection::GetSamplers() const
{
  return mUniformOpaques;
}

Graphics::ShaderLanguage Reflection::GetLanguage() const
{
  auto version = Graphics::ShaderLanguage::GLSL_3_2;

  auto gl = mController.GetGL();
  if(!gl)
  {
    // Do nothing during shutdown
    return version;
  }

  int majorVersion, minorVersion;
  gl->GetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  gl->GetIntegerv(GL_MINOR_VERSION, &minorVersion);
  DALI_LOG_RELEASE_INFO("GL Version (integer) : %d.%d\n", majorVersion, minorVersion);
  DALI_LOG_RELEASE_INFO("GLSL Version : %s\n", gl->GetString(GL_SHADING_LANGUAGE_VERSION));

  // TODO: the language version is hardcoded for now, but we may use what we get
  // from GL_SHADING_LANGUAGE_VERSION?
  return version;
}

void Reflection::SortOpaques()
{
  //Determine declaration order of each sampler
  auto& programCreateInfo = mProgram.GetCreateInfo();

  std::vector<uint8_t> data;
  std::string          vertShader;
  std::string          fragShader;

  for(auto& shaderState : *programCreateInfo.shaderState)
  {
    if(shaderState.pipelineStage == PipelineStage::VERTEX_SHADER)
    {
      vertShader = GetShaderSource(shaderState);
    }
    else if(shaderState.pipelineStage == PipelineStage::FRAGMENT_SHADER)
    {
      fragShader = GetShaderSource(shaderState);
    }
  }

  int              samplerPosition = 0;
  std::vector<int> samplerPositions(mUniformOpaques.size(), -1);

  ParseShaderSamplers(vertShader, mUniformOpaques, samplerPosition, samplerPositions);
  ParseShaderSamplers(fragShader, mUniformOpaques, samplerPosition, samplerPositions);

  std::sort(mUniformOpaques.begin(), mUniformOpaques.end(), [](const UniformInfo& a, const UniformInfo& b) { return a.offset < b.offset; });
}

} // namespace Dali::Graphics::GLES
