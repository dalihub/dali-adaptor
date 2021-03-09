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
    auto& gl = *mImpl->controller.GetGL();
    gl.DeleteProgram(mImpl->glProgram);
    return true;
  }
  return false;
}

bool ProgramImpl::Create()
{
  // Create and link new program
  auto& gl      = *mImpl->controller.GetGL();
  auto  program = gl.CreateProgram();

  const auto& info = mImpl->createInfo;
  for(const auto& state : *info.shaderState)
  {
    const auto* shader = static_cast<const GLES::Shader*>(state.shader);

    // Compile shader first (ignored when compiled)
    shader->Compile();

    gl.AttachShader(program, shader->GetGLShader());
  }
  gl.LinkProgram(program);

  GLint status{0};
  gl.GetProgramiv(program, GL_LINK_STATUS, &status);
  if(status != GL_TRUE)
  {
    char    output[4096];
    GLsizei size{0u};
    gl.GetProgramInfoLog(program, 4096, &size, output);

    // log on error
    // TODO: un-printf-it
    printf("Log: %s\n", output);
    gl.DeleteProgram(program);
    return false;
  }

  mImpl->glProgram = program;

  // Initialize reflection
  mImpl->reflection->BuildUniformReflection();
  mImpl->reflection->BuildVertexAttributeReflection();

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
