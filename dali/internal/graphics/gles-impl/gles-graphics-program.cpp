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
 */

// CLASS HEADER
#include "gles-graphics-program.h"

// INTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/file-loader.h>
#include <dali/internal/graphics/common/shader-parser.h>
#include <dali/public-api/dali-adaptor-version.h>
#include "egl-graphics-controller.h"
#include "gles-graphics-reflection.h"
#include "gles-graphics-shader.h"

// EXTERNAL HEADERS
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <iostream>

static constexpr const char* FRAGMENT_SHADER_ADVANCED_BLEND_EQUATION_PREFIX =
  "#ifdef GL_KHR_blend_equation_advanced\n"
  "#extension GL_KHR_blend_equation_advanced : enable\n"
  "#endif\n"

  "#if defined(GL_KHR_blend_equation_advanced) || __VERSION__>=320\n"
  "  layout(blend_support_all_equations) out;\n"
  "#endif\n";

#if defined(DEBUG_ENABLED)
Debug::Filter* gGraphicsProgramLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_GRAPHICS_PROGRAM");
#endif

extern std::string GetSystemProgramBinaryPath();

namespace
{
const char* VERSION_SEPARATOR = "-";
const char* SHADER_SUFFIX     = ".shader";
} // namespace

namespace Dali::Graphics::GLES
{
using Integration::GlAbstraction;

/**
 * Memory compare working on 4-byte types. Since all types used in shaders are
 * size of 4*N then no need for size and alignment checks.
 */
template<class A, class B>
inline bool memcmp4(A* a, B* b, uint32_t size)
{
  auto* pa = reinterpret_cast<const uint32_t*>(a);
  auto* pb = reinterpret_cast<const uint32_t*>(b);
  size >>= 2;
  while(size-- && *pa++ == *pb++)
    ;
  return (-1u == size);
};

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

    // Create new reference of std::string_view.
    name            = std::string(info.name);
    createInfo.name = name;
  }

  ~Impl()
  {
    delete createInfo.shaderState;
  }

  EglGraphicsController& controller;
  ProgramCreateInfo      createInfo;
  std::string            name;
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
  if(DALI_UNLIKELY(EglGraphicsController::IsShuttingDown()))
  {
    return false; // Early out if shutting down
  }

  if(mImpl->glProgram)
  {
    auto* gl = mImpl->controller.GetGL();
    if(DALI_LIKELY(gl))
    {
      gl->DeleteProgram(mImpl->glProgram);
      return true;
    }
  }
  return false;
}

void ProgramImpl::Preprocess()
{
  auto* gl               = mImpl->controller.GetGL();
  bool  advancedBlending = DALI_LIKELY(gl) ? gl->IsAdvancedBlendEquationSupported() : false;
  // For now only Vertex and Fragment shader stages supported
  // and one per stage
  std::string  vertexString;
  std::string  fragmentString;
  std::string* currentString = nullptr;

  const GLES::Shader* vsh = nullptr;
  const GLES::Shader* fsh = nullptr;

  const auto& info = mImpl->createInfo;

  for(const auto& state : *info.shaderState)
  {
    const auto* shader = static_cast<const GLES::Shader*>(state.shader);
    if(shader && state.pipelineStage == PipelineStage::VERTEX_SHADER)
    {
      // Only TEXT source mode can be processed
      currentString = &vertexString;
      vsh           = shader;
    }
    else if(shader && state.pipelineStage == PipelineStage::FRAGMENT_SHADER)
    {
      // Only TEXT source mode can be processed
      currentString = &fragmentString;
      fsh           = shader;
    }
    else
    {
      // no valid stream to push
      currentString = nullptr;
      DALI_LOG_ERROR("Shader state contains invalid shader source (most likely binary)! Can't process!\n");
    }

    // Check if stream valid
    if(currentString && currentString->empty() && shader && shader->GetCreateInfo().sourceMode == ShaderSourceMode::TEXT)
    {
      *currentString = std::string(reinterpret_cast<const char*>(shader->GetCreateInfo().sourceData),
                                   shader->GetCreateInfo().sourceSize);
    }
    else
    {
      if(shader)
      {
        DALI_LOG_ERROR("Preprocessing of binary shaders isn't allowed!\n");
      }
      else
      {
        DALI_LOG_ERROR("Shader state contains invalid shader pointer! pipeline stage : %d\n", static_cast<int>(state.pipelineStage));
      }
    }
  }

  // if we have both streams ready
  if(!vertexString.empty() && !fragmentString.empty())
  {
    // In case we have one modern shader and one legacy counterpart we need to enforce
    // output language.
    Internal::ShaderParser::ShaderParserInfo parseInfo{};
    parseInfo.vertexShaderCode            = &vertexString;
    parseInfo.fragmentShaderCode          = &fragmentString;
    parseInfo.vertexShaderLegacyVersion   = vsh->GetGLSLVersion();
    parseInfo.fragmentShaderLegacyVersion = fsh->GetGLSLVersion();

    if(advancedBlending)
    {
      parseInfo.fragmentShaderPrefix = FRAGMENT_SHADER_ADVANCED_BLEND_EQUATION_PREFIX;
    }

    // set up language dialect for parsed shader
    auto glslVersion        = mImpl->controller.GetGraphicsInterface()->GetShaderLanguageVersion();
    parseInfo.language      = Internal::ShaderParser::OutputLanguage(glslVersion); // We default to GLSL3
    parseInfo.outputVersion = std::max(vsh->GetGLSLVersion(), fsh->GetGLSLVersion());

    std::vector<std::string> newShaders;

    Internal::ShaderParser::Parse(parseInfo, newShaders);

    // substitute shader code
    vsh->GetImplementation()->SetPreprocessedCode(newShaders[0].data(), newShaders[0].size());
    fsh->GetImplementation()->SetPreprocessedCode(newShaders[1].data(), newShaders[1].size());
  }
  else
  {
    DALI_LOG_ERROR("Preprocessing shader code failed!");
  }
}

