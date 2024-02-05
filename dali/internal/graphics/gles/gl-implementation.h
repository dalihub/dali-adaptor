#ifndef DALI_INTERNAL_GL_IMPLEMENTATION_H
#define DALI_INTERNAL_GL_IMPLEMENTATION_H

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

// EXTERNAL INCLUDES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/internal/graphics/common/egl-include.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <cstdlib>
#include <cstring>
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/graphics/gles/gl-extensions-support.h>
#include <dali/internal/graphics/gles/gles-abstraction.h>
#include <dali/internal/graphics/gles/gles2-implementation.h>
#include <dali/internal/graphics/gles/gles3-implementation.h>
#include <dali/internal/system/common/time-service.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#ifndef START_DURATION_CHECK
#define START_DURATION_CHECK()                         \
  uint64_t startTimeNanoSeconds = 0ull;                \
  uint64_t endTimeNanoSeconds   = 0ull;                \
  if(mLogEnabled)                                      \
  {                                                    \
    TimeService::GetNanoseconds(startTimeNanoSeconds); \
  }
#endif

#ifndef FINISH_DURATION_CHECK
#define FINISH_DURATION_CHECK(functionName)                                                                                                                \
  if(mLogEnabled)                                                                                                                                          \
  {                                                                                                                                                        \
    TimeService::GetNanoseconds(endTimeNanoSeconds);                                                                                                       \
    if(static_cast<uint32_t>((endTimeNanoSeconds - startTimeNanoSeconds) / 1000000ull) >= mLogThreshold)                                                   \
    {                                                                                                                                                      \
      DALI_LOG_RELEASE_INFO("%s takes long time! [%.6lf ms]\n", functionName, static_cast<double>(endTimeNanoSeconds - startTimeNanoSeconds) / 1000000.0); \
    }                                                                                                                                                      \
  }
#endif

#ifndef FINISH_DURATION_CHECK_WITH_FORMAT
#define FINISH_DURATION_CHECK_WITH_FORMAT(functionName, format, ...)                                                                                                                 \
  if(mLogEnabled)                                                                                                                                                                    \
  {                                                                                                                                                                                  \
    TimeService::GetNanoseconds(endTimeNanoSeconds);                                                                                                                                 \
    if(static_cast<uint32_t>((endTimeNanoSeconds - startTimeNanoSeconds) / 1000000ull) >= mLogThreshold)                                                                             \
    {                                                                                                                                                                                \
      DALI_LOG_RELEASE_INFO("%s takes long time! [%.6lf ms] " format "\n", functionName, static_cast<double>(endTimeNanoSeconds - startTimeNanoSeconds) / 1000000.0, ##__VA_ARGS__); \
    }                                                                                                                                                                                \
  }
#endif
} // namespace
/**
 * GlImplementation is a concrete implementation for GlAbstraction.
 * The class provides an OpenGL-ES 2.0 or 3.0 implementation.
 * The class is provided when creating the Integration::Core object.
 */
class GlImplementation : public Dali::Integration::GlAbstraction
{
public:
  GlImplementation();

  virtual ~GlImplementation()
  {
  }

  void PreRender() override
  {
    /* Do nothing in main implementation */
  }

  void PostRender() override
  {
    /* Do nothing in main implementation */
  }

  void ContextCreated();

  void SetGlesVersion(const int32_t glesVersion)
  {
    if(mGlesVersion / 10 != glesVersion / 10)
    {
      mGlesVersion = glesVersion;
      if(mGlesVersion >= 30)
      {
        mImpl.reset(new Gles3Implementation());
      }
      else
      {
        mImpl.reset(new Gles2Implementation());
      }
    }
  }

  void SetIsSurfacelessContextSupported(const bool isSupported)
  {
    mIsSurfacelessContextSupported = isSupported;
  }

  bool IsSurfacelessContextSupported() const override
  {
    return mIsSurfacelessContextSupported;
  }

  void SetIsAdvancedBlendEquationSupported(const bool isSupported)
  {
    mGlExtensionSupportedCacheList.MarkSupported(GlExtensionCache::GlExtensionCheckerType::BLEND_EQUATION_ADVANCED, isSupported);
  }

  bool IsAdvancedBlendEquationSupported() override
  {
    ConditionalWait::ScopedLock lock(mContextCreatedWaitCondition);

    const auto type = GlExtensionCache::GlExtensionCheckerType::BLEND_EQUATION_ADVANCED;
    if(!mIsContextCreated && !mGlExtensionSupportedCacheList.IsCached(type))
    {
      mContextCreatedWaitCondition.Wait(lock);
    }
    return mGlExtensionSupportedCacheList.IsSupported(type);
  }

  void SetIsMultisampledRenderToTextureSupported(const bool isSupported)
  {
    mGlExtensionSupportedCacheList.MarkSupported(GlExtensionCache::GlExtensionCheckerType::MULTISAMPLED_RENDER_TO_TEXTURE, isSupported);
  }

  bool IsMultisampledRenderToTextureSupported() override
  {
    ConditionalWait::ScopedLock lock(mContextCreatedWaitCondition);

    const auto type = GlExtensionCache::GlExtensionCheckerType::MULTISAMPLED_RENDER_TO_TEXTURE;
    if(!mIsContextCreated && !mGlExtensionSupportedCacheList.IsCached(type))
    {
      mContextCreatedWaitCondition.Wait(lock);
    }
    return mGlExtensionSupportedCacheList.IsSupported(type);
  }

  bool IsBlendEquationSupported(DevelBlendEquation::Type blendEquation) override
  {
    switch(blendEquation)
    {
      case DevelBlendEquation::ADD:
      case DevelBlendEquation::SUBTRACT:
      case DevelBlendEquation::REVERSE_SUBTRACT:
      {
        return true;
      }
      case DevelBlendEquation::MIN:
      case DevelBlendEquation::MAX:
      {
        return (GetGlesVersion() >= 30);
      }
      case DevelBlendEquation::MULTIPLY:
      case DevelBlendEquation::SCREEN:
      case DevelBlendEquation::OVERLAY:
      case DevelBlendEquation::DARKEN:
      case DevelBlendEquation::LIGHTEN:
      case DevelBlendEquation::COLOR_DODGE:
      case DevelBlendEquation::COLOR_BURN:
      case DevelBlendEquation::HARD_LIGHT:
      case DevelBlendEquation::SOFT_LIGHT:
      case DevelBlendEquation::DIFFERENCE:
      case DevelBlendEquation::EXCLUSION:
      case DevelBlendEquation::HUE:
      case DevelBlendEquation::SATURATION:
      case DevelBlendEquation::COLOR:
      case DevelBlendEquation::LUMINOSITY:
      {
        return IsAdvancedBlendEquationSupported();
      }

      default:
      {
        return false;
      }
    }

    return false;
  }

  uint32_t GetShaderLanguageVersion() override
  {
    return static_cast<uint32_t>(GetShadingLanguageVersion());
  }

  std::string GetShaderVersionPrefix() override
  {
    if(mShaderVersionPrefix == "")
    {
      mShaderVersionPrefix = "#version " + std::to_string(GetShadingLanguageVersion());
      if(GetShadingLanguageVersion() < 300)
      {
        mShaderVersionPrefix += "\n";
      }
      else
      {
        mShaderVersionPrefix += " es\n";
      }
    }
    return mShaderVersionPrefix;
  }

  std::string GetVertexShaderPrefix() override
  {
    if(mVertexShaderPrefix == "")
    {
      mVertexShaderPrefix = GetShaderVersionPrefix();

      if(GetShadingLanguageVersion() < 300)
      {
        mVertexShaderPrefix += "#define INPUT attribute\n";
        mVertexShaderPrefix += "#define OUTPUT varying\n";
      }
      else
      {
        mVertexShaderPrefix += "#define INPUT in\n";
        mVertexShaderPrefix += "#define OUTPUT out\n";
      }
    }
    return mVertexShaderPrefix;
  }

  std::string GetFragmentShaderPrefix() override;

  bool TextureRequiresConverting(const GLenum imageGlFormat, const GLenum textureGlFormat, const bool isSubImage) const override
  {
    bool convert = ((imageGlFormat == GL_RGB) && (textureGlFormat == GL_RGBA));
    if(mGlesVersion >= 30)
    {
      // Don't convert manually from RGB to RGBA if GLES >= 3.0 and a sub-image is uploaded.
      convert = (convert && !isSubImage);
    }
    return convert;
  }

  int GetMaxTextureSize()
  {
    ConditionalWait::ScopedLock lock(mContextCreatedWaitCondition);
    if(!mIsContextCreated)
    {
      mContextCreatedWaitCondition.Wait(lock);
    }
    return mMaxTextureSize;
  }

  int GetMaxCombinedTextureUnits()
  {
    ConditionalWait::ScopedLock lock(mContextCreatedWaitCondition);
    if(!mIsContextCreated)
    {
      mContextCreatedWaitCondition.Wait(lock);
    }
    return mMaxCombinedTextureUnits;
  }

  int GetMaxTextureSamples()
  {
    ConditionalWait::ScopedLock lock(mContextCreatedWaitCondition);
    if(!mIsContextCreated)
    {
      mContextCreatedWaitCondition.Wait(lock);
    }
    return mMaxTextureSamples;
  }

  int32_t GetGlesVersion()
  {
    ConditionalWait::ScopedLock lock(mContextCreatedWaitCondition);
    if(!mIsContextCreated)
    {
      mContextCreatedWaitCondition.Wait(lock);
    }
    return mGlesVersion;
  }

  void SetShadingLanguageVersion(int shadingLanguageVersion)
  {
    mShadingLanguageVersion       = shadingLanguageVersion;
    mShadingLanguageVersionCached = true;
  }

  int GetShadingLanguageVersion()
  {
    ConditionalWait::ScopedLock lock(mContextCreatedWaitCondition);
    if(!mIsContextCreated && !mShadingLanguageVersionCached)
    {
      mContextCreatedWaitCondition.Wait(lock);
    }
    return mShadingLanguageVersion;
  }

