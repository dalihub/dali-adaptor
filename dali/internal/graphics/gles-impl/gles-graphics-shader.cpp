/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <iomanip>
#include "egl-graphics-controller.h"

namespace Dali::Graphics::GLES
{
static std::string AddLineNumbers(const char* src)
{
  int                line = 1;
  std::ostringstream oss;

  // Let we print 4095 prefix of shader codes.
  char  buffer[4096];
  char* copy    = strncpy(buffer, src, 4095);
  char* nextPtr = nullptr;
  buffer[4095]  = '\0';

  char* delim = strtok_r(copy, "\n", &nextPtr);
  while(delim)
  {
    oss << std::setw(4) << line << "  " << delim << "\n";
    delim = strtok_r(nullptr, "\n", &nextPtr);
    ++line;
  }
  return oss.str();
}

struct ShaderImpl::Impl
{
  explicit Impl(Graphics::EglGraphicsController& _controller, const Graphics::ShaderCreateInfo& _createInfo)
  : controller(_controller)
  {
    createInfo.pipelineStage  = _createInfo.pipelineStage;
    createInfo.shaderlanguage = _createInfo.shaderlanguage;
    createInfo.sourceMode     = _createInfo.sourceMode;
    createInfo.shaderVersion  = _createInfo.shaderVersion;

    // Make a copy of source code. if code is meant to be used
    // by modern parser, skip the prefix part
    size_t dataSize;

    ShaderImpl::StripLegacyCodeIfNeeded(_createInfo, sourceOffset, glslVersion, dataSize);

    source.resize(dataSize);
    std::copy(reinterpret_cast<const uint8_t*>(_createInfo.sourceData) + sourceOffset,
              reinterpret_cast<const uint8_t*>(_createInfo.sourceData) + sourceOffset + dataSize,
              source.data());

    // Substitute pointer
    createInfo.sourceData = source.data();
    createInfo.sourceSize = dataSize;
  }

  ~Impl() {};

  bool Compile()
  {
    auto* gl = controller.GetGL();

    if(DALI_UNLIKELY(!gl))
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
        const auto src    = !sourcePreprocessed.empty() ? reinterpret_cast<const char*>(sourcePreprocessed.data()) : reinterpret_cast<const char*>(createInfo.sourceData);

        // null-terminated char already included. So we should remove last character (null terminator) from size.
        GLint size = static_cast<GLint>(!sourcePreprocessed.empty() ? static_cast<uint32_t>(sourcePreprocessed.size()) : createInfo.sourceSize) - 1;

        if(src != nullptr && size >= 0)
        {
          gl->ShaderSource(shader, 1, const_cast<const char**>(&src), &size);
          gl->CompileShader(shader);

          GLint status{0};
          gl->GetShaderiv(shader, GL_COMPILE_STATUS, &status);
          if(status != GL_TRUE)
          {
            char    output[4096];
            GLsizei outputSize{0u};
            gl->GetShaderInfoLog(shader, 4096, &outputSize, output);

            std::string withLines = AddLineNumbers(reinterpret_cast<const char*>(src));
            DALI_LOG_ERROR("Code: \n%s\n", withLines.c_str());
            DALI_LOG_ERROR("glCompileShader() failed: \n%s\n", output);
            gl->DeleteShader(shader);
            return false;
          }
          glShader = shader;
        }
        else
        {
          DALI_LOG_ERROR("glCompileShader() failed: source is nullptr, or size is negative! src : %p, size : %d\n", src, size);
          return false;
        }
      }
      return true;
    }
    return true;
  }

  void Destroy()
  {
    auto* gl = controller.GetGL();

    if(DALI_LIKELY(gl) && glShader)
    {
      gl->DeleteShader(glShader);
      glShader = 0;
    }
  }

  void SetPreprocessedCode(void* data, uint32_t size)
  {
    if(size == 0)
    {
      sourcePreprocessed.clear();
      return;
    }

    const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(data);

    if(*(dataPtr + size - 1) != '\0')
    {
      sourcePreprocessed.resize(size + 1 /* Include null-terminated char */);
      sourcePreprocessed[size] = '\0';
    }
    else
    {
      // null-terminated char already included.
      sourcePreprocessed.resize(size);
    }

    std::copy(dataPtr, dataPtr + size, sourcePreprocessed.data());

    // Update glsl version from preprocessed code
    if(glslVersion == 0u)
    {
      // For legacy shaders we need to make sure that the #version is a very first line
      // so need to strip //@legacy-prefix-end tag
      std::string_view sourceText = {reinterpret_cast<const char*>(sourcePreprocessed.data()), sourcePreprocessed.size()};
      auto             versionPos = sourceText.find("#version", 0);
      if(versionPos == std::string::npos)
      {
        // if there's no version yet it's a legacy shader we assign 100
        glslVersion = 100;
      }
      else
      {
        // save version of legacy shader
        char* end;
        glslVersion = uint32_t(std::strtol(sourceText.data() + versionPos + 9, &end, 10));
      }
    }
  }

  EglGraphicsController& controller;
  ShaderCreateInfo       createInfo;
  std::vector<uint8_t>   source{};
  std::vector<uint8_t>   sourcePreprocessed{};

  size_t sourceOffset{0u}; /// byte offset of source data from original CreateInfo.
                           /// It will be changed after call StripLegacyCodeIfNeeded
                           /// More detail, createInfo.sourceData[0] == source[0] == (original CreateInfo).sourceData[sourceOffset];

  uint32_t glShader{};
  uint32_t refCount{0u};
  uint32_t flushCount{0u};  ///< Number of frames at refCount=0
  uint32_t glslVersion{0u}; ///< 0 - unknown, otherwise valid #version like 130, 300, etc.
};

