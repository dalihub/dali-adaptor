/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <dali-test-suite-utils.h>

#include <dali/internal/graphics/gles/gl-implementation.h>
#include <dali/internal/graphics/gles/gl-proxy-implementation.h>
#include <dali/internal/system/common/environment-options.h>

void gl_implementation_startup(void)
{
}

void gl_implementation_cleanup(void)
{
}

using namespace Dali::Internal::Adaptor;

namespace
{
void CallAllMethods(Dali::Integration::GlAbstraction& implementation)
{
  // These tests are purely for coverage, they don't really test anything!
  try
  {
    implementation.PreRender();
    implementation.PostRender();
    implementation.IsSurfacelessContextSupported();

    // Cannot call the following functions as they have a Wait
    //    implementation.IsAdvancedBlendEquationSupported();
    //    implementation.IsMultisampledRenderToTextureSupported();
    //    implementation.IsBlendEquationSupported(Dali::DevelBlendEquation::ADD);
    //    implementation.GetShaderVersionPrefix();
    //    implementation.GetVertexShaderPrefix();
    //    implementation.GetFragmentShaderPrefix();

    implementation.TextureRequiresConverting(0, 0, false);
    implementation.ActiveTexture(0);
    implementation.AttachShader(0, 0);
    implementation.BindAttribLocation(0, 0, nullptr);
    implementation.BindBuffer(0, 0);
    implementation.BindFramebuffer(0, 0);
    implementation.BindRenderbuffer(0, 0);
    implementation.BindTexture(0, 0);
    implementation.BlendColor(0, 0, 0, 0);
    implementation.BlendEquation(0);
    implementation.BlendEquationSeparate(0, 0);
    implementation.BlendFunc(0, 0);
    implementation.BlendFuncSeparate(0, 0, 0, 0);
    implementation.BufferData(0, 0, nullptr, 0);
    implementation.BufferSubData(0, 0, 0, nullptr);
    implementation.CheckFramebufferStatus(0);
    implementation.Clear(0);
    implementation.ClearColor(0, 0, 0, 0);
    implementation.ClearDepthf(0);
    implementation.ClearStencil(0);
    implementation.ColorMask(0, 0, 0, 0);
    implementation.CompileShader(0);
    implementation.CompressedTexImage2D(0, 0, 0, 0, 0, 0, 0, nullptr);
    implementation.CompressedTexSubImage2D(0, 0, 0, 0, 0, 0, 0, 0, nullptr);
    implementation.CopyTexImage2D(0, 0, 0, 0, 0, 0, 0, 0);
    implementation.CopyTexSubImage2D(0, 0, 0, 0, 0, 0, 0, 0);
    implementation.CreateProgram();
    implementation.CreateShader(0);
    implementation.CullFace(0);
    implementation.DeleteBuffers(0, nullptr);
    implementation.DeleteFramebuffers(0, nullptr);
    implementation.DeleteProgram(0);
    implementation.DeleteRenderbuffers(0, nullptr);
    implementation.DeleteShader(0);
    implementation.DeleteTextures(0, nullptr);
    implementation.DepthFunc(0);
    implementation.DepthMask(0);
    implementation.DepthRangef(0, 0);
    implementation.DetachShader(0, 0);
    implementation.Disable(0);
    implementation.DisableVertexAttribArray(0);
    implementation.DrawArrays(0, 0, 0);
    implementation.DrawElements(0, 0, 0, nullptr);
    implementation.Enable(0);
    implementation.EnableVertexAttribArray(0);
    implementation.Finish();
    implementation.Flush();
    implementation.FramebufferRenderbuffer(0, 0, 0, 0);
    implementation.FramebufferTexture2D(0, 0, 0, 0, 0);
    implementation.FrontFace(0);
    implementation.GenBuffers(0, nullptr);
    implementation.GenerateMipmap(0);
    implementation.GenFramebuffers(0, nullptr);
    implementation.GenRenderbuffers(0, nullptr);
    implementation.GenTextures(0, nullptr);
    implementation.GetActiveAttrib(0, 0, 0, nullptr, nullptr, nullptr, nullptr);
    implementation.GetActiveUniform(0, 0, 0, nullptr, nullptr, nullptr, nullptr);
    implementation.GetAttachedShaders(0, 0, nullptr, nullptr);
    implementation.GetAttribLocation(0, nullptr);
    implementation.GetBooleanv(0, nullptr);
    implementation.GetBufferParameteriv(0, 0, nullptr);
    implementation.GetError();
    implementation.GetFloatv(0, nullptr);
    implementation.GetFramebufferAttachmentParameteriv(0, 0, 0, nullptr);
    implementation.GetIntegerv(0, nullptr);
    implementation.GetProgramiv(0, 0, nullptr);
    implementation.GetProgramInfoLog(0, 0, nullptr, nullptr);
    implementation.GetRenderbufferParameteriv(0, 0, nullptr);
    implementation.GetShaderiv(0, 0, nullptr);
    implementation.GetShaderInfoLog(0, 0, nullptr, nullptr);
    implementation.GetShaderPrecisionFormat(0, 0, nullptr, nullptr);
    implementation.GetShaderSource(0, 0, nullptr, nullptr);
    implementation.GetString(0);
    implementation.GetTexParameterfv(0, 0, nullptr);
    implementation.GetTexParameteriv(0, 0, nullptr);
    implementation.GetUniformfv(0, 0, nullptr);
    implementation.GetUniformiv(0, 0, nullptr);
    implementation.GetUniformLocation(0, nullptr);
    implementation.GetVertexAttribfv(0, 0, nullptr);
    implementation.GetVertexAttribiv(0, 0, nullptr);
    implementation.GetVertexAttribPointerv(0, 0, nullptr);
    implementation.Hint(0, 0);
    implementation.IsBuffer(0);
    implementation.IsEnabled(0);
    implementation.IsFramebuffer(0);
    implementation.IsProgram(0);
    implementation.IsRenderbuffer(0);
    implementation.IsShader(0);
    implementation.IsTexture(0);
    implementation.LineWidth(0);
    implementation.LinkProgram(0);
    implementation.PixelStorei(0, 0);
    implementation.PolygonOffset(0, 0);
    implementation.ReadPixels(0, 0, 0, 0, 0, 0, nullptr);
    implementation.ReleaseShaderCompiler();
    implementation.RenderbufferStorage(0, 0, 0, 0);
    implementation.SampleCoverage(0, 0);
    implementation.Scissor(0, 0, 0, 0);
    implementation.ShaderBinary(0, nullptr, 0, nullptr, 0);
    implementation.ShaderSource(0, 0, nullptr, nullptr);
    implementation.StencilFunc(0, 0, 0);
    implementation.StencilFuncSeparate(0, 0, 0, 0);
    implementation.StencilMask(0);
    implementation.StencilMaskSeparate(0, 0);
    implementation.StencilOp(0, 0, 0);
    implementation.StencilOpSeparate(0, 0, 0, 0);
    implementation.TexImage2D(0, 0, 0, 0, 0, 0, 0, 0, nullptr);
    implementation.TexParameterf(0, 0, 0);
    implementation.TexParameterfv(0, 0, nullptr);
    implementation.TexParameteri(0, 0, 0);
    implementation.TexParameteriv(0, 0, nullptr);
    implementation.TexSubImage2D(0, 0, 0, 0, 0, 0, 0, 0, nullptr);
    implementation.Uniform1f(0, 0);
    implementation.Uniform1fv(0, 0, nullptr);
    implementation.Uniform1i(0, 0);
    implementation.Uniform1iv(0, 0, nullptr);
    implementation.Uniform2f(0, 0, 0);
    implementation.Uniform2fv(0, 0, nullptr);
    implementation.Uniform2i(0, 0, 0);
    implementation.Uniform2iv(0, 0, nullptr);
    implementation.Uniform3f(0, 0, 0, 0);
    implementation.Uniform3fv(0, 0, nullptr);
    implementation.Uniform3i(0, 0, 0, 0);
    implementation.Uniform3iv(0, 0, nullptr);
    implementation.Uniform4f(0, 0, 0, 0, 0);
    implementation.Uniform4fv(0, 0, nullptr);
    implementation.Uniform4i(0, 0, 0, 0, 0);
    implementation.Uniform4iv(0, 0, nullptr);
    implementation.UniformMatrix2fv(0, 0, 0, nullptr);
    implementation.UniformMatrix3fv(0, 0, 0, nullptr);
    implementation.UniformMatrix4fv(0, 0, 0, nullptr);
    implementation.UseProgram(0);
    implementation.ValidateProgram(0);
    implementation.VertexAttrib1f(0, 0);
    implementation.VertexAttrib1fv(0, nullptr);
    implementation.VertexAttrib2f(0, 0, 0);
    implementation.VertexAttrib2fv(0, nullptr);
    implementation.VertexAttrib3f(0, 0, 0, 0);
    implementation.VertexAttrib3fv(0, nullptr);
    implementation.VertexAttrib4f(0, 0, 0, 0, 0);
    implementation.VertexAttrib4fv(0, nullptr);
    implementation.VertexAttribPointer(0, 0, 0, 0, 0, nullptr);
    implementation.Viewport(0, 0, 0, 0);
    implementation.ReadBuffer(0);
    implementation.DrawRangeElements(0, 0, 0, 0, 0, nullptr);
    implementation.TexImage3D(0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr);
    implementation.TexSubImage3D(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr);
    implementation.CopyTexSubImage3D(0, 0, 0, 0, 0, 0, 0, 0, 0);
    implementation.CompressedTexImage3D(0, 0, 0, 0, 0, 0, 0, 0, nullptr);
    implementation.CompressedTexSubImage3D(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr);
    implementation.GenQueries(0, nullptr);
    implementation.DeleteQueries(0, nullptr);
    implementation.IsQuery(0);
    implementation.BeginQuery(0, 0);
    implementation.EndQuery(0);
    implementation.GetQueryiv(0, 0, nullptr);
    implementation.GetQueryObjectuiv(0, 0, nullptr);
    implementation.UnmapBuffer(0);
    implementation.GetBufferPointerv(0, 0, nullptr);
    implementation.DrawBuffers(0, nullptr);
    implementation.UniformMatrix2x3fv(0, 0, 0, nullptr);
    implementation.UniformMatrix3x2fv(0, 0, 0, nullptr);
    implementation.UniformMatrix2x4fv(0, 0, 0, nullptr);
    implementation.UniformMatrix4x2fv(0, 0, 0, nullptr);
    implementation.UniformMatrix3x4fv(0, 0, 0, nullptr);
    implementation.UniformMatrix4x3fv(0, 0, 0, nullptr);
    implementation.BlitFramebuffer(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    implementation.RenderbufferStorageMultisample(0, 0, 0, 0, 0);
    implementation.FramebufferTexture2DMultisample(0, 0, 0, 0, 0, 0);
    implementation.FramebufferTextureLayer(0, 0, 0, 0, 0);
    implementation.MapBufferRange(0, 0, 0, 0);
    implementation.FlushMappedBufferRange(0, 0, 0);
    implementation.BindVertexArray(0);
    implementation.DeleteVertexArrays(0, nullptr);
    implementation.GenVertexArrays(0, nullptr);
    implementation.IsVertexArray(0);
    implementation.GetIntegeri_v(0, 0, nullptr);
    implementation.BeginTransformFeedback(0);
    implementation.EndTransformFeedback();
    implementation.BindBufferRange(0, 0, 0, 0, 0);
    implementation.BindBufferBase(0, 0, 0);
    implementation.TransformFeedbackVaryings(0, 0, nullptr, 0);
    implementation.GetTransformFeedbackVarying(0, 0, 0, nullptr, nullptr, nullptr, nullptr);
    implementation.VertexAttribIPointer(0, 0, 0, 0, nullptr);
    implementation.GetVertexAttribIiv(0, 0, nullptr);
    implementation.GetVertexAttribIuiv(0, 0, nullptr);
    implementation.VertexAttribI4i(0, 0, 0, 0, 0);
    implementation.VertexAttribI4ui(0, 0, 0, 0, 0);
    implementation.VertexAttribI4iv(0, nullptr);
    implementation.VertexAttribI4uiv(0, nullptr);
    implementation.GetUniformuiv(0, 0, nullptr);
    implementation.GetFragDataLocation(0, nullptr);
    implementation.Uniform1ui(0, 0);
    implementation.Uniform2ui(0, 0, 0);
    implementation.Uniform3ui(0, 0, 0, 0);
    implementation.Uniform4ui(0, 0, 0, 0, 0);
    implementation.Uniform1uiv(0, 0, nullptr);
    implementation.Uniform2uiv(0, 0, nullptr);
    implementation.Uniform3uiv(0, 0, nullptr);
    implementation.Uniform4uiv(0, 0, nullptr);
    implementation.ClearBufferiv(0, 0, nullptr);
    implementation.ClearBufferuiv(0, 0, nullptr);
    implementation.ClearBufferfv(0, 0, nullptr);
    implementation.ClearBufferfi(0, 0, 0, 0);
    implementation.GetStringi(0, 0);
    implementation.CopyBufferSubData(0, 0, 0, 0, 0);
    implementation.GetUniformIndices(0, 0, nullptr, nullptr);
    implementation.GetActiveUniformsiv(0, 0, nullptr, 0, nullptr);
    implementation.GetUniformBlockIndex(0, nullptr);
    implementation.GetActiveUniformBlockiv(0, 0, 0, nullptr);
    implementation.GetActiveUniformBlockName(0, 0, 0, nullptr, nullptr);
    implementation.UniformBlockBinding(0, 0, 0);
    implementation.DrawArraysInstanced(0, 0, 0, 0);
    implementation.DrawElementsInstanced(0, 0, 0, nullptr, 0);
    implementation.FenceSync(0, 0);
    implementation.IsSync(0);
    implementation.DeleteSync(0);
    implementation.ClientWaitSync(0, 0, 0);
    implementation.WaitSync(0, 0, 0);
    implementation.GetInteger64v(0, nullptr);
    implementation.GetSynciv(0, 0, 0, nullptr, nullptr);
    implementation.GetInteger64i_v(0, 0, nullptr);
    implementation.GetBufferParameteri64v(0, 0, nullptr);
    implementation.GenSamplers(0, nullptr);
    implementation.DeleteSamplers(0, nullptr);
    implementation.IsSampler(0);
    implementation.BindSampler(0, 0);
    implementation.SamplerParameteri(0, 0, 0);
    implementation.SamplerParameteriv(0, 0, nullptr);
    implementation.SamplerParameterf(0, 0, 0);
    implementation.SamplerParameterfv(0, 0, nullptr);
    implementation.GetSamplerParameteriv(0, 0, nullptr);
    implementation.GetSamplerParameterfv(0, 0, nullptr);
    implementation.VertexAttribDivisor(0, 0);
    implementation.BindTransformFeedback(0, 0);
    implementation.DeleteTransformFeedbacks(0, nullptr);
    implementation.GenTransformFeedbacks(0, nullptr);
    implementation.IsTransformFeedback(0);
    implementation.PauseTransformFeedback();
    implementation.ResumeTransformFeedback();
    implementation.GetProgramBinary(0, 0, nullptr, nullptr, nullptr);
    implementation.ProgramBinary(0, 0, nullptr, 0);
    implementation.ProgramParameteri(0, 0, 0);
    implementation.InvalidateFramebuffer(0, 0, nullptr);
    implementation.InvalidateSubFramebuffer(0, 0, nullptr, 0, 0, 0, 0);
    implementation.TexStorage2D(0, 0, 0, 0, 0);
    implementation.TexStorage3D(0, 0, 0, 0, 0, 0);
    implementation.GetInternalformativ(0, 0, 0, 0, nullptr);
    implementation.BlendBarrier();

    DALI_TEST_CHECK(true); // Should get here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(false); // Should not get here!
  }
}
} // unnamed namespace

int UtcDaliGlImplementationConstruction(void)
{
  // This derived destructor of the class is called which is purely for increasing function coverage
  Dali::Integration::GlAbstraction* abstraction    = new GlImplementation;
  GlImplementation*                 implementation = dynamic_cast<GlImplementation*>(abstraction);
  DALI_TEST_CHECK(implementation);
  delete implementation;
  END_TEST;
}

int UtcDaliGlProxyImplementationConstruction(void)
{
  // This derived destructor of the class is called which is purely for increasing function coverage
  EnvironmentOptions                envOptions;
  Dali::Integration::GlAbstraction* abstraction    = new GlProxyImplementation(envOptions);
  GlProxyImplementation*            implementation = dynamic_cast<GlProxyImplementation*>(abstraction);
  DALI_TEST_CHECK(implementation);
  delete implementation;
  END_TEST;
}

int UtcDaliGlImplementationMethods(void)
{
  GlImplementation implementation;
  CallAllMethods(implementation);
  END_TEST;
}

int UtcDaliGlProxyImplementationMethods(void)
{
  EnvironmentOptions    envOptions;
  GlProxyImplementation implementation(envOptions);
  CallAllMethods(implementation);
  END_TEST;
}