/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
  return type == GL_SAMPLER_2D || type == GL_SAMPLER_3D;
}

bool SortByLocation(Dali::Graphics::UniformInfo a, Dali::Graphics::UniformInfo b)
{
  return a.location < b.location;
}

} // namespace

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

  gl->GetProgramiv(glProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
  gl->GetProgramiv(glProgram, GL_ACTIVE_ATTRIBUTES, &nAttribs);

  mVertexInputAttributes.clear();
  mVertexInputAttributes.resize(nAttribs);

  name = new GLchar[maxLength];
  for(int i = 0; i < nAttribs; i++)
  {
    gl->GetActiveAttrib(glProgram, i, maxLength, &written, &size, &type, name);
    location = gl->GetAttribLocation(glProgram, name);

    if(location >= 0)
    {
      AttributeInfo attributeInfo;
      attributeInfo.location = location;
      attributeInfo.name     = name;
      attributeInfo.format   = GetVertexAttributeTypeFormat(type);
      mVertexInputAttributes.insert(mVertexInputAttributes.begin() + location, attributeInfo);
    }
  }
  delete[] name;
}

void Reflection::BuildUniformReflection()
{
  auto glProgram = mProgram.GetGlProgram();

  int   maxLen;
  char* name;

  int numUniforms = 0;

  auto gl = mController.GetGL();

  gl->GetProgramiv(glProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
  gl->GetProgramiv(glProgram, GL_ACTIVE_UNIFORMS, &numUniforms);

  mUniformBlocks.clear();
  mDefaultUniformBlock.members.clear();
  mUniformOpaques.clear();

  name = new char[maxLen];

  using UniformLocationSizePair = std::pair<uint32_t, uint32_t>;
  std::vector<UniformLocationSizePair> uniformSizes;

  uniformSizes.reserve(numUniforms);

  for(int i = 0; i < numUniforms; ++i)
  {
    int    size;
    GLenum type;
    int    written;
    gl->GetActiveUniform(glProgram, i, maxLen, &written, &size, &type, name);
    int location = gl->GetUniformLocation(glProgram, name);
    uniformSizes.push_back(std::make_pair(location, GetGLDataTypeSize(type)));

    Dali::Graphics::UniformInfo uniformInfo;
    uniformInfo.name         = name;
    uniformInfo.uniformClass = IsSampler(type) ? Dali::Graphics::UniformClass::COMBINED_IMAGE_SAMPLER : Dali::Graphics::UniformClass::UNIFORM;
    uniformInfo.location     = IsSampler(type) ? 0 : location;
    uniformInfo.binding      = IsSampler(type) ? location : 0;
    uniformInfo.bufferIndex  = 0;

    if(IsSampler(type))
    {
      mUniformOpaques.push_back(uniformInfo);
    }
    else
    {
      mDefaultUniformBlock.members.push_back(uniformInfo);
    }
  }

  // Re-order according to uniform locations.
  if(mDefaultUniformBlock.members.size() > 1)
  {
    std::sort(mDefaultUniformBlock.members.begin(), mDefaultUniformBlock.members.end(), SortByLocation);
  }

  if(mUniformOpaques.size() > 1)
  {
    std::sort(mUniformOpaques.begin(), mUniformOpaques.end(), SortByLocation);
  }

  // Calculate the uniform offset
  for(unsigned int i = 0; i < mDefaultUniformBlock.members.size(); ++i)
  {
    if(i == 0)
    {
      mDefaultUniformBlock.members[i].offset = 0;
    }
    else
    {
      uint32_t previousUniformLocation       = mDefaultUniformBlock.members[i - 1].location;
      auto     previousUniform               = std::find_if(uniformSizes.begin(), uniformSizes.end(), [&previousUniformLocation](const UniformLocationSizePair& iter) { return iter.first == previousUniformLocation; });
      mDefaultUniformBlock.members[i].offset = mDefaultUniformBlock.members[i - 1].offset + previousUniform->second;
    }
  }

  uint32_t lastUniformLocation = mDefaultUniformBlock.members.back().location;
  auto     lastUniform         = std::find_if(uniformSizes.begin(), uniformSizes.end(), [&lastUniformLocation](const UniformLocationSizePair& iter) { return iter.first == lastUniformLocation; });
  mDefaultUniformBlock.size    = mDefaultUniformBlock.members.back().offset + lastUniform->second;

  mUniformBlocks.push_back(mDefaultUniformBlock);

  delete[] name;
}

// TODO: Maybe this is not needed if uniform block is not support by dali shaders?
void Reflection::BuildUniformBlockReflection()
{
  auto gl               = mController.GetGL();
  auto glProgram        = mProgram.GetGlProgram();
  int  numUniformBlocks = 0;
  gl->GetProgramiv(glProgram, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);

  mUniformBlocks.clear();
  mUniformBlocks.resize(numUniformBlocks);

  int uniformBlockMaxLength = 0;
  gl->GetProgramiv(glProgram, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &uniformBlockMaxLength);

  char* uniformBlockName = new char[uniformBlockMaxLength];
  for(int i = 0; i < numUniformBlocks; i++)
  {
    int length;
    int blockBinding;
    int blockDataSize;
    gl->GetActiveUniformBlockName(glProgram, i, uniformBlockMaxLength, &length, uniformBlockName);
    gl->GetActiveUniformBlockiv(glProgram, i, GL_UNIFORM_BLOCK_BINDING, &blockBinding);
    gl->GetActiveUniformBlockiv(glProgram, i, GL_UNIFORM_BLOCK_DATA_SIZE, &blockDataSize);

    Dali::Graphics::UniformBlockInfo uniformBlockInfo;
    uniformBlockInfo.name    = uniformBlockName;
    uniformBlockInfo.size    = blockDataSize;
    uniformBlockInfo.binding = blockBinding;

    int nUnis;
    gl->GetActiveUniformBlockiv(glProgram, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &nUnis);
    int* unifIndexes = new GLint[nUnis];
    gl->GetActiveUniformBlockiv(glProgram, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, unifIndexes);
    char* uniformName{};
    int   maxUniLen;
    gl->GetProgramiv(glProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLen);

    for(int unif = 0; unif < nUnis; ++unif)
    {
      int    uniIndex = unifIndexes[unif];
      int    size;
      GLenum type;

      gl->GetActiveUniform(glProgram, uniIndex, maxUniLen, &length, &size, &type, uniformName);
      int location = gl->GetUniformLocation(glProgram, uniformName);

      Dali::Graphics::UniformInfo uniform;
      uniform.name     = uniformName;
      uniform.location = location;
      uniformBlockInfo.members.push_back(uniform);
    }

    delete[] unifIndexes;

    mUniformBlocks.push_back(uniformBlockInfo);
  }
  delete[] uniformBlockName;
}

uint32_t Reflection::GetVertexAttributeLocation(const std::string& name) const
{
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
  if(location >= mVertexInputAttributes.size())
  {
    return Dali::Graphics::VertexInputAttributeFormat::UNDEFINED;
  }

  return mVertexInputAttributes[location].format;
}

std::string Reflection::GetVertexAttributeName(uint32_t location) const
{
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
    index++;
  }

  // check samplers
  for(auto&& uniform : mUniformOpaques)
  {
    if(uniform.name == name)
    {
      out.uniformClass = Graphics::UniformClass::COMBINED_IMAGE_SAMPLER;
      out.binding      = uniform.binding;
      out.name         = name;
      out.offset       = 0;
      out.location     = uniform.location;
      return true;
    }
  }

  return false;
}

std::vector<Dali::Graphics::UniformInfo> Reflection::GetSamplers() const
{
  return mUniformOpaques;
}

Graphics::ShaderLanguage Reflection::GetLanguage() const
{
  auto gl = mController.GetGL();

  int majorVersion, minorVersion;
  gl->GetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  gl->GetIntegerv(GL_MINOR_VERSION, &minorVersion);
  printf("GL Version (integer) : %d.%d\n", majorVersion, minorVersion);
  printf("GLSL Version : %s\n", gl->GetString(GL_SHADING_LANGUAGE_VERSION));

  // TODO: the language version is hardcoded for now, but we may use what we get
  // from GL_SHADING_LANGUAGE_VERSION?
  return Graphics::ShaderLanguage::GLSL_3_2;
}

} // namespace Dali::Graphics::GLES