ShaderImpl::ShaderImpl(const Graphics::ShaderCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
{
  mImpl = std::make_unique<Impl>(controller, createInfo);
}

ShaderImpl::~ShaderImpl()
{
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
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

[[nodiscard]] uint32_t ShaderImpl::GetGLSLVersion() const
{
  return mImpl->glslVersion;
}

[[nodiscard]] size_t ShaderImpl::GetSourceOffset() const
{
  return mImpl->sourceOffset;
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

bool ShaderImpl::HasPreprocessedCode() const
{
  return !mImpl->sourcePreprocessed.empty();
}

std::string_view ShaderImpl::GetPreprocessedCode() const
{
  return {reinterpret_cast<const char*>(mImpl->sourcePreprocessed.data()), mImpl->sourcePreprocessed.size()};
}

[[nodiscard]] EglGraphicsController& ShaderImpl::GetController() const
{
  return mImpl->controller;
}

void ShaderImpl::StripLegacyCodeIfNeeded(const ShaderCreateInfo& info, size_t& startIndex, uint32_t& glslVersion, size_t& finalDataSize)
{
  startIndex = 0u;

  // Fast-out if shader is not a text.
  if(info.sourceMode != ShaderSourceMode::TEXT)
  {
    glslVersion   = info.shaderVersion;
    finalDataSize = info.sourceSize;
    return;
  }

  // Make a copy of source code. if code is meant to be used
  // by modern parser, skip the prefix part
  auto text   = reinterpret_cast<const char*>(info.sourceData);
  auto result = std::string_view(text).find("//@legacy-prefix-end");
  glslVersion = 0u;
  if(info.shaderVersion != 0)
  {
    if(result != 0 && result != std::string::npos)
    {
      DALI_LOG_ERROR("Shader processing: @legacy-prefix-end must be a very first statement!\n");
    }
    else if(result == 0)
    {
      char* end;
      startIndex  = std::strtoul(reinterpret_cast<const char*>(info.sourceData) + 21, &end, 10);
      glslVersion = 0;
    }
  }
  else
  {
    // For legacy shaders we need to make sure that the #version is a very first line
    // so need to strip //@legacy-prefix-end tag
    auto versionPos = std::string_view(text).find("#version", 0);
    if(versionPos == std::string::npos)
    {
      startIndex = 0; // not trimming anything

      // if there's no version yet it's a legacy shader we assign 100
      glslVersion = 100;
    }
    else
    {
      // save version of legacy shader
      char* end;
      glslVersion = uint32_t(std::strtol(std::string_view(text).data() + versionPos + 9, &end, 10));
      startIndex  = versionPos;
    }
  }
  finalDataSize = info.sourceSize - startIndex;
}

void ShaderImpl::SetPreprocessedCode(void* data, uint32_t size)
{
  mImpl->SetPreprocessedCode(data, size);
}

std::string_view ShaderImpl::GetSourceStringView() const
{
  if(HasPreprocessedCode())
  {
    return GetPreprocessedCode();
  }
  return std::string_view(reinterpret_cast<const char*>(mImpl->createInfo.sourceData), mImpl->createInfo.sourceSize);
}

Shader::~Shader()
{
  if(!mShader->Release())
  {
    if(DALI_UNLIKELY(EglGraphicsController::IsShuttingDown()))
    {
      return; // Early out if shutting down
    }

    GetImplementation()->GetController().GetPipelineCache().MarkShaderCacheFlushRequired();
  }
}

[[nodiscard]] const ShaderCreateInfo& Shader::GetCreateInfo() const
{
  return GetImplementation()->GetCreateInfo();
}

void Shader::DiscardResource()
{
  GetImplementation()->GetController().DiscardResource(this);
}

uint32_t Shader::GetGLSLVersion() const
{
  return GetImplementation()->GetGLSLVersion();
}

std::string_view Shader::GetSourceStringView() const
{
  return GetImplementation()->GetSourceStringView();
}

} // namespace Dali::Graphics::GLES
