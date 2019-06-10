#ifndef DALI_INTERNAL_GLES2_IMPLEMENTATION_H
#define DALI_INTERNAL_GLES2_IMPLEMENTATION_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/gl-extensions.h>
#include <dali/internal/graphics/gles/gles-abstraction.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class Gles2Implementation : public GlesAbstraction
{

public:
  Gles2Implementation() {}

  ~Gles2Implementation() {}

  void ReadBuffer( GLenum mode ) override
  {
    DALI_LOG_ERROR( "glReadBuffer is not supported in OpenGL es 2.0\n" );
  }

  void DrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices ) override
  {
    DALI_LOG_ERROR( "glDrawRangeElements is not supported in OpenGL es 2.0\n" );
  }

  void TexImage3D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels ) override
  {
    DALI_LOG_ERROR( "glTexImage3D is not supported in OpenGL es 2.0\n" );
  }

  void TexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels ) override
  {
    DALI_LOG_ERROR( "glTexSubImage3D is not supported in OpenGL es 2.0\n" );
  }

  void CopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height ) override
  {
    DALI_LOG_ERROR( "glCopyTexSubImage3D is not supported in OpenGL es 2.0\n" );
  }

  void CompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data ) override
  {
    DALI_LOG_ERROR( "glCompressedTexImage3D is not supported in OpenGL es 2.0\n" );
  }

  void CompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data ) override
  {
    DALI_LOG_ERROR( "glCompressedTexSubImage3D is not supported in OpenGL es 2.0\n" );
  }

  void GenQueries( GLsizei n, GLuint* ids ) override
  {
    DALI_LOG_ERROR( "glGenQueries is not supported in OpenGL es 2.0\n" );
  }

  void DeleteQueries( GLsizei n, const GLuint* ids ) override
  {
    DALI_LOG_ERROR( "glDeleteQueries is not supported in OpenGL es 2.0\n" );
  }

  GLboolean IsQuery( GLuint id ) override
  {
    DALI_LOG_ERROR( "glIsQuery is not supported in OpenGL es 2.0\n" );
    return 0;
  }

  void BeginQuery( GLenum target, GLuint id ) override
  {
    DALI_LOG_ERROR( "glBeginQuery is not supported in OpenGL es 2.0\n" );
  }

  void EndQuery( GLenum target ) override
  {
    DALI_LOG_ERROR( "glEndQuery is not supported in OpenGL es 2.0\n" );
  }

  void GetQueryiv( GLenum target, GLenum pname, GLint* params ) override
  {
    DALI_LOG_ERROR( "glGetQueryiv is not supported in OpenGL es 2.0\n" );
  }

  void GetQueryObjectuiv( GLuint id, GLenum pname, GLuint* params ) override
  {
    DALI_LOG_ERROR( "glGetQueryObjectuiv is not supported in OpenGL es 2.0\n" );
  }

  GLboolean UnmapBuffer( GLenum target ) override
  {
    DALI_LOG_ERROR( "glUnmapBuffer is not supported in OpenGL es 2.0\n" );
    return 0;
  }

  void GetBufferPointerv( GLenum target, GLenum pname, GLvoid** params ) override
  {
    DALI_LOG_ERROR( "glGetBufferPointerv is not supported in OpenGL es 2.0\n" );
  }

  void DrawBuffers( GLsizei n, const GLenum* bufs ) override
  {
    DALI_LOG_ERROR( "glDrawBuffers is not supported in OpenGL es 2.0\n" );
  }

  void UniformMatrix2x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    DALI_LOG_ERROR( "glUniformMatrix2x3fv is not supported in OpenGL es 2.0\n" );
  }

  void UniformMatrix3x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    DALI_LOG_ERROR( "glUniformMatrix3x2fv is not supported in OpenGL es 2.0\n" );
  }

  void UniformMatrix2x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    DALI_LOG_ERROR( "glUniformMatrix2x4fv is not supported in OpenGL es 2.0\n" );
  }

  void UniformMatrix4x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    DALI_LOG_ERROR( "glUniformMatrix4x2fv is not supported in OpenGL es 2.0\n" );
  }

  void UniformMatrix3x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    DALI_LOG_ERROR( "glUniformMatrix3x4fv is not supported in OpenGL es 2.0\n" );
  }

  void UniformMatrix4x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    DALI_LOG_ERROR( "glUniformMatrix4x3fv is not supported in OpenGL es 2.0\n" );
  }

  void BlitFramebuffer( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter ) override
  {
    DALI_LOG_ERROR( "glBlitFramebuffer is not supported in OpenGL es 2.0\n" );
  }

  void RenderbufferStorageMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ) override
  {
    DALI_LOG_ERROR( "glRenderbufferStorageMultisample is not supported in OpenGL es 2.0\n" );
  }

  void FramebufferTextureLayer( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer ) override
  {
    DALI_LOG_ERROR( "glFramebufferTextureLayer is not supported in OpenGL es 2.0\n" );
  }

  GLvoid* MapBufferRange( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access ) override
  {
    DALI_LOG_ERROR( "glMapBufferRange is not supported in OpenGL es 2.0\n" );
    return NULL;
  }

  void FlushMappedBufferRange( GLenum target, GLintptr offset, GLsizeiptr length ) override
  {
    DALI_LOG_ERROR( "glFlushMappedBufferRange is not supported in OpenGL es 2.0\n" );
  }

  void BindVertexArray( GLuint array ) override
  {
    DALI_LOG_ERROR( "glBindVertexArray is not supported in OpenGL es 2.0\n" );
  }

  void DeleteVertexArrays( GLsizei n, const GLuint* arrays ) override
  {
    DALI_LOG_ERROR( "glDeleteVertexArrays is not supported in OpenGL es 2.0\n" );
  }

  void GenVertexArrays( GLsizei n, GLuint* arrays ) override
  {
    DALI_LOG_ERROR( "glGenVertexArrays is not supported in OpenGL es 2.0\n" );
  }

  GLboolean IsVertexArray( GLuint array ) override
  {
    DALI_LOG_ERROR( "glIsVertexArray is not supported in OpenGL es 2.0\n" );
    return 0;
  }

  void GetIntegeri_v( GLenum target, GLuint index, GLint* data ) override
  {
    DALI_LOG_ERROR( "glGetIntegeri_v is not supported in OpenGL es 2.0\n" );
  }

  void BeginTransformFeedback( GLenum primitiveMode ) override
  {
    DALI_LOG_ERROR( "glBeginTransformFeedback is not supported in OpenGL es 2.0\n" );
  }

  void EndTransformFeedback( void ) override
  {
    DALI_LOG_ERROR( "glEndTransformFeedback is not supported in OpenGL es 2.0\n" );
  }

  void BindBufferRange( GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size ) override
  {
    DALI_LOG_ERROR( "glBindBufferRange is not supported in OpenGL es 2.0\n" );
  }

  void BindBufferBase( GLenum target, GLuint index, GLuint buffer ) override
  {
    DALI_LOG_ERROR( "glBindBufferBase is not supported in OpenGL es 2.0\n" );
  }

  void TransformFeedbackVaryings( GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode ) override
  {
    DALI_LOG_ERROR( "glTransformFeedbackVaryings is not supported in OpenGL es 2.0\n" );
  }

  void GetTransformFeedbackVarying( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name ) override
  {
    DALI_LOG_ERROR( "glGetTransformFeedbackVarying is not supported in OpenGL es 2.0\n" );
  }

  void VertexAttribIPointer( GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer ) override
  {
    DALI_LOG_ERROR( "glVertexAttribIPointer is not supported in OpenGL es 2.0\n" );
  }

  void GetVertexAttribIiv( GLuint index, GLenum pname, GLint* params ) override
  {
    DALI_LOG_ERROR( "glGetVertexAttribIiv is not supported in OpenGL es 2.0\n" );
  }

  void GetVertexAttribIuiv( GLuint index, GLenum pname, GLuint* params ) override
  {
    DALI_LOG_ERROR( "glGetVertexAttribIuiv is not supported in OpenGL es 2.0\n" );
  }

  void VertexAttribI4i( GLuint index, GLint x, GLint y, GLint z, GLint w ) override
  {
    DALI_LOG_ERROR( "glVertexAttribI4i is not supported in OpenGL es 2.0\n" );
  }

  void VertexAttribI4ui( GLuint index, GLuint x, GLuint y, GLuint z, GLuint w ) override
  {
    DALI_LOG_ERROR( "glVertexAttribI4ui is not supported in OpenGL es 2.0\n" );
  }

  void VertexAttribI4iv( GLuint index, const GLint* v ) override
  {
    DALI_LOG_ERROR( "glVertexAttribI4iv is not supported in OpenGL es 2.0\n" );
  }

  void VertexAttribI4uiv( GLuint index, const GLuint* v ) override
  {
    DALI_LOG_ERROR( "glVertexAttribI4uiv is not supported in OpenGL es 2.0\n" );
  }

  void GetUniformuiv( GLuint program, GLint location, GLuint* params ) override
  {
    DALI_LOG_ERROR( "glGetUniformuiv is not supported in OpenGL es 2.0\n" );
  }

  GLint GetFragDataLocation( GLuint program, const GLchar *name ) override
  {
    DALI_LOG_ERROR( "glGetFragDataLocation is not supported in OpenGL es 2.0\n" );
    return -1;
  }

  void Uniform1ui( GLint location, GLuint v0 ) override
  {
    DALI_LOG_ERROR( "glUniform1ui is not supported in OpenGL es 2.0\n" );
  }

  void Uniform2ui( GLint location, GLuint v0, GLuint v1 ) override
  {
    DALI_LOG_ERROR( "glUniform2ui is not supported in OpenGL es 2.0\n" );
  }

  void Uniform3ui( GLint location, GLuint v0, GLuint v1, GLuint v2 ) override
  {
    DALI_LOG_ERROR( "glUniform3ui is not supported in OpenGL es 2.0\n" );
  }

  void Uniform4ui( GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3 ) override
  {
    DALI_LOG_ERROR( "glUniform4ui is not supported in OpenGL es 2.0\n" );
  }

  void Uniform1uiv( GLint location, GLsizei count, const GLuint* value ) override
  {
    DALI_LOG_ERROR( "glUniform1uiv is not supported in OpenGL es 2.0\n" );
  }

  void Uniform2uiv( GLint location, GLsizei count, const GLuint* value ) override
  {
    DALI_LOG_ERROR( "glUniform2uiv is not supported in OpenGL es 2.0\n" );
  }

  void Uniform3uiv( GLint location, GLsizei count, const GLuint* value ) override
  {
    DALI_LOG_ERROR( "glUniform3uiv is not supported in OpenGL es 2.0\n" );
  }

  void Uniform4uiv( GLint location, GLsizei count, const GLuint* value ) override
  {
    DALI_LOG_ERROR( "glUniform4uiv is not supported in OpenGL es 2.0\n" );
  }

  void ClearBufferiv( GLenum buffer, GLint drawbuffer, const GLint* value ) override
  {
    DALI_LOG_ERROR( "glClearBufferiv is not supported in OpenGL es 2.0\n" );
  }

  void ClearBufferuiv( GLenum buffer, GLint drawbuffer, const GLuint* value ) override
  {
    DALI_LOG_ERROR( "glClearBufferuiv is not supported in OpenGL es 2.0\n" );
  }

  void ClearBufferfv( GLenum buffer, GLint drawbuffer, const GLfloat* value ) override
  {
    DALI_LOG_ERROR( "glClearBufferfv is not supported in OpenGL es 2.0\n" );
  }

  void ClearBufferfi( GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil ) override
  {
    DALI_LOG_ERROR( "glClearBufferfi is not supported in OpenGL es 2.0\n" );
  }

  const GLubyte* GetStringi( GLenum name, GLuint index ) override
  {
    DALI_LOG_ERROR( "glGetStringi is not supported in OpenGL es 2.0\n" );
    return NULL;
  }

  void CopyBufferSubData( GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size ) override
  {
    DALI_LOG_ERROR( "glCopyBufferSubData is not supported in OpenGL es 2.0\n" );
  }

  void GetUniformIndices( GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices ) override
  {
    DALI_LOG_ERROR( "glGetUniformIndices is not supported in OpenGL es 2.0\n" );
  }

  void GetActiveUniformsiv( GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params ) override
  {
    DALI_LOG_ERROR( "glGetActiveUniformsiv is not supported in OpenGL es 2.0\n" );
  }

  GLuint GetUniformBlockIndex( GLuint program, const GLchar* uniformBlockName ) override
  {
    DALI_LOG_ERROR( "glGetUniformBlockIndex is not supported in OpenGL es 2.0\n" );
    return 0;
  }

  void GetActiveUniformBlockiv( GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params ) override
  {
    DALI_LOG_ERROR( "glGetActiveUniformBlockiv is not supported in OpenGL es 2.0\n" );
  }

  void GetActiveUniformBlockName( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName ) override
  {
    DALI_LOG_ERROR( "glGetActiveUniformBlockName is not supported in OpenGL es 2.0\n" );
  }

  void UniformBlockBinding( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding ) override
  {
    DALI_LOG_ERROR( "glUniformBlockBinding is not supported in OpenGL es 2.0\n" );
  }

  void DrawArraysInstanced( GLenum mode, GLint first, GLsizei count, GLsizei instanceCount ) override
  {
    DALI_LOG_ERROR( "glDrawArraysInstanced is not supported in OpenGL es 2.0\n" );
  }

  void DrawElementsInstanced( GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount ) override
  {
    DALI_LOG_ERROR( "glDrawElementsInstanced is not supported in OpenGL es 2.0\n" );
  }

  GLsync FenceSync( GLenum condition, GLbitfield flags ) override
  {
    DALI_LOG_ERROR( "glFenceSync is not supported in OpenGL es 2.0\n" );
    return NULL;
  }

  GLboolean IsSync( GLsync sync ) override
  {
    DALI_LOG_ERROR( "glIsSync is not supported in OpenGL es 2.0\n" );
    return 0;
  }

  void DeleteSync( GLsync sync ) override
  {
    DALI_LOG_ERROR( "glDeleteSync is not supported in OpenGL es 2.0\n" );
  }

  GLenum ClientWaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout ) override
  {
    DALI_LOG_ERROR( "glClientWaitSync is not supported in OpenGL es 2.0\n" );
    return 0;
  }

  void WaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout ) override
  {
    DALI_LOG_ERROR( "glWaitSync is not supported in OpenGL es 2.0\n" );
  }

  void GetInteger64v( GLenum pname, GLint64* params ) override
  {
    DALI_LOG_ERROR( "glGetInteger64v is not supported in OpenGL es 2.0\n" );
  }

  void GetSynciv( GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values ) override
  {
    DALI_LOG_ERROR( "glGetSynciv is not supported in OpenGL es 2.0\n" );
  }

  void GetInteger64i_v( GLenum target, GLuint index, GLint64* data ) override
  {
    DALI_LOG_ERROR( "glGetInteger64i_v is not supported in OpenGL es 2.0\n" );
  }

  void GetBufferParameteri64v( GLenum target, GLenum pname, GLint64* params ) override
  {
    DALI_LOG_ERROR( "glGetBufferParameteri64v is not supported in OpenGL es 2.0\n" );
  }

  void GenSamplers( GLsizei count, GLuint* samplers ) override
  {
    DALI_LOG_ERROR( "glGenSamplers is not supported in OpenGL es 2.0\n" );
  }

  void DeleteSamplers( GLsizei count, const GLuint* samplers ) override
  {
    DALI_LOG_ERROR( "glDeleteSamplers is not supported in OpenGL es 2.0\n" );
  }

  GLboolean IsSampler( GLuint sampler ) override
  {
    DALI_LOG_ERROR( "glIsSampler is not supported in OpenGL es 2.0\n" );
    return 0;
  }

  void BindSampler( GLuint unit, GLuint sampler ) override
  {
    DALI_LOG_ERROR( "glBindSampler is not supported in OpenGL es 2.0\n" );
  }

  void SamplerParameteri( GLuint sampler, GLenum pname, GLint param ) override
  {
    DALI_LOG_ERROR( "glSamplerParameteri is not supported in OpenGL es 2.0\n" );
  }

  void SamplerParameteriv( GLuint sampler, GLenum pname, const GLint* param ) override
  {
    DALI_LOG_ERROR( "glSamplerParameteriv is not supported in OpenGL es 2.0\n" );
  }

  void SamplerParameterf( GLuint sampler, GLenum pname, GLfloat param ) override
  {
    DALI_LOG_ERROR( "glSamplerParameterf is not supported in OpenGL es 2.0\n" );
  }

  void SamplerParameterfv( GLuint sampler, GLenum pname, const GLfloat* param ) override
  {
    DALI_LOG_ERROR( "glSamplerParameterfv is not supported in OpenGL es 2.0\n" );
  }

  void GetSamplerParameteriv( GLuint sampler, GLenum pname, GLint* params ) override
  {
    DALI_LOG_ERROR( "glGetSamplerParameteriv is not supported in OpenGL es 2.0\n" );
  }

  void GetSamplerParameterfv( GLuint sampler, GLenum pname, GLfloat* params ) override
  {
    DALI_LOG_ERROR( "glGetSamplerParameterfv is not supported in OpenGL es 2.0\n" );
  }

  void VertexAttribDivisor( GLuint index, GLuint divisor ) override
  {
    DALI_LOG_ERROR( "glVertexAttribDivisor is not supported in OpenGL es 2.0\n" );
  }

  void BindTransformFeedback( GLenum target, GLuint id ) override
  {
    DALI_LOG_ERROR( "glBindTransformFeedback is not supported in OpenGL es 2.0\n" );
  }

  void DeleteTransformFeedbacks( GLsizei n, const GLuint* ids ) override
  {
    DALI_LOG_ERROR( "glDeleteTransformFeedbacks is not supported in OpenGL es 2.0\n" );
  }

  void GenTransformFeedbacks( GLsizei n, GLuint* ids ) override
  {
    DALI_LOG_ERROR( "glGenTransformFeedbacks is not supported in OpenGL es 2.0\n" );
  }

  GLboolean IsTransformFeedback( GLuint id ) override
  {
    DALI_LOG_ERROR( "glIsTransformFeedback is not supported in OpenGL es 2.0\n" );
    return 0;
  }

  void PauseTransformFeedback( void ) override
  {
    DALI_LOG_ERROR( "glPauseTransformFeedback is not supported in OpenGL es 2.0\n" );
  }

  void ResumeTransformFeedback( void ) override
  {
    DALI_LOG_ERROR( "glResumeTransformFeedback is not supported in OpenGL es 2.0\n" );
  }

  void GetProgramBinary( GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary ) override
  {
    mGlExtensions.GetProgramBinaryOES( program, bufSize, length, binaryFormat, binary );
  }

  void ProgramBinary( GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length ) override
  {
    mGlExtensions.ProgramBinaryOES( program, binaryFormat, binary, length );
  }

  void ProgramParameteri( GLuint program, GLenum pname, GLint value ) override
  {
    DALI_LOG_ERROR( "glProgramParameteri is not supported in OpenGL es 2.0\n" );
  }

  void InvalidateFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments ) override
  {
    mGlExtensions.DiscardFrameBuffer( target, numAttachments, attachments );
  }

  void InvalidateSubFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height ) override
  {
    DALI_LOG_ERROR( "glInvalidateSubFramebuffer is not supported in OpenGL es 2.0\n" );
  }

  void TexStorage2D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height ) override
  {
    DALI_LOG_ERROR( "glTexStorage2D is not supported in OpenGL es 2.0\n" );
  }

  void TexStorage3D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth ) override
  {
    DALI_LOG_ERROR( "glTexStorage3D is not supported in OpenGL es 2.0\n" );
  }

  void GetInternalformativ( GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params ) override
  {
    DALI_LOG_ERROR( "glGetInternalformativ is not supported in OpenGL es 2.0\n" );
  }

private:
  ECoreX::GlExtensions mGlExtensions;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GLES2_IMPLEMENTATION_H
