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

        // For glsl 3+ we will compile dummy shader to make sure this is
        // the highest supported version. Spec doesn't guarantee that
        // GL_SHADING_LANGUAGE_VERSION will return expected version.
        if(mShadingLanguageVersion > 300)
        {
          // try to compile simple shader to see if returned value is valid
          // for the platform.
          auto        vertexShader   = glCreateShader(GL_VERTEX_SHADER);
          auto        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
          std::string version        = "#version " + std::to_string(mShadingLanguageVersion) + " es\n";

          std::string vertexShaderCode   = version + "\nvoid main() { gl_Position.xyz = vec3(0,0,0); }\n";
          std::string fragmentShaderCode = version + "\nout highp vec4 fragColor;\nvoid main() { fragColor = vec4(0,0,0,1); }\n";

          const auto vshData = vertexShaderCode.data();
          auto       size    = GLint(vertexShaderCode.size());
          glShaderSource(vertexShader, 1, &vshData, &size);

          size               = GLint(fragmentShaderCode.size());
          const auto fshData = fragmentShaderCode.data();
          glShaderSource(fragmentShader, 1, &fshData, &size);

          glCompileShader(vertexShader);
          glCompileShader(fragmentShader);

          // check for errors
          GLint status{0};
          int   stage = 0; // 0 - vertex shader, 1 - fragment shader, for error reporting
          glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
          if(status == GL_TRUE)
          {
            stage = 1;
            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
          }
          if(status != GL_TRUE)
          {
            DALI_LOG_ERROR("Graphics backend can't compile shader with GLSL version %d\n", int(mShadingLanguageVersion));
            DALI_LOG_ERROR("Using GLSL 300 ES as a fallback\n");
            auto    shaderModule = stage == 0 ? vertexShader : fragmentShader;
            char    msg[4096];
            GLsizei msgSize;
            glGetShaderInfoLog(shaderModule, 4096, &msgSize, msg);
            msg[msgSize] = 0;
            DALI_LOG_ERROR(msg);
            mShadingLanguageVersion = 300; // fallback to the minimum GLES3 version;
          }
          glDeleteShader(vertexShader);
          glDeleteShader(fragmentShader);

          // clear GL error flags so it won't mess with further calls
          glGetError();
        }
        break;
      }
      tokenCount++;
    }
  }

  mTimeCheckerFilter = Dali::Integration::TimeChecker::ThresholdFilter::New(std::numeric_limits<uint32_t>::max(), DALI_ENV_EGL_PERFORMANCE_LOG_THRESHOLD_TIME);

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
      mFragmentShaderPrefix += "#define FLAT\n";
      mFragmentShaderPrefix += "#define OUT_COLOR gl_FragColor\n";
      mFragmentShaderPrefix += "#define TEXTURE texture2D\n";
    }
    else
    {
      mFragmentShaderPrefix += "#define INPUT in\n";
      mFragmentShaderPrefix += "#define FLAT flat\n";
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

bool GlImplementation::ApplyNativeFragmentShader(Dali::Integration::GlAbstraction& impl, std::string& shader, const char* customSamplerType, int count)
{
  // Identify if it's a modern shader. Modern shader will dynamically apply GLSL version
  bool        hasGLSLVersionString = true;
  std::string extensionString;
  bool        modified        = false;
  size_t      versionPosition = shader.find("@version");
  if(versionPosition != std::string::npos)
  {
    // we have a new shader, pull GLSL version
    auto glslVersion = impl.GetShaderLanguageVersion();
    if(glslVersion < 300) // old GLES2
    {
      extensionString = OES_EGL_IMAGE_EXTERNAL_STRING;
    }
    else
    {
      extensionString = OES_EGL_IMAGE_EXTERNAL_STRING_ESSL3;
    }
  }
  else
  {
    // If the shader isn't modern, we look for 'classic' #version and
    // based on that we assign extension requirement
    std::string versionString = "#version";
    versionPosition           = shader.find(versionString);
    if(versionPosition != std::string::npos)
    {
      size_t shadingLanguageVersionPosition = shader.find_first_not_of(" \t", versionPosition + versionString.length());
      if(shadingLanguageVersionPosition != std::string::npos &&
         shader.substr(shadingLanguageVersionPosition, 3) == LEGACY_SHADING_LANGUAGE_VERSION)
      {
        extensionString = OES_EGL_IMAGE_EXTERNAL_STRING;
      }
      else
      {
        extensionString = OES_EGL_IMAGE_EXTERNAL_STRING_ESSL3;
      }
    }
    else
    // If version isn't supplied we assume GLES2 shader (as this is the only
    // dialect allowing source without #version
    {
      extensionString      = OES_EGL_IMAGE_EXTERNAL_STRING;
      versionPosition      = 0;
      hasGLSLVersionString = false;
    }
  }

  // insert extension string if not there yet
  if(!extensionString.empty() && shader.find(extensionString) == std::string::npos)
  {
    modified                 = true;
    size_t extensionPosition = 0;
    if(hasGLSLVersionString)
    {
      extensionPosition = shader.find_first_of("\n", versionPosition) + 1;
    }
    shader.insert(extensionPosition, extensionString);
  }

  if(shader.find(customSamplerType) == std::string::npos)
  {
    size_t pos           = 0;
    int    replacedCount = 0;

    while((pos = shader.find(DEFAULT_SAMPLER_TYPE, pos)) != std::string::npos)
    {
      if(count >= 0 && replacedCount >= count)
      {
        break;
      }
      shader.replace(pos, strlen(DEFAULT_SAMPLER_TYPE), customSamplerType);
      pos += strlen(customSamplerType);
      replacedCount++;
      modified = true;
    }
  }

  return modified;
}

bool GlImplementation::ApplyNativeFragmentShader(std::string& shader, const char* customSamplerType, int count)
{
  return ApplyNativeFragmentShader(*this, shader, customSamplerType, count);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