bool ProgramImpl::Create()
{
  // Create and link new program
  auto* gl = mImpl->controller.GetGL();
  if(DALI_UNLIKELY(!gl))
  {
    // Do nothing during shutdown
    return false;
  }

  auto program = gl->CreateProgram();

  DALI_LOG_DEBUG_INFO("Program[%s] create program id : %u\n", mImpl->name.c_str(), program);

  mImpl->glProgram = program;

  const auto& info = mImpl->createInfo;
  Preprocess();
  DALI_LOG_DEBUG_INFO("Program[%s] pre-process finish for program id : %u\n", mImpl->name.c_str(), program);

  auto cachedProgramBinary = false;

  if(IsEnableProgramBinary())
  {
    DALI_LOG_DEBUG_INFO("[Enable] Shader program binary, Try load program binary. \n");
    cachedProgramBinary = LoadProgramBinary();
  }

  if(!cachedProgramBinary)
  {
    for(const auto& state : *info.shaderState)
    {
      const auto* shader = static_cast<const GLES::Shader*>(state.shader);

      // Compile shader first (ignored when compiled)
      if(shader->GetImplementation()->Compile())
      {
        auto shaderId = shader->GetImplementation()->GetGLShader();
        DALI_LOG_DEBUG_INFO("Program[%s] attach shader : %u\n", mImpl->name.c_str(), shaderId);
        gl->AttachShader(program, shaderId);
      }
    }

    DALI_LOG_DEBUG_INFO("Program[%s] call glLinkProgram\n", mImpl->name.c_str());
    gl->LinkProgram(program);
  }
  else
  {
    DALI_LOG_DEBUG_INFO("ProgramBinary[%s] is already been created. Skip glCompile and glLink \n", mImpl->name.c_str());
  }

  GLint status{0};
  gl->GetProgramiv(program, GL_LINK_STATUS, &status);
  if(status != GL_TRUE)
  {
    char    output[4096];
    GLsizei size{0u};
    gl->GetProgramInfoLog(program, 4096, &size, output);

    // log on error
    DALI_LOG_ERROR("glLinkProgram[%s] failed:\n%s\n", mImpl->name.c_str(), output);
    gl->DeleteProgram(program);
    mImpl->glProgram = 0u;
    return false;
  }

  if(IsEnableProgramBinary() && !cachedProgramBinary)
  {
    SaveProgramBinary();
  }

  // Initialize reflection
  mImpl->reflection->BuildVertexAttributeReflection();
  mImpl->reflection->BuildUniformBlockReflection();

  // populate uniform cache memory for standalone uniforms (it's not needed
  // for real UBOs as real UBOs work with whole memory blocks)
  auto& reflection = mImpl->reflection;
  if(!reflection->GetStandaloneUniformExtraInfo().empty())
  {
    UniformBlockInfo blockInfo;
    reflection->GetUniformBlock(0, blockInfo);
    auto uniformCacheSize = blockInfo.size;
    mImpl->uniformData.resize(uniformCacheSize);

    std::fill(mImpl->uniformData.begin(), mImpl->uniformData.end(), 0);

    BuildStandaloneUniformCache();
  }

  // Set up uniform block bindings
  auto blockCount = reflection->GetUniformBlockCount();
  for(uint32_t i = 1; i < blockCount; ++i) // Ignore emulated block at #0
  {
    UniformBlockInfo uboInfo{};
    reflection->GetUniformBlock(i, uboInfo);

    // make binding point
    auto blockIndex = gl->GetUniformBlockIndex(program, uboInfo.name.c_str());
    gl->UniformBlockBinding(program, blockIndex, uboInfo.binding);
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

uint32_t ProgramImpl::GetRefCount() const
{
  return mImpl->refCount;
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

  if(DALI_UNLIKELY(EglGraphicsController::IsShuttingDown()))
  {
    return; // Early out if shutting down
  }

  auto* gl = GetController().GetGL();
  if(DALI_UNLIKELY(!gl))
  {
    return; // Early out if no GL found
  }

  // Set uniforms
  int  index    = 0;
  auto cachePtr = reinterpret_cast<char*>(mImpl->uniformData.data());
  for(const auto& info : extraInfos)
  {
    auto& setter = mImpl->uniformSetters[index++];
    auto  offset = info.offset;
    if(!memcmp4(&cachePtr[offset], &ptr[offset], info.size * info.arraySize))
    {
      switch(setter.type)
      {
        case UniformSetter::Type::FLOAT:
        {
          (gl->*(setter.uniformfProc))(info.location, info.arraySize, reinterpret_cast<const float*>(&ptr[offset]));
          break;
        }
        case UniformSetter::Type::INT:
        {
          (gl->*(setter.uniformiProc))(info.location, info.arraySize, reinterpret_cast<const int*>(&ptr[offset]));
          break;
        }
        case UniformSetter::Type::MATRIX:
        {
          (gl->*(setter.uniformMatrixProc))(info.location, info.arraySize, GL_FALSE, reinterpret_cast<const float*>(&ptr[offset]));
          break;
        }
        case UniformSetter::Type::UNDEFINED:
        {
        }
      }
    }
  }
  // Update caches
  memmove(mImpl->uniformData.data(), ptr, mImpl->uniformData.size());
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
      case GLType::INT:
      {
        mImpl->uniformSetters[index].uniformiProc = &GlAbstraction::Uniform1iv;
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

bool ProgramImpl::IsEnableProgramBinary() const
{
  if(mImpl->controller.IsUsingProgramBinary())
  {
    const auto& info = mImpl->createInfo;
    if(info.useFileCache)
    {
      return true;
    }

    // If the Hint of shader is not Shader::Hint::FILE_CACHE_SUPPORT, we can't enable program binary
    DALI_LOG_DEBUG_INFO("[Enable] Shader program binary, but this shader[%s] cannot be use file caching. because Shader::Hint::FILE_CACHE_SUPPORT is not set \n", mImpl->name.c_str());
  }

  return false;
}
std::string ProgramImpl::GetProgramBinaryName()
{
  // Check shader with dali-version, name and total shader size
  const auto& info            = mImpl->createInfo;
  uint32_t    totalShaderSize = 0u;
  for(const auto& state : *info.shaderState)
  {
    const auto* shader = static_cast<const GLES::Shader*>(state.shader);
    totalShaderSize += shader->GetCreateInfo().sourceSize;
  }

  std::string programBinaryName = std::to_string(ADAPTOR_MAJOR_VERSION) + VERSION_SEPARATOR + std::to_string(ADAPTOR_MINOR_VERSION) + VERSION_SEPARATOR + std::to_string(ADAPTOR_MICRO_VERSION) + VERSION_SEPARATOR + mImpl->name + VERSION_SEPARATOR + std::to_string(totalShaderSize) + SHADER_SUFFIX;
  return programBinaryName;
}

bool ProgramImpl::LoadProgramBinary()
{
  auto binaryShaderFilename = GetSystemProgramBinaryPath() + GetProgramBinaryName();

  bool               result = false;
  Dali::Vector<char> buffer;
  result = Dali::FileLoader::ReadFile(binaryShaderFilename, buffer);

  if(result)
  {
    if(buffer.Size() == 0)
    {
      DALI_LOG_ERROR("Can't load binary shader from file [%s]\n", binaryShaderFilename.c_str());
      return false;
    }

    auto* gl = mImpl->controller.GetGL();
    if(DALI_UNLIKELY(!gl))
    {
      DALI_LOG_ERROR("Can't Get GL \n");
      return false;
    }

    // if formatsLength is less than 0 or greater than 2, it is difficult to guarantee accurate operation.
    // currently, we don't consider having multiple formats. so it is implemented to operate only when there is one format.
    GLint formatsLength = -1;
    gl->GetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formatsLength);
    if(formatsLength != 1)
    {
      DALI_LOG_ERROR("GL_NUM_PROGRAM_BINARY_FORMATS return invalid value : %d. Load failed \n", formatsLength);
      return false;
    }

    std::vector<GLint> formats(formatsLength);
    gl->GetIntegerv(GL_PROGRAM_BINARY_FORMATS, &formats[0]);
    if(!formats[0])
    {
      DALI_LOG_ERROR("GL_PROGRAM_BINARY_FORMATS is failed \n");
      return false;
    }

    gl->ProgramBinary(mImpl->glProgram, formats[0], buffer.Begin(), buffer.Size());

    GLint status{0};
    gl->GetProgramiv(mImpl->glProgram, GL_LINK_STATUS, &status);
    if(status != GL_TRUE)
    {
      char    output[4096];
      GLsizei size{0u};
      gl->GetProgramInfoLog(mImpl->glProgram, 4096, &size, output);

      // log on error
      DALI_LOG_ERROR("glLinkProgram[%s] failed:\n%s. Need to re-compile shader\n", mImpl->name.c_str(), output);
      return false;
    }
  }

  return result;
}

void ProgramImpl::SaveProgramBinary()
{
  GLint  binaryLength{0u};
  GLint  binarySize{0u};
  GLenum format;
  auto*  gl = mImpl->controller.GetGL();
  if(DALI_UNLIKELY(!gl))
  {
    DALI_LOG_ERROR("Can't Get GL \n");
    return;
  }

  gl->GetProgramiv(mImpl->glProgram, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
  if(binaryLength == 0)
  {
    DALI_LOG_ERROR("GL_PROGRAM_BINARY_LENGTH is zero. maybe this device doesn't support glProgramBinary \n");
    return;
  }

  std::vector<uint8_t> programBinary(binaryLength);
  gl->GetProgramBinary(mImpl->glProgram, binaryLength, &binarySize, &format, programBinary.data());
  if(binarySize != binaryLength)
  {
    DALI_LOG_ERROR("Program binary created but size mismatch %d != %d\n", binarySize, binaryLength);
    return;
  }

  auto programBinaryName     = GetSystemProgramBinaryPath() + GetProgramBinaryName();
  auto programBinaryNameTemp = programBinaryName + std::to_string(getpid()) + ".tmp";
  bool loaded                = SaveFile(programBinaryNameTemp, (unsigned char*)programBinary.data(), binaryLength);
  if(!loaded)
  {
    DALI_LOG_ERROR("Program binary save failed!! file = %s \n", programBinaryName.c_str());
    return;
  }

  if(std::filesystem::exists(programBinaryName))
  {
    std::filesystem::remove(programBinaryNameTemp);
  }
  else
  {
    std::filesystem::rename(programBinaryNameTemp, programBinaryName);
  }

  DALI_LOG_DEBUG_INFO("ProgramBinary is saved [success:%d] file = %s buffer size = %d \n", loaded, programBinaryName.c_str(), binaryLength);
}

bool ProgramImpl::SaveFile(const std::string& filename, const unsigned char* buffer, unsigned int numBytes)
{
  DALI_ASSERT_DEBUG(0 != filename.length());

  bool result = false;

  std::filebuf buf;
  buf.open(filename.c_str(), std::ios::out | std::ios_base::trunc | std::ios::binary);
  if(buf.is_open())
  {
    std::ostream stream(&buf);

    // determine size of buffer
    int length = static_cast<int>(numBytes);

    // write contents of buffer to the file
    stream.write(reinterpret_cast<const char*>(buffer), length);

    if(!stream.bad())
    {
      result = true;
    }
    else
    {
      DALI_LOG_ERROR("std::ostream.write failed!\n");
    }
  }
  else
  {
    DALI_LOG_ERROR("std::filebuf.open failed!\n");
  }

  if(!result)
  {
    const int errorMessageMaxLength               = 128;
    char      errorMessage[errorMessageMaxLength] = {}; // Initailze as null.

    // Return type of stderror_r is different between system type. We should not use return value.
    [[maybe_unused]] auto ret = strerror_r(errno, errorMessage, errorMessageMaxLength - 1);

    DALI_LOG_ERROR("Can't write to %s. buffer pointer : %p, length : %u, error message : [%s]\n", filename.c_str(), buffer, numBytes, errorMessage);
  }

  return result;
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
