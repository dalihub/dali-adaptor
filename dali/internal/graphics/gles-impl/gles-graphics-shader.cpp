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
struct ShaderImpl::Impl
{
  explicit Impl(Graphics::EglGraphicsController& _controller, const Graphics::ShaderCreateInfo& _createInfo)
  : controller(_controller)
  {
    createInfo.pipelineStage  = _createInfo.pipelineStage;
    createInfo.shaderlanguage = _createInfo.shaderlanguage;
    createInfo.sourceMode     = _createInfo.sourceMode;
    createInfo.sourceSize     = _createInfo.sourceSize;

    // Make a copy of source code
    source.resize(_createInfo.sourceSize);
    std::copy(reinterpret_cast<const uint8_t*>(_createInfo.sourceData),
              reinterpret_cast<const uint8_t*>(_createInfo.sourceData) + _createInfo.sourceSize,
              source.data());

    // Substitute pointer
    createInfo.sourceData = source.data();
  }

  ~Impl(){};

  bool Compile()
  {
    auto gl = controller.GetGL();

    if(!gl)
    {
      return false;
    }

    if(!glShader)
    {
      GLenum pipelineStage{0u};
      switch(createInfo.pipelineStage)
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
        const auto src    = reinterpret_cast<const char*>(createInfo.sourceData);
        GLint      size   = createInfo.sourceSize;

        gl->ShaderSource(shader, 1, const_cast<const char**>(&src), &size);
        gl->CompileShader(shader);

        GLint status{0};
        gl->GetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(status != GL_TRUE)
        {
          char    output[4096];
          GLsizei outputSize{0u};
          gl->GetShaderInfoLog(shader, 4096, &outputSize, output);
          DALI_LOG_ERROR("Code: %.*s\n", size, reinterpret_cast<const char*>(createInfo.sourceData));
          DALI_LOG_ERROR("glCompileShader() failed: \n%s\n", output);
          gl->DeleteShader(shader);
          return false;
        }
        glShader = shader;
      }
      return true;
    }
    return true;
  }

  void Destroy()
  {
    auto gl = controller.GetGL();

    if(gl && glShader)
    {
      gl->DeleteShader(glShader);
      glShader = 0;
    }
  }

  EglGraphicsController& controller;
  ShaderCreateInfo       createInfo;
  std::vector<uint8_t>   source{};

  uint32_t glShader{};
  uint32_t refCount{0u};
  uint32_t flushCount{0u}; ///< Number of frames at refCount=0
};

ShaderImpl::ShaderImpl(const Graphics::ShaderCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
{
  mImpl = std::make_unique<Impl>(controller, createInfo);
}

ShaderImpl::~ShaderImpl()
{
  if(!mImpl->controller.IsShuttingDown())
  {
    mImpl->Destroy();
  }
}

uint32_t ShaderImpl::Retain()
{
  mImpl->flushCount = 0;
  return ++mImpl->refCount;
}

uint32_t ShaderImpl::Release()
{
  uint32_t remainingCount = --mImpl->refCount;
  mImpl->flushCount       = 0;
  return remainingCount;
}

[[nodiscard]] uint32_t ShaderImpl::GetRefCount() const
{
  return mImpl->refCount;
}

[[nodiscard]] uint32_t ShaderImpl::IncreaseFlushCount()
{
  return ++mImpl->flushCount;
}

[[nodiscard]] uint32_t ShaderImpl::GetFlushCount() const
{
  return mImpl->flushCount;
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

[[nodiscard]] uint32_t ShaderImpl::GetGLShader() const
{
  return mImpl->glShader;
}

const ShaderCreateInfo& ShaderImpl::GetCreateInfo() const
{
  return mImpl->createInfo;
}

[[nodiscard]] EglGraphicsController& ShaderImpl::GetController() const
{
  return mImpl->controller;
}

Shader::~Shader()
{
  if(!mShader->Release())
  {
    GetImplementation()->GetController().GetPipelineCache().MarkShaderCacheFlushRequired();
  }
}

[[nodiscard]] const ShaderCreateInfo& Shader::GetCreateInfo() const
{
  return GetImplementation()->GetCreateInfo();
}

void Shader::DiscardResource()
{
  auto& controller = GetImplementation()->GetController();
  if(!controller.IsShuttingDown())
  {
    controller.DiscardResource(this);
  }
}

} // namespace Dali::Graphics::GLES
