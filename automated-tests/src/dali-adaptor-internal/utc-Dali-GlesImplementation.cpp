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

#include <dali/internal/graphics/gles/gles2-implementation.h>
#include <dali/internal/graphics/gles/gles3-implementation.h>

void gles_implementation_startup(void)
{
}

void gles_implementation_cleanup(void)
{
}

using namespace Dali::Internal::Adaptor;

namespace
{
void CallAllMethods(GlesAbstraction& implementation)
{
  // These tests are purely for coverage, they don't really test anything!
  try
  {
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

template<typename Impl>
void CreateWithBaseClassPtr()
{
  // This derived destructor of the class is called which is purely for increasing function coverage
  GlesAbstraction* abstraction = new Impl;
  try
  {
    abstraction->ReadBuffer(0);
    DALI_TEST_CHECK(true);
  }
  catch(...)
  {
    DALI_TEST_CHECK(false);
  }
  delete abstraction;
}

} // unnamed namespace

int UtcDaliGles2ImplementationConstruction(void)
{
  CreateWithBaseClassPtr<Gles2Implementation>();
  END_TEST;
}

int UtcDaliGles3ImplementationConstruction(void)
{
  CreateWithBaseClassPtr<Gles3Implementation>();
  END_TEST;
}

int UtcDaliGles2ImplementationMethods(void)
{
  Gles2Implementation implementation;
  CallAllMethods(implementation);
  END_TEST;
}

int UtcDaliGles3ImplementationMethods(void)
{
  Gles3Implementation implementation;
  CallAllMethods(implementation);
  END_TEST;
}