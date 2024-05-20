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

// CLASS HEADER
#include <dali/internal/graphics/gles/gl-implementation.h>

// EXTERNAL INCLUDES
#include <limits>

// INTERNAL INCLUDES
#include <dali/internal/system/common/environment-variables.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
static constexpr int32_t INITIAL_GLES_VERSION                         = 30;
static constexpr int32_t GLES_VERSION_SUPPORT_BLEND_EQUATION_ADVANCED = 32;

static constexpr int32_t MINIMUM_GLES_VERSION_GET_MAXIMUM_MULTISAMPLES_TO_TEXTURE = 30;

static constexpr const char* LEGACY_SHADING_LANGUAGE_VERSION = "100";

static constexpr const char* DEFAULT_SAMPLER_TYPE = "sampler2D";

static constexpr const char* FRAGMENT_SHADER_ADVANCED_BLEND_EQUATION_PREFIX =
  "#ifdef GL_KHR_blend_equation_advanced\n"
  "#extension GL_KHR_blend_equation_advanced : enable\n"
  "#endif\n"

  "#if defined(GL_KHR_blend_equation_advanced) || __VERSION__>=320\n"
  "  layout(blend_support_all_equations) out;\n"
  "#endif\n";

static constexpr const char* FRAGMENT_SHADER_OUTPUT_COLOR_STRING =
  "out mediump vec4 fragColor;\n";

static constexpr const char* OES_EGL_IMAGE_EXTERNAL_STRING = "#extension GL_OES_EGL_image_external:require\n";

static constexpr const char* OES_EGL_IMAGE_EXTERNAL_STRING_ESSL3 = "#extension GL_OES_EGL_image_external_essl3:require\n";

static uint32_t GetPerformanceLogThresholdTime()
{
  auto     timeString = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_EGL_PERFORMANCE_LOG_THRESHOLD_TIME);
  uint32_t time       = timeString ? static_cast<uint32_t>(std::atoi(timeString)) : std::numeric_limits<uint32_t>::max();
  return time;
}
} // namespace

GlImplementation::GlImplementation()
: mGlExtensionSupportedCacheList(),
  mContextCreatedWaitCondition(),
  mMaxTextureSize(0),
  mMaxCombinedTextureUnits(0),
  mMaxTextureSamples(0),
  mVertexShaderPrefix(""),
  mGlesVersion(INITIAL_GLES_VERSION),
  mShadingLanguageVersion(100),
  mShadingLanguageVersionCached(false),
  mIsSurfacelessContextSupported(false),
  mIsContextCreated(false)
{
  mImpl.reset(new Gles3Implementation());
}

void GlImplementation::ContextCreated()
{
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTextureSize);
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &mMaxCombinedTextureUnits);

  // Since gles 2.0 didn't return well for GL_MAJOR_VERSION and GL_MINOR_VERSION,
  // Only change gles version for the device that support above gles 3.0.
  if(mGlesVersion >= INITIAL_GLES_VERSION)
  {
    GLint majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    mGlesVersion = majorVersion * 10 + minorVersion;
  }

  if(mGlesVersion >= GLES_VERSION_SUPPORT_BLEND_EQUATION_ADVANCED)
  {
    SetIsAdvancedBlendEquationSupported(true);
  }

  if(mGlExtensionSupportedCacheList.NeedFullCheck())
  {
    // fully check gl extensions if we miss some extension supported
    mGlExtensionSupportedCacheList.EnsureGlExtensionSupportedCheck();
  }

  if(IsMultisampledRenderToTextureSupported())
  {
    mMaxTextureSamples = 0;

    if(mGlesVersion >= MINIMUM_GLES_VERSION_GET_MAXIMUM_MULTISAMPLES_TO_TEXTURE)
    {
      // Try to get maximum FBO MSAA sampling level from GL_RENDERBUFFER first.
      // If false, than ask again to GL_MAX_SAMPLES_EXT.
      GetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, 1, &mMaxTextureSamples);
    }
    if(mMaxTextureSamples == 0)
    {
      glGetIntegerv(GL_MAX_SAMPLES_EXT, &mMaxTextureSamples);
    }
  }

  if(!mShadingLanguageVersionCached)
  {
    std::istringstream shadingLanguageVersionStream(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
    std::string        token;
    uint32_t           tokenCount = 0;
    while(std::getline(shadingLanguageVersionStream, token, ' '))
    {
      if(tokenCount == 3 && token == "ES")
      {
        std::getline(shadingLanguageVersionStream, token, '.');
        mShadingLanguageVersion = std::atoi(token.c_str());
        mShadingLanguageVersion *= 100;
        std::getline(shadingLanguageVersionStream, token, '.');
        mShadingLanguageVersion += std::atoi(token.c_str());
        break;
      }
      tokenCount++;
    }
  }

  mLogThreshold = GetPerformanceLogThresholdTime();
  mLogEnabled   = mLogThreshold < std::numeric_limits<uint32_t>::max() ? true : false;

  {
    ConditionalWait::ScopedLock lock(mContextCreatedWaitCondition);
    mIsContextCreated = true;
    mContextCreatedWaitCondition.Notify(lock);
  }
}

std::string GlImplementation::GetFragmentShaderPrefix()
{
  if(mFragmentShaderPrefix == "")
  {
    mFragmentShaderPrefix = GetShaderVersionPrefix();

    if(GetShadingLanguageVersion() < 300)
    {
      mFragmentShaderPrefix += "#define INPUT varying\n";
      mFragmentShaderPrefix += "#define OUT_COLOR gl_FragColor\n";
      mFragmentShaderPrefix += "#define TEXTURE texture2D\n";
    }
    else
    {
      mFragmentShaderPrefix += "#define INPUT in\n";
      mFragmentShaderPrefix += "#define OUT_COLOR fragColor\n";
      mFragmentShaderPrefix += "#define TEXTURE texture\n";

      if(IsAdvancedBlendEquationSupported())
      {
        mFragmentShaderPrefix += FRAGMENT_SHADER_ADVANCED_BLEND_EQUATION_PREFIX;
      }

      mFragmentShaderPrefix += FRAGMENT_SHADER_OUTPUT_COLOR_STRING;
    }
  }
  return mFragmentShaderPrefix;
}

bool GlImplementation::ApplyNativeFragmentShader(std::string& shader, const char* customSamplerType)
{
  bool        modified        = false;
  std::string versionString   = "#version";
  size_t      versionPosition = shader.find(versionString);
  if(versionPosition != std::string::npos)
  {
    std::string extensionString;
    size_t      shadingLanguageVersionPosition = shader.find_first_not_of(" \t", versionPosition + versionString.length());
    if(shadingLanguageVersionPosition != std::string::npos &&
       shader.substr(shadingLanguageVersionPosition, 3) == LEGACY_SHADING_LANGUAGE_VERSION)
    {
      extensionString = OES_EGL_IMAGE_EXTERNAL_STRING;
    }
    else
    {
      extensionString = OES_EGL_IMAGE_EXTERNAL_STRING_ESSL3;
    }

    if(shader.find(extensionString) == std::string::npos)
    {
      modified                 = true;
      size_t extensionPosition = shader.find_first_of("\n", versionPosition) + 1;
      shader.insert(extensionPosition, extensionString);
    }
  }
  else
  {
    if(shader.find(OES_EGL_IMAGE_EXTERNAL_STRING) == std::string::npos)
    {
      modified = true;
      shader   = OES_EGL_IMAGE_EXTERNAL_STRING + shader;
    }
  }

  if(shader.find(customSamplerType) == std::string::npos)
  {
    size_t pos = shader.find(DEFAULT_SAMPLER_TYPE);
    if(pos != std::string::npos)
    {
      modified = true;
      shader.replace(pos, strlen(DEFAULT_SAMPLER_TYPE), customSamplerType);
    }
  }

  return modified;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
