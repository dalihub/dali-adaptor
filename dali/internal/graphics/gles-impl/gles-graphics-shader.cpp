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

// CLASS HEADER
#include "gles-graphics-shader.h"

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include "egl-graphics-controller.h"

namespace Dali::Graphics::GLES
{
struct Shader::Impl
{
  Impl()  = default;
  ~Impl() = default;

  std::vector<char> source{};
  uint32_t          glShader{};
};

Shader::Shader(const Graphics::ShaderCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: ShaderResource(createInfo, controller)
{
  // push shader to the create queue
  mImpl = std::make_unique<Impl>();

  // Make a copy of source code
  mImpl->source.resize(createInfo.sourceSize);
  std::copy(reinterpret_cast<const char*>(mCreateInfo.sourceData),
            reinterpret_cast<const char*>(mCreateInfo.sourceData) + mCreateInfo.sourceSize,
            mImpl->source.begin());

  // Substitute pointer
  mCreateInfo.sourceData = mImpl->source.data();
}

Shader::~Shader() = default;

bool Shader::Compile() const
{
  auto gl = GetController().GetGL();

  if(!gl)
  {
    return false;
  }

  if(!mImpl->glShader)
  {
    GLenum pipelineStage{0u};
    switch(GetCreateInfo().pipelineStage)
    {
      case Graphics::PipelineStage::TOP_OF_PIPELINE:
      {
        break;
      }
      case Graphics::PipelineStage::VERTEX_SHADER:
      {
        pipelineStage = GL_VERTEX_SHADER;
        break;
      }
      case Graphics::PipelineStage::GEOMETRY_SHADER:
      {
        break;
      }
      case Graphics::PipelineStage::FRAGMENT_SHADER:
      {
        pipelineStage = GL_FRAGMENT_SHADER;
        break;
      }
      case Graphics::PipelineStage::COMPUTE_SHADER:
      {
        break;
      }
      case Graphics::PipelineStage::TESSELATION_CONTROL:
      {
        break;
      }
      case Graphics::PipelineStage::TESSELATION_EVALUATION:
      {
        break;
      }
      case Graphics::PipelineStage::BOTTOM_OF_PIPELINE:
      {
        break;
      }
    }

    if(pipelineStage)
    {
      auto       shader = gl->CreateShader(pipelineStage);
      const auto src    = reinterpret_cast<const char*>(GetCreateInfo().sourceData);
      GLint      size   = GetCreateInfo().sourceSize;
      gl->ShaderSource(shader, 1, const_cast<const char**>(&src), &size);
      gl->CompileShader(shader);

      GLint status{0};
      gl->GetShaderiv(shader, GL_COMPILE_STATUS, &status);
      if(status != GL_TRUE)
      {
        char    output[4096];
        GLsizei size{0u};
        gl->GetShaderInfoLog(shader, 4096, &size, output);
        DALI_LOG_ERROR("Code: %s\n", reinterpret_cast<const char*>(GetCreateInfo().sourceData));
        DALI_LOG_ERROR("glCompileShader() failed: \n%s\n", output);
        gl->DeleteShader(shader);
        return false;
      }
      mImpl->glShader = shader;
    }
    return true;
  }
  return true;
}

uint32_t Shader::GetGLShader() const
{
  return mImpl->glShader;
}

void Shader::DestroyResource()
{
  if(mImpl->glShader)
  {
    auto gl = GetController().GetGL();
    if(!gl)
    {
      return;
    }
    gl->DeleteShader(mImpl->glShader);
  }
}

void Shader::DiscardResource()
{
  GetController().DiscardResource(this);
}

} // namespace Dali::Graphics::GLES
