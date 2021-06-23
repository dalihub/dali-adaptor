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
 */

// CLASS HEADER
#include "gles-graphics-program.h"

// INTERNAL HEADERS
#include "egl-graphics-controller.h"
#include "gles-graphics-reflection.h"
#include "gles-graphics-shader.h"

// EXTERNAL HEADERS
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

namespace Dali::Graphics::GLES
{
using Integration::GlAbstraction;

/**
 * Structure stores pointer to the function
 * which will set the uniform of particular type
 */
struct UniformSetter
{
  union
  {
    void (GlAbstraction::*uniformfProc)(GLint, GLsizei, const float*);
    void (GlAbstraction::*uniformiProc)(GLint, GLsizei, const int*);
    void (GlAbstraction::*uniformMatrixProc)(GLint, GLsizei, GLboolean, const float*);
  };

  enum class Type
  {
    UNDEFINED = 0,
    FLOAT,
    INT,
    MATRIX
  };

  Type type;
};

struct ProgramImpl::Impl
{
  explicit Impl(EglGraphicsController& _controller, const ProgramCreateInfo& info)
  : controller(_controller)
  {
    createInfo = info;
    if(info.shaderState)
    {
      createInfo.shaderState = new std::vector<ShaderState>(*info.shaderState);
    }
  }

  ~Impl()
  {
    delete createInfo.shaderState;
  }

  EglGraphicsController& controller;
  ProgramCreateInfo      createInfo;
  uint32_t               glProgram{};
  uint32_t               refCount{0u};

  std::unique_ptr<GLES::Reflection> reflection{nullptr};

  // Uniform cache
  std::vector<uint8_t> uniformData;