  bool ApplyNativeFragmentShader(std::string& shader, const char* customSamplerType);

  /* OpenGL ES 2.0 */

  void ActiveTexture(GLenum texture) override
  {
    glActiveTexture(texture);
  }

  void AttachShader(GLuint program, GLuint shader) override
  {
    glAttachShader(program, shader);
  }

  void BindAttribLocation(GLuint program, GLuint index, const char* name) override
  {
    glBindAttribLocation(program, index, name);
  }

  void BindBuffer(GLenum target, GLuint buffer) override
  {
    glBindBuffer(target, buffer);
  }

  void BindFramebuffer(GLenum target, GLuint framebuffer) override
  {
    glBindFramebuffer(target, framebuffer);
  }

  void BindRenderbuffer(GLenum target, GLuint renderbuffer) override
  {
    glBindRenderbuffer(target, renderbuffer);
  }

  void BindTexture(GLenum target, GLuint texture) override
  {
    glBindTexture(target, texture);
  }

  void BlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) override
  {
    glBlendColor(red, green, blue, alpha);
  }

  void BlendEquation(GLenum mode) override
  {
    glBlendEquation(mode);
  }

  void BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) override
  {
    glBlendEquationSeparate(modeRGB, modeAlpha);
  }

  void BlendFunc(GLenum sfactor, GLenum dfactor) override
  {
    glBlendFunc(sfactor, dfactor);
  }

  void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) override
  {
    glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
  }

  void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) override
  {
    glBufferData(target, size, data, usage);
  }

  void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) override
  {
    glBufferSubData(target, offset, size, data);
  }

  GLenum CheckFramebufferStatus(GLenum target) override
  {
    return glCheckFramebufferStatus(target);
  }

  void Clear(GLbitfield mask) override
  {
    START_DURATION_CHECK();

    glClear(mask);

    FINISH_DURATION_CHECK("glClear");
  }

  void ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) override
  {
    glClearColor(red, green, blue, alpha);
  }

  void ClearDepthf(GLclampf depth) override
  {
    glClearDepthf(depth);
  }

  void ClearStencil(GLint s) override
  {
    glClearStencil(s);
  }

  void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) override
  {
    glColorMask(red, green, blue, alpha);
  }

  void CompileShader(GLuint shader) override
  {
    START_DURATION_CHECK();

    glCompileShader(shader);

    FINISH_DURATION_CHECK_WITH_FORMAT("glCompileShader", "shader id : %u", shader);
  }

  void CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data) override
  {
    START_DURATION_CHECK();

    glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);

    FINISH_DURATION_CHECK_WITH_FORMAT("glCompressedTexImage2D", "size : %u x %u", width, height);
  }

  void CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) override
  {
    START_DURATION_CHECK();

    glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);

    FINISH_DURATION_CHECK_WITH_FORMAT("glCompressedTexSubImage2D", "size : %u x %u", width, height);
  }

  void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) override
  {
    glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
  }

  void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) override
  {
    glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
  }

  GLuint CreateProgram(void) override
  {
    return glCreateProgram();
  }

  GLuint CreateShader(GLenum type) override
  {
    return glCreateShader(type);
  }

  void CullFace(GLenum mode) override
  {
    glCullFace(mode);
  }

  void DeleteBuffers(GLsizei n, const GLuint* buffers) override
  {
    glDeleteBuffers(n, buffers);
  }

  void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers) override
  {
    glDeleteFramebuffers(n, framebuffers);
  }

  void DeleteProgram(GLuint program) override
  {
    glDeleteProgram(program);
  }

  void DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) override
  {
    glDeleteRenderbuffers(n, renderbuffers);
  }

  void DeleteShader(GLuint shader) override
  {
    glDeleteShader(shader);
  }

  void DeleteTextures(GLsizei n, const GLuint* textures) override
  {
    glDeleteTextures(n, textures);
  }

  void DepthFunc(GLenum func) override
  {
    glDepthFunc(func);
  }

  void DepthMask(GLboolean flag) override
  {
    glDepthMask(flag);
  }

  void DepthRangef(GLclampf zNear, GLclampf zFar) override
  {
    glDepthRangef(zNear, zFar);
  }

  void DetachShader(GLuint program, GLuint shader) override
  {
    glDetachShader(program, shader);
  }

  void Disable(GLenum cap) override
  {
    glDisable(cap);
  }

  void DisableVertexAttribArray(GLuint index) override
  {
    glDisableVertexAttribArray(index);
  }

  void DrawArrays(GLenum mode, GLint first, GLsizei count) override
  {
    glDrawArrays(mode, first, count);
  }

  void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) override
  {
    glDrawElements(mode, count, type, indices);
  }

  void Enable(GLenum cap) override
  {
    glEnable(cap);
  }

  void EnableVertexAttribArray(GLuint index) override
  {
    glEnableVertexAttribArray(index);
  }

  void Finish(void) override
  {
    START_DURATION_CHECK();

    glFinish();

    FINISH_DURATION_CHECK("glFinish");
  }

  void Flush(void) override
  {
    START_DURATION_CHECK();

    glFlush();

    FINISH_DURATION_CHECK("glFlush");
  }

  void FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) override
  {
    glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
  }

  void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) override
  {
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
  }

  void FrontFace(GLenum mode) override
  {
    glFrontFace(mode);
  }

  void GenBuffers(GLsizei n, GLuint* buffers) override
  {
    glGenBuffers(n, buffers);
  }

  void GenerateMipmap(GLenum target) override
  {
    glGenerateMipmap(target);
  }

  void GenFramebuffers(GLsizei n, GLuint* framebuffers) override
  {
    glGenFramebuffers(n, framebuffers);
  }

  void GenRenderbuffers(GLsizei n, GLuint* renderbuffers) override
  {
    glGenRenderbuffers(n, renderbuffers);
  }

  void GenTextures(GLsizei n, GLuint* textures) override
  {
    glGenTextures(n, textures);
  }

  void GetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name) override
  {
    glGetActiveAttrib(program, index, bufsize, length, size, type, name);
  }

  void GetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name) override
  {
    glGetActiveUniform(program, index, bufsize, length, size, type, name);
  }

  void GetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders) override
  {
    glGetAttachedShaders(program, maxcount, count, shaders);
  }

  int GetAttribLocation(GLuint program, const char* name) override
  {
    return glGetAttribLocation(program, name);
  }

  void GetBooleanv(GLenum pname, GLboolean* params) override
  {
    glGetBooleanv(pname, params);
  }

  void GetBufferParameteriv(GLenum target, GLenum pname, GLint* params) override
  {
    glGetBufferParameteriv(target, pname, params);
  }

  GLenum GetError(void) override
  {
    return glGetError();
  }

  void GetFloatv(GLenum pname, GLfloat* params) override
  {
    glGetFloatv(pname, params);
  }

  void GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params) override
  {
    glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
  }

  void GetIntegerv(GLenum pname, GLint* params) override
  {
    glGetIntegerv(pname, params);
  }

  void GetProgramiv(GLuint program, GLenum pname, GLint* params) override
  {
    glGetProgramiv(program, pname, params);
  }

  void GetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog) override
  {
    glGetProgramInfoLog(program, bufsize, length, infolog);
  }

  void GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params) override
  {
    glGetRenderbufferParameteriv(target, pname, params);
  }

  void GetShaderiv(GLuint shader, GLenum pname, GLint* params) override
  {
    glGetShaderiv(shader, pname, params);
  }

  void GetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog) override
  {
    glGetShaderInfoLog(shader, bufsize, length, infolog);
  }

  void GetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) override
  {
    glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
  }

  void GetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, char* source) override
  {
    glGetShaderSource(shader, bufsize, length, source);
  }

  const GLubyte* GetString(GLenum name) override
  {
    return glGetString(name);
  }

  void GetTexParameterfv(GLenum target, GLenum pname, GLfloat* params) override
  {
    glGetTexParameterfv(target, pname, params);
  }

  void GetTexParameteriv(GLenum target, GLenum pname, GLint* params) override
  {
    glGetTexParameteriv(target, pname, params);
  }

  void GetUniformfv(GLuint program, GLint location, GLfloat* params) override
  {
    glGetUniformfv(program, location, params);
  }

  void GetUniformiv(GLuint program, GLint location, GLint* params) override
  {
    glGetUniformiv(program, location, params);
  }

  int GetUniformLocation(GLuint program, const char* name) override
  {
    return glGetUniformLocation(program, name);
  }

  void GetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params) override
  {
    glGetVertexAttribfv(index, pname, params);
  }

  void GetVertexAttribiv(GLuint index, GLenum pname, GLint* params) override
  {
    glGetVertexAttribiv(index, pname, params);
  }

  void GetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer) override
  {
    glGetVertexAttribPointerv(index, pname, pointer);
  }

  void Hint(GLenum target, GLenum mode) override
  {
    glHint(target, mode);
  }

  GLboolean IsBuffer(GLuint buffer) override
  {
    return glIsBuffer(buffer);
  }

  GLboolean IsEnabled(GLenum cap) override
  {
    return glIsEnabled(cap);
  }

  GLboolean IsFramebuffer(GLuint framebuffer) override
  {
    return glIsFramebuffer(framebuffer);
  }

  GLboolean IsProgram(GLuint program) override
  {
    return glIsProgram(program);
  }

  GLboolean IsRenderbuffer(GLuint renderbuffer) override
  {
    return glIsRenderbuffer(renderbuffer);
  }

  GLboolean IsShader(GLuint shader) override
  {
    return glIsShader(shader);
  }

  GLboolean IsTexture(GLuint texture) override
  {
    return glIsTexture(texture);
  }

  void LineWidth(GLfloat width) override
  {
    glLineWidth(width);
  }

  void LinkProgram(GLuint program) override
  {
    START_DURATION_CHECK();

    glLinkProgram(program);

    FINISH_DURATION_CHECK_WITH_FORMAT("glLinkProgram", "program id : %u", program);
  }

  void PixelStorei(GLenum pname, GLint param) override
  {
    glPixelStorei(pname, param);
  }

  void PolygonOffset(GLfloat factor, GLfloat units) override
  {
    glPolygonOffset(factor, units);
  }

  void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) override
  {
    glReadPixels(x, y, width, height, format, type, pixels);
  }

  void ReleaseShaderCompiler(void) override
  {
    glReleaseShaderCompiler();
  }

  void RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) override
  {
    glRenderbufferStorage(target, internalformat, width, height);
  }

  void SampleCoverage(GLclampf value, GLboolean invert) override
  {
    glSampleCoverage(value, invert);
  }

  void Scissor(GLint x, GLint y, GLsizei width, GLsizei height) override
  {
    glScissor(x, y, width, height);
  }

  void ShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length) override
  {
    glShaderBinary(n, shaders, binaryformat, binary, length);
  }

  void ShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length) override
  {
    glShaderSource(shader, count, string, length);
  }

  void StencilFunc(GLenum func, GLint ref, GLuint mask) override
  {
    glStencilFunc(func, ref, mask);
  }

  void StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) override
  {
    glStencilFuncSeparate(face, func, ref, mask);
  }

  void StencilMask(GLuint mask) override
  {
    glStencilMask(mask);
  }

  void StencilMaskSeparate(GLenum face, GLuint mask) override
  {
    glStencilMaskSeparate(face, mask);
  }

  void StencilOp(GLenum fail, GLenum zfail, GLenum zpass) override
  {
    glStencilOp(fail, zfail, zpass);
  }

  void StencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass) override
  {
    glStencilOpSeparate(face, fail, zfail, zpass);
  }

  void TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) override
  {
    START_DURATION_CHECK();

    glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);

    FINISH_DURATION_CHECK_WITH_FORMAT("glTexImage2D", "size : %u x %u, format : %d, type : %d", width, height, static_cast<int>(format), static_cast<int>(type));
  }

  void TexParameterf(GLenum target, GLenum pname, GLfloat param) override
  {
    glTexParameterf(target, pname, param);
  }

  void TexParameterfv(GLenum target, GLenum pname, const GLfloat* params) override
  {
    glTexParameterfv(target, pname, params);
  }

  void TexParameteri(GLenum target, GLenum pname, GLint param) override
  {
    glTexParameteri(target, pname, param);
  }

  void TexParameteriv(GLenum target, GLenum pname, const GLint* params) override
  {
    glTexParameteriv(target, pname, params);
  }

  void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) override
  {
    START_DURATION_CHECK();

    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);

    FINISH_DURATION_CHECK_WITH_FORMAT("glTexSubImage2D", "size : %u x %u, format : %d, type : %d", width, height, static_cast<int>(format), static_cast<int>(type));
  }

  void Uniform1f(GLint location, GLfloat x) override
  {
    glUniform1f(location, x);
  }

  void Uniform1fv(GLint location, GLsizei count, const GLfloat* v) override
  {
    glUniform1fv(location, count, v);
  }

  void Uniform1i(GLint location, GLint x) override
  {
    glUniform1i(location, x);
  }

  void Uniform1iv(GLint location, GLsizei count, const GLint* v) override
  {
    glUniform1iv(location, count, v);
  }

  void Uniform2f(GLint location, GLfloat x, GLfloat y) override
  {
    glUniform2f(location, x, y);
  }

  void Uniform2fv(GLint location, GLsizei count, const GLfloat* v) override
  {
    glUniform2fv(location, count, v);
  }

  void Uniform2i(GLint location, GLint x, GLint y) override
  {
    glUniform2i(location, x, y);
  }

  void Uniform2iv(GLint location, GLsizei count, const GLint* v) override
  {
    glUniform2iv(location, count, v);
  }

  void Uniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z) override
  {
    glUniform3f(location, x, y, z);
  }

  void Uniform3fv(GLint location, GLsizei count, const GLfloat* v) override
  {
    glUniform3fv(location, count, v);
  }

  void Uniform3i(GLint location, GLint x, GLint y, GLint z) override
  {
    glUniform3i(location, x, y, z);
  }

  void Uniform3iv(GLint location, GLsizei count, const GLint* v) override
  {
    glUniform3iv(location, count, v);
  }

  void Uniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) override
  {
    glUniform4f(location, x, y, z, w);
  }

  void Uniform4fv(GLint location, GLsizei count, const GLfloat* v) override
  {
    glUniform4fv(location, count, v);
  }

  void Uniform4i(GLint location, GLint x, GLint y, GLint z, GLint w) override
  {
    glUniform4i(location, x, y, z, w);
  }

  void Uniform4iv(GLint location, GLsizei count, const GLint* v) override
  {
    glUniform4iv(location, count, v);
  }

  void UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
  {
    glUniformMatrix2fv(location, count, transpose, value);
  }

  void UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
  {
    glUniformMatrix3fv(location, count, transpose, value);
  }

  void UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
  {
    glUniformMatrix4fv(location, count, transpose, value);
  }

  void UseProgram(GLuint program) override
  {
    glUseProgram(program);
  }

  void ValidateProgram(GLuint program) override
  {
    glValidateProgram(program);
  }

  void VertexAttrib1f(GLuint indx, GLfloat x) override
  {
    glVertexAttrib1f(indx, x);
  }

  void VertexAttrib1fv(GLuint indx, const GLfloat* values) override
  {
    glVertexAttrib1fv(indx, values);
  }

  void VertexAttrib2f(GLuint indx, GLfloat x, GLfloat y) override
  {
    glVertexAttrib2f(indx, x, y);
  }

  void VertexAttrib2fv(GLuint indx, const GLfloat* values) override
  {
    glVertexAttrib2fv(indx, values);
  }

  void VertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z) override
  {
    glVertexAttrib3f(indx, x, y, z);
  }

  void VertexAttrib3fv(GLuint indx, const GLfloat* values) override
  {
    glVertexAttrib3fv(indx, values);
  }

  void VertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w) override
  {
    glVertexAttrib4f(indx, x, y, z, w);
  }

  void VertexAttrib4fv(GLuint indx, const GLfloat* values) override
  {
    glVertexAttrib4fv(indx, values);
  }

  void VertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr) override
  {
    glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
  }

  void Viewport(GLint x, GLint y, GLsizei width, GLsizei height) override
  {
    glViewport(x, y, width, height);
  }

  /* OpenGL ES 3.0 */

  void ReadBuffer(GLenum mode) override
  {
    mImpl->ReadBuffer(mode);
  }

  void DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices) override
  {
    mImpl->DrawRangeElements(mode, start, end, count, type, indices);
  }

  void TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels) override
  {
    mImpl->TexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
  }

  void TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels) override
  {
    mImpl->TexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
  }

  void CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) override
  {
    mImpl->CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
  }

  void CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data) override
  {
    mImpl->CompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
  }

  void CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data) override
  {
    mImpl->CompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
  }

  void GenQueries(GLsizei n, GLuint* ids) override
  {
    mImpl->GenQueries(n, ids);
  }

  void DeleteQueries(GLsizei n, const GLuint* ids) override
  {
    mImpl->DeleteQueries(n, ids);
  }

  GLboolean IsQuery(GLuint id) override
  {
    return mImpl->IsQuery(id);
  }

  void BeginQuery(GLenum target, GLuint id) override
  {
    mImpl->BeginQuery(target, id);
  }

  void EndQuery(GLenum target) override
  {
    mImpl->EndQuery(target);
  }

  void GetQueryiv(GLenum target, GLenum pname, GLint* params) override
  {
    mImpl->GetQueryiv(target, pname, params);
  }

  void GetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params) override
  {
    mImpl->GetQueryObjectuiv(id, pname, params);
  }

  GLboolean UnmapBuffer(GLenum target) override
  {
    return mImpl->UnmapBuffer(target);
  }

  void GetBufferPointerv(GLenum target, GLenum pname, GLvoid** params) override
  {
    mImpl->GetBufferPointerv(target, pname, params);
  }

  void DrawBuffers(GLsizei n, const GLenum* bufs) override
  {
    mImpl->DrawBuffers(n, bufs);
  }

  void UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
  {
    mImpl->UniformMatrix2x3fv(location, count, transpose, value);
  }

  void UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
  {
    mImpl->UniformMatrix3x2fv(location, count, transpose, value);
  }

  void UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
  {
    mImpl->UniformMatrix2x4fv(location, count, transpose, value);
  }

  void UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
  {
    mImpl->UniformMatrix4x2fv(location, count, transpose, value);
  }

  void UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
  {
    mImpl->UniformMatrix3x4fv(location, count, transpose, value);
  }

  void UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override
  {
    mImpl->UniformMatrix4x3fv(location, count, transpose, value);
  }

  void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) override
  {
    mImpl->BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
  }

  void RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) override
  {
    mImpl->RenderbufferStorageMultisample(target, samples, internalformat, width, height);
  }

  void FramebufferTexture2DMultisample(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples) override
  {
    mImpl->FramebufferTexture2DMultisample(target, attachment, textarget, texture, level, samples);
  }

  void FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) override
  {
    mImpl->FramebufferTextureLayer(target, attachment, texture, level, layer);
  }

  GLvoid* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) override
  {
    return mImpl->MapBufferRange(target, offset, length, access);
  }

  void FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) override
  {
    mImpl->FlushMappedBufferRange(target, offset, length);
  }

  void BindVertexArray(GLuint array) override
  {
    mImpl->BindVertexArray(array);
  }

  void DeleteVertexArrays(GLsizei n, const GLuint* arrays) override
  {
    mImpl->DeleteVertexArrays(n, arrays);
  }

  void GenVertexArrays(GLsizei n, GLuint* arrays) override
  {
    mImpl->GenVertexArrays(n, arrays);
  }

  GLboolean IsVertexArray(GLuint array) override
  {
    return mImpl->IsVertexArray(array);
  }

  void GetIntegeri_v(GLenum target, GLuint index, GLint* data) override
  {
    mImpl->GetIntegeri_v(target, index, data);
  }

  void BeginTransformFeedback(GLenum primitiveMode) override
  {
    mImpl->BeginTransformFeedback(primitiveMode);
  }

  void EndTransformFeedback(void) override
  {
    mImpl->EndTransformFeedback();
  }

  void BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) override
  {
    mImpl->BindBufferRange(target, index, buffer, offset, size);
  }

  void BindBufferBase(GLenum target, GLuint index, GLuint buffer) override
  {
    mImpl->BindBufferBase(target, index, buffer);
  }

  void TransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode) override
  {
    mImpl->TransformFeedbackVaryings(program, count, varyings, bufferMode);
  }

  void GetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) override
  {
    mImpl->GetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
  }

  void VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) override
  {
    mImpl->VertexAttribIPointer(index, size, type, stride, pointer);
  }

  void GetVertexAttribIiv(GLuint index, GLenum pname, GLint* params) override
  {
    mImpl->GetVertexAttribIiv(index, pname, params);
  }

  void GetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params) override
  {
    mImpl->GetVertexAttribIuiv(index, pname, params);
  }

  void VertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w) override
  {
    mImpl->VertexAttribI4i(index, x, y, z, w);
  }

  void VertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) override
  {
    mImpl->VertexAttribI4ui(index, x, y, z, w);
  }

  void VertexAttribI4iv(GLuint index, const GLint* v) override
  {
    mImpl->VertexAttribI4iv(index, v);
  }

  void VertexAttribI4uiv(GLuint index, const GLuint* v) override
  {
    mImpl->VertexAttribI4uiv(index, v);
  }

  void GetUniformuiv(GLuint program, GLint location, GLuint* params) override
  {
    mImpl->GetUniformuiv(program, location, params);
  }

  GLint GetFragDataLocation(GLuint program, const GLchar* name) override
  {
    return mImpl->GetFragDataLocation(program, name);
  }

  void Uniform1ui(GLint location, GLuint v0) override
  {
    mImpl->Uniform1ui(location, v0);
  }

  void Uniform2ui(GLint location, GLuint v0, GLuint v1) override
  {
    mImpl->Uniform2ui(location, v0, v1);
  }

  void Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2) override
  {
    mImpl->Uniform3ui(location, v0, v1, v2);
  }

  void Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) override
  {
    mImpl->Uniform4ui(location, v0, v1, v2, v3);
  }

  void Uniform1uiv(GLint location, GLsizei count, const GLuint* value) override
  {
    mImpl->Uniform1uiv(location, count, value);
  }

  void Uniform2uiv(GLint location, GLsizei count, const GLuint* value) override
  {
    mImpl->Uniform2uiv(location, count, value);
  }

  void Uniform3uiv(GLint location, GLsizei count, const GLuint* value) override
  {
    mImpl->Uniform3uiv(location, count, value);
  }

  void Uniform4uiv(GLint location, GLsizei count, const GLuint* value) override
  {
    mImpl->Uniform4uiv(location, count, value);
  }

  void ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value) override
  {
    mImpl->ClearBufferiv(buffer, drawbuffer, value);
  }

  void ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value) override
  {
    mImpl->ClearBufferuiv(buffer, drawbuffer, value);
  }

  void ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value) override
  {
    mImpl->ClearBufferfv(buffer, drawbuffer, value);
  }

  void ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) override
  {
    mImpl->ClearBufferfi(buffer, drawbuffer, depth, stencil);
  }

  const GLubyte* GetStringi(GLenum name, GLuint index) override
  {
    return mImpl->GetStringi(name, index);
  }

  void CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) override
  {
    mImpl->CopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
  }

  void GetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices) override
  {
    mImpl->GetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
  }

  void GetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params) override
  {
    mImpl->GetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
  }

  GLuint GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName) override
  {
    return mImpl->GetUniformBlockIndex(program, uniformBlockName);
  }

  void GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params) override
  {
    mImpl->GetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
  }

  void GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName) override
  {
    mImpl->GetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
  }

  void UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) override
  {
    mImpl->UniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
  }

  void DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount) override
  {
    mImpl->DrawArraysInstanced(mode, first, count, instanceCount);
  }

  void DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount) override
  {
    mImpl->DrawElementsInstanced(mode, count, type, indices, instanceCount);
  }

  GLsync FenceSync(GLenum condition, GLbitfield flags) override
  {
    return mImpl->FenceSync(condition, flags);
  }

  GLboolean IsSync(GLsync sync) override
  {
    return mImpl->IsSync(sync);
  }

  void DeleteSync(GLsync sync) override
  {
    mImpl->DeleteSync(sync);
  }

  GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) override
  {
    return mImpl->ClientWaitSync(sync, flags, timeout);
  }

  void WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) override
  {
    mImpl->WaitSync(sync, flags, timeout);
  }

  void GetInteger64v(GLenum pname, GLint64* params) override
  {
    mImpl->GetInteger64v(pname, params);
  }

  void GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) override
  {
    mImpl->GetSynciv(sync, pname, bufSize, length, values);
  }

  void GetInteger64i_v(GLenum target, GLuint index, GLint64* data) override
  {
    mImpl->GetInteger64i_v(target, index, data);
  }

  void GetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params) override
  {
    mImpl->GetBufferParameteri64v(target, pname, params);
  }

  void GenSamplers(GLsizei count, GLuint* samplers) override
  {
    mImpl->GenSamplers(count, samplers);
  }

  void DeleteSamplers(GLsizei count, const GLuint* samplers) override
  {
    mImpl->DeleteSamplers(count, samplers);
  }

  GLboolean IsSampler(GLuint sampler) override
  {
    return mImpl->IsSampler(sampler);
  }

  void BindSampler(GLuint unit, GLuint sampler) override
  {
    mImpl->BindSampler(unit, sampler);
  }

  void SamplerParameteri(GLuint sampler, GLenum pname, GLint param) override
  {
    mImpl->SamplerParameteri(sampler, pname, param);
  }

  void SamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param) override
  {
    mImpl->SamplerParameteriv(sampler, pname, param);
  }

  void SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param) override
  {
    mImpl->SamplerParameterf(sampler, pname, param);
  }

  void SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param) override
  {
    mImpl->SamplerParameterfv(sampler, pname, param);
  }

  void GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params) override
  {
    mImpl->GetSamplerParameteriv(sampler, pname, params);
  }

  void GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params) override
  {
    mImpl->GetSamplerParameterfv(sampler, pname, params);
  }

  void VertexAttribDivisor(GLuint index, GLuint divisor) override
  {
    mImpl->VertexAttribDivisor(index, divisor);
  }

  void BindTransformFeedback(GLenum target, GLuint id) override
  {
    mImpl->BindTransformFeedback(target, id);
  }

  void DeleteTransformFeedbacks(GLsizei n, const GLuint* ids) override
  {
    mImpl->DeleteTransformFeedbacks(n, ids);
  }

  void GenTransformFeedbacks(GLsizei n, GLuint* ids) override
  {
    mImpl->GenTransformFeedbacks(n, ids);
  }

  GLboolean IsTransformFeedback(GLuint id) override
  {
    return mImpl->IsTransformFeedback(id);
  }

  void PauseTransformFeedback(void) override
  {
    mImpl->PauseTransformFeedback();
  }

  void ResumeTransformFeedback(void) override
  {
    mImpl->ResumeTransformFeedback();
  }

  void GetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary) override
  {
    mImpl->GetProgramBinary(program, bufSize, length, binaryFormat, binary);
  }

  void ProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length) override
  {
    mImpl->ProgramBinary(program, binaryFormat, binary, length);
  }

  void ProgramParameteri(GLuint program, GLenum pname, GLint value) override
  {
    mImpl->ProgramParameteri(program, pname, value);
  }

  void InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments) override
  {
    mImpl->InvalidateFramebuffer(target, numAttachments, attachments);
  }

  void InvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height) override
  {
    mImpl->InvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
  }

  void TexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) override
  {
    mImpl->TexStorage2D(target, levels, internalformat, width, height);
  }

  void TexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) override
  {
    mImpl->TexStorage3D(target, levels, internalformat, width, height, depth);
  }

  void GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params) override
  {
    mImpl->GetInternalformativ(target, internalformat, pname, bufSize, params);
  }

  void BlendBarrier(void)
  {
    if(mGlExtensionSupportedCacheList.IsSupported(GlExtensionCache::GlExtensionCheckerType::BLEND_EQUATION_ADVANCED))
    {
      mImpl->BlendBarrier();
    }
  }

private:
  std::unique_ptr<GlesAbstraction> mImpl;

  GlExtensionCache::GlExtensionSupportedCacheList mGlExtensionSupportedCacheList;

  ConditionalWait mContextCreatedWaitCondition;
  GLint           mMaxTextureSize;
  GLint           mMaxCombinedTextureUnits;
  GLint           mMaxTextureSamples;
  std::string     mShaderVersionPrefix;
  std::string     mVertexShaderPrefix;
  std::string     mFragmentShaderPrefix;
  int32_t         mGlesVersion;
  int32_t         mShadingLanguageVersion;
  uint32_t        mLogThreshold{0};
  bool            mShadingLanguageVersionCached;
  bool            mIsSurfacelessContextSupported;
  bool            mIsContextCreated;
  bool            mLogEnabled{false};
};
#ifdef START_DURATION_CHECK
#undef START_DURATION_CHECK
#endif

#ifdef FINISH_DURATION_CHECK
#undef FINISH_DURATION_CHECK
#endif

#ifdef FINISH_DURATION_CHECK_WITH_FORMAT
#undef FINISH_DURATION_CHECK_WITH_FORMAT
#endif

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GL_IMPLEMENTATION_H