  // List of standalone uniform setters
  std::vector<UniformSetter> uniformSetters;
};

ProgramImpl::ProgramImpl(const Graphics::ProgramCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
{
  // Create implementation
  mImpl = std::make_unique<Impl>(controller, createInfo);

  // Build reflection
  mImpl->reflection = std::make_unique<GLES::Reflection>(*this, controller);
}

ProgramImpl::~ProgramImpl() = default;

bool ProgramImpl::Destroy()
{
  if(mImpl->glProgram)
  {
    auto gl = mImpl->controller.GetGL();
    if(!gl)
    {
      return false;
    }
    gl->DeleteProgram(mImpl->glProgram);
    return true;
  }
  return false;
}

bool ProgramImpl::Create()
{
  // Create and link new program
  auto gl = mImpl->controller.GetGL();
  if(!gl)
  {
    // Do nothing during shutdown
    return false;
  }

  auto program = gl->CreateProgram();

  const auto& info = mImpl->createInfo;
  for(const auto& state : *info.shaderState)
  {
    const auto* shader = static_cast<const GLES::Shader*>(state.shader);

    // Compile shader first (ignored when compiled)
    if(shader->Compile())
    {
      gl->AttachShader(program, shader->GetGLShader());
    }
  }
  gl->LinkProgram(program);

  GLint status{0};
  gl->GetProgramiv(program, GL_LINK_STATUS, &status);
  if(status != GL_TRUE)
  {
    char    output[4096];
    GLsizei size{0u};
    gl->GetProgramInfoLog(program, 4096, &size, output);

    // log on error
    // TODO: un-printf-it
    printf("Log: %s\n", output);
    gl->DeleteProgram(program);
    return false;
  }

  mImpl->glProgram = program;

  // Initialize reflection
  mImpl->reflection->BuildUniformReflection();
  mImpl->reflection->BuildVertexAttributeReflection();

  // populate uniform cache memory for standalone uniforms (it's not needed
  // for real UBOs as real UBOs work with whole memory blocks)
  auto& reflection = mImpl->reflection;
  if(!reflection->GetStandaloneUniformExtraInfo().empty())
  {
    UniformBlockInfo blockInfo;
    mImpl->reflection->GetUniformBlock(0, blockInfo);
    auto uniformCacheSize = blockInfo.size;
    mImpl->uniformData.resize(uniformCacheSize);

    std::fill(mImpl->uniformData.begin(), mImpl->uniformData.end(), 0);

    BuildStandaloneUniformCache();
  }
  return true;
}

uint32_t ProgramImpl::GetGlProgram() const
{
  return mImpl->glProgram;
}

uint32_t ProgramImpl::Retain()
{
  return ++mImpl->refCount;
}

uint32_t ProgramImpl::Release()
{
  return --mImpl->refCount;
}

const GLES::Reflection& ProgramImpl::GetReflection() const
{
  return *mImpl->reflection;
}

bool ProgramImpl::GetParameter(uint32_t parameterId, void* out)
{
  if(parameterId == 1) // a magic number to access program id
  {
    *reinterpret_cast<decltype(&mImpl->glProgram)>(out) = mImpl->glProgram;
    return true;
  }
  return false;
}

EglGraphicsController& ProgramImpl::GetController() const
{
  return mImpl->controller;
}

const ProgramCreateInfo& ProgramImpl::GetCreateInfo() const
{
  return mImpl->createInfo;
}

void ProgramImpl::UpdateStandaloneUniformBlock(const char* ptr)
{
  const auto& reflection = GetReflection();

  const auto& extraInfos = reflection.GetStandaloneUniformExtraInfo();

  auto& gl = *GetController().GetGL();

  // Set uniforms
  int  index    = 0;
  auto cachePtr = reinterpret_cast<char*>(mImpl->uniformData.data());
  for(const auto& info : extraInfos)
  {
    auto& setter = mImpl->uniformSetters[index++];

    auto offset = info.offset;
    switch(setter.type)
    {
      case UniformSetter::Type::FLOAT:
      {
        if(0 != memcmp(&cachePtr[offset], &ptr[offset], info.size * info.arraySize))
        {
          (gl.*(setter.uniformfProc))(info.location, info.arraySize, reinterpret_cast<const float*>(&ptr[offset]));
          memcpy(&cachePtr[offset], &ptr[offset], info.size * info.arraySize);
        }
        break;
      }
      case UniformSetter::Type::INT:
      {
        if(0 != memcmp(&cachePtr[offset], &ptr[offset], info.size * info.arraySize))
        {
          (gl.*(setter.uniformiProc))(info.location, info.arraySize, reinterpret_cast<const int*>(&ptr[offset]));
          memcpy(&cachePtr[offset], &ptr[offset], info.size * info.arraySize);
        }
        break;
      }
      case UniformSetter::Type::MATRIX:
      {
        if(0 != memcmp(&cachePtr[offset], &ptr[offset], info.size * info.arraySize))
        {
          (gl.*(setter.uniformMatrixProc))(info.location, info.arraySize, GL_FALSE, reinterpret_cast<const float*>(&ptr[offset]));
          memcpy(&cachePtr[offset], &ptr[offset], info.size * info.arraySize);
        }
        break;
      }
      case UniformSetter::Type::UNDEFINED:
      {
      }
    }
  }
}

void ProgramImpl::BuildStandaloneUniformCache()
{
  const auto& reflection = GetReflection();
  const auto& extraInfos = reflection.GetStandaloneUniformExtraInfo();

  // Prepare pointers to the uniform setter calls
  mImpl->uniformSetters.resize(extraInfos.size());
  int index = 0;
  for(const auto& info : extraInfos)
  {
    auto type                         = GLTypeConversion(info.type).type;
    mImpl->uniformSetters[index].type = UniformSetter::Type::UNDEFINED;
    switch(type)
    {
      case GLType::FLOAT_VEC2:
      {
        mImpl->uniformSetters[index].uniformfProc = &GlAbstraction::Uniform2fv;
        mImpl->uniformSetters[index].type         = UniformSetter::Type::FLOAT;
        break;
      }
      case GLType::FLOAT_VEC3:
      {
        mImpl->uniformSetters[index].uniformfProc = &GlAbstraction::Uniform3fv;
        mImpl->uniformSetters[index].type         = UniformSetter::Type::FLOAT;
        break;
      }
      case GLType::FLOAT_VEC4:
      {
        mImpl->uniformSetters[index].uniformfProc = &GlAbstraction::Uniform4fv;
        mImpl->uniformSetters[index].type         = UniformSetter::Type::FLOAT;
        break;
      }
      case GLType::INT_VEC2:
      {
        mImpl->uniformSetters[index].uniformiProc = &GlAbstraction::Uniform2iv;
        mImpl->uniformSetters[index].type         = UniformSetter::Type::INT;
        break;
      }
      case GLType::INT_VEC3:
      {
        mImpl->uniformSetters[index].uniformiProc = &GlAbstraction::Uniform3iv;
        mImpl->uniformSetters[index].type         = UniformSetter::Type::INT;
        break;
      }
      case GLType::INT_VEC4:
      {
        mImpl->uniformSetters[index].uniformiProc = &GlAbstraction::Uniform4iv;
        mImpl->uniformSetters[index].type         = UniformSetter::Type::INT;
        break;
      }
      case GLType::BOOL:
      case GLType::BOOL_VEC2:
      case GLType::BOOL_VEC3:
      case GLType::BOOL_VEC4:
      case GLType::FLOAT:
      {
        mImpl->uniformSetters[index].uniformfProc = &GlAbstraction::Uniform1fv;
        mImpl->uniformSetters[index].type         = UniformSetter::Type::FLOAT;
        break;
      }
      case GLType::FLOAT_MAT2:
      {
        mImpl->uniformSetters[index].uniformMatrixProc = &GlAbstraction::UniformMatrix2fv;
        mImpl->uniformSetters[index].type              = UniformSetter::Type::MATRIX;
        break;
      }
      case GLType::FLOAT_MAT3:
      {
        mImpl->uniformSetters[index].uniformMatrixProc = &GlAbstraction::UniformMatrix3fv;
        mImpl->uniformSetters[index].type              = UniformSetter::Type::MATRIX;
        break;
      }
      case GLType::FLOAT_MAT4:
      {
        mImpl->uniformSetters[index].uniformMatrixProc = &GlAbstraction::UniformMatrix4fv;
        mImpl->uniformSetters[index].type              = UniformSetter::Type::MATRIX;
        break;
      }
      case GLType::SAMPLER_2D:
      case GLType::SAMPLER_CUBE:
      default:
      {
      }
    }
    index++;
  }
}

Program::~Program()
{
  // Destroy GL resources of implementation. This should happen
  // only if there's no more pipelines using this program so
  // it is safe to do it in the destructor
  if(!mProgram->Release())
  {
    mProgram->Destroy();
  }
}

const GLES::Reflection& Program::GetReflection() const
{
  return mProgram->GetReflection();
}

EglGraphicsController& Program::GetController() const
{
  return GetImplementation()->GetController();
}

const ProgramCreateInfo& Program::GetCreateInfo() const
{
  return GetImplementation()->GetCreateInfo();
}

void Program::DiscardResource()
{
  GetController().DiscardResource(this);
}

}; // namespace Dali::Graphics::GLES
