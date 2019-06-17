#ifndef DALI_INTERNAL_GLES3_IMPLEMENTATION_H
#define DALI_INTERNAL_GLES3_IMPLEMENTATION_H

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
#include <GLES3/gl3.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/gles-abstraction.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class Gles3Implementation : public GlesAbstraction
{

public:
  Gles3Implementation() {}

  ~Gles3Implementation() {}

  void ReadBuffer( GLenum mode ) override
  {
    glReadBuffer( mode );
  }

  void DrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices ) override
  {
    glDrawRangeElements( mode, start, end, count, type, indices );
  }

  void TexImage3D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels ) override
  {
    glTexImage3D( target, level, internalformat, width, height, depth, border, format, type, pixels );
  }

  void TexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels ) override
  {
    glTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels );
  }

  void CopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height ) override
  {
    glCopyTexSubImage3D( target, level, xoffset, yoffset, zoffset, x, y, width, height );
  }

  void CompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data ) override
  {
    glCompressedTexImage3D( target, level, internalformat, width, height, depth, border, imageSize, data );
  }

  void CompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data ) override
  {
    glCompressedTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data );
  }

  void GenQueries( GLsizei n, GLuint* ids ) override
  {
    glGenQueries( n, ids );
  }

  void DeleteQueries( GLsizei n, const GLuint* ids ) override
  {
    glDeleteQueries( n, ids );
  }

  GLboolean IsQuery( GLuint id ) override
  {
    return glIsQuery( id );
  }

  void BeginQuery( GLenum target, GLuint id ) override
  {
    glBeginQuery( target, id );
  }

  void EndQuery( GLenum target ) override
  {
    glEndQuery( target );
  }

  void GetQueryiv( GLenum target, GLenum pname, GLint* params ) override
  {
    glGetQueryiv( target, pname, params );
  }

  void GetQueryObjectuiv( GLuint id, GLenum pname, GLuint* params ) override
  {
    glGetQueryObjectuiv( id, pname, params );
  }

  GLboolean UnmapBuffer( GLenum target ) override
  {
    return glUnmapBuffer( target );
  }

  void GetBufferPointerv( GLenum target, GLenum pname, GLvoid** params ) override
  {
    glGetBufferPointerv( target, pname, params );
  }

  void DrawBuffers( GLsizei n, const GLenum* bufs ) override
  {
    glDrawBuffers( n, bufs );
  }

  void UniformMatrix2x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    glUniformMatrix2x3fv( location, count, transpose, value );
  }

  void UniformMatrix3x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    glUniformMatrix3x2fv( location, count, transpose, value );
  }

  void UniformMatrix2x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    glUniformMatrix2x4fv( location, count, transpose, value );
  }

  void UniformMatrix4x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    glUniformMatrix4x2fv( location, count, transpose, value );
  }

  void UniformMatrix3x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    glUniformMatrix3x4fv( location, count, transpose, value );
  }

  void UniformMatrix4x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) override
  {
    glUniformMatrix4x3fv( location, count, transpose, value );
  }

  void BlitFramebuffer( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter ) override
  {
    glBlitFramebuffer( srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter );
  }

  void RenderbufferStorageMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ) override
  {
    glRenderbufferStorageMultisample( target, samples, internalformat, width, height );
  }

  void FramebufferTextureLayer( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer ) override
  {
    glFramebufferTextureLayer( target, attachment, texture, level, layer );
  }

  GLvoid* MapBufferRange( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access ) override
  {
    return glMapBufferRange( target, offset, length, access );
  }

  void FlushMappedBufferRange( GLenum target, GLintptr offset, GLsizeiptr length ) override
  {
    glFlushMappedBufferRange( target, offset, length );
  }

  void BindVertexArray( GLuint array ) override
  {
    glBindVertexArray( array );
  }

  void DeleteVertexArrays( GLsizei n, const GLuint* arrays ) override
  {
    glDeleteVertexArrays( n, arrays );
  }

  void GenVertexArrays( GLsizei n, GLuint* arrays ) override
  {
    glGenVertexArrays( n, arrays );
  }

  GLboolean IsVertexArray( GLuint array ) override
  {
    return glIsVertexArray( array );
  }

  void GetIntegeri_v( GLenum target, GLuint index, GLint* data ) override
  {
    glGetIntegeri_v( target, index, data );
  }

  void BeginTransformFeedback( GLenum primitiveMode ) override
  {
    glBeginTransformFeedback( primitiveMode );
  }

  void EndTransformFeedback( void ) override
  {
    glEndTransformFeedback();
  }

  void BindBufferRange( GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size ) override
  {
    glBindBufferRange( target, index, buffer, offset, size );
  }

  void BindBufferBase( GLenum target, GLuint index, GLuint buffer ) override
  {
    glBindBufferBase( target, index, buffer );
  }

  void TransformFeedbackVaryings( GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode ) override
  {
    glTransformFeedbackVaryings( program, count, varyings, bufferMode );
  }

  void GetTransformFeedbackVarying( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name ) override
  {
    glGetTransformFeedbackVarying( program, index, bufSize, length, size, type, name );
  }

  void VertexAttribIPointer( GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer ) override
  {
    glVertexAttribIPointer( index, size, type, stride, pointer );
  }

  void GetVertexAttribIiv( GLuint index, GLenum pname, GLint* params ) override
  {
    glGetVertexAttribIiv( index, pname, params );
  }

  void GetVertexAttribIuiv( GLuint index, GLenum pname, GLuint* params ) override
  {
    glGetVertexAttribIuiv( index, pname, params );
  }

  void VertexAttribI4i( GLuint index, GLint x, GLint y, GLint z, GLint w ) override
  {
    glVertexAttribI4i( index, x, y, z, w );
  }

  void VertexAttribI4ui( GLuint index, GLuint x, GLuint y, GLuint z, GLuint w ) override
  {
    glVertexAttribI4ui( index, x, y, z, w );
  }

  void VertexAttribI4iv( GLuint index, const GLint* v ) override
  {
    glVertexAttribI4iv( index, v );
  }

  void VertexAttribI4uiv( GLuint index, const GLuint* v ) override
  {
    glVertexAttribI4uiv( index, v );
  }

  void GetUniformuiv( GLuint program, GLint location, GLuint* params ) override
  {
    glGetUniformuiv( program, location, params );
  }

  GLint GetFragDataLocation( GLuint program, const GLchar *name ) override
  {
    return glGetFragDataLocation( program, name );
  }

  void Uniform1ui( GLint location, GLuint v0 ) override
  {
    glUniform1ui( location, v0 );
  }

  void Uniform2ui( GLint location, GLuint v0, GLuint v1 ) override
  {
    glUniform2ui( location, v0, v1 );
  }

  void Uniform3ui( GLint location, GLuint v0, GLuint v1, GLuint v2 ) override
  {
    glUniform3ui( location, v0, v1, v2 );
  }

  void Uniform4ui( GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3 ) override
  {
    glUniform4ui( location, v0, v1, v2, v3 );
  }

  void Uniform1uiv( GLint location, GLsizei count, const GLuint* value ) override
  {
    glUniform1uiv( location, count, value );
  }

  void Uniform2uiv( GLint location, GLsizei count, const GLuint* value ) override
  {
    glUniform2uiv( location, count, value );
  }

  void Uniform3uiv( GLint location, GLsizei count, const GLuint* value ) override
  {
    glUniform3uiv( location, count, value );
  }

  void Uniform4uiv( GLint location, GLsizei count, const GLuint* value ) override
  {
    glUniform4uiv( location, count, value );
  }

  void ClearBufferiv( GLenum buffer, GLint drawbuffer, const GLint* value ) override
  {
    glClearBufferiv( buffer, drawbuffer, value );
  }

  void ClearBufferuiv( GLenum buffer, GLint drawbuffer, const GLuint* value ) override
  {
    glClearBufferuiv( buffer, drawbuffer, value );
  }

  void ClearBufferfv( GLenum buffer, GLint drawbuffer, const GLfloat* value ) override
  {
    glClearBufferfv( buffer, drawbuffer, value );
  }

  void ClearBufferfi( GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil ) override
  {
    glClearBufferfi( buffer, drawbuffer, depth, stencil );
  }

  const GLubyte* GetStringi( GLenum name, GLuint index ) override
  {
    return glGetStringi( name, index );
  }

  void CopyBufferSubData( GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size ) override
  {
    glCopyBufferSubData( readTarget, writeTarget, readOffset, writeOffset, size );
  }

  void GetUniformIndices( GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices ) override
  {
    glGetUniformIndices( program, uniformCount, uniformNames, uniformIndices );
  }

  void GetActiveUniformsiv( GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params ) override
  {
    glGetActiveUniformsiv( program, uniformCount, uniformIndices, pname, params );
  }

  GLuint GetUniformBlockIndex( GLuint program, const GLchar* uniformBlockName ) override
  {
    return glGetUniformBlockIndex( program, uniformBlockName );
  }

  void GetActiveUniformBlockiv( GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params ) override
  {
    glGetActiveUniformBlockiv( program, uniformBlockIndex, pname, params );
  }

  void GetActiveUniformBlockName( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName ) override
  {
    glGetActiveUniformBlockName( program, uniformBlockIndex, bufSize, length, uniformBlockName );
  }

  void UniformBlockBinding( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding ) override
  {
    glUniformBlockBinding( program, uniformBlockIndex, uniformBlockBinding );
  }

  void DrawArraysInstanced( GLenum mode, GLint first, GLsizei count, GLsizei instanceCount ) override
  {
    glDrawArraysInstanced( mode, first, count, instanceCount );
  }

  void DrawElementsInstanced( GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount ) override
  {
    glDrawElementsInstanced( mode, count, type, indices, instanceCount );
  }

  GLsync FenceSync( GLenum condition, GLbitfield flags ) override
  {
    return glFenceSync( condition, flags );
  }

  GLboolean IsSync( GLsync sync ) override
  {
    return glIsSync( sync );
  }

  void DeleteSync( GLsync sync ) override
  {
    glDeleteSync( sync );
  }

  GLenum ClientWaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout ) override
  {
    return glClientWaitSync( sync, flags, timeout );
  }

  void WaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout ) override
  {
    glWaitSync( sync, flags, timeout );
  }

  void GetInteger64v( GLenum pname, GLint64* params ) override
  {
    glGetInteger64v( pname, params );
  }

  void GetSynciv( GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values ) override
  {
    glGetSynciv( sync, pname, bufSize, length, values );
  }

  void GetInteger64i_v( GLenum target, GLuint index, GLint64* data ) override
  {
    glGetInteger64i_v( target, index, data );
  }

  void GetBufferParameteri64v( GLenum target, GLenum pname, GLint64* params ) override
  {
    glGetBufferParameteri64v( target, pname, params );
  }

  void GenSamplers( GLsizei count, GLuint* samplers ) override
  {
    glGenSamplers( count, samplers );
  }

  void DeleteSamplers( GLsizei count, const GLuint* samplers ) override
  {
    glDeleteSamplers( count, samplers );
  }

  GLboolean IsSampler( GLuint sampler ) override
  {
    return glIsSampler( sampler );
  }

  void BindSampler( GLuint unit, GLuint sampler ) override
  {
    glBindSampler( unit, sampler );
  }

  void SamplerParameteri( GLuint sampler, GLenum pname, GLint param ) override
  {
    glSamplerParameteri( sampler, pname, param );
  }

  void SamplerParameteriv( GLuint sampler, GLenum pname, const GLint* param ) override
  {
    glSamplerParameteriv( sampler, pname, param );
  }

  void SamplerParameterf( GLuint sampler, GLenum pname, GLfloat param ) override
  {
    glSamplerParameterf( sampler, pname, param );
  }

  void SamplerParameterfv( GLuint sampler, GLenum pname, const GLfloat* param ) override
  {
    glSamplerParameterfv( sampler, pname, param );
  }

  void GetSamplerParameteriv( GLuint sampler, GLenum pname, GLint* params ) override
  {
    glGetSamplerParameteriv( sampler, pname, params );
  }

  void GetSamplerParameterfv( GLuint sampler, GLenum pname, GLfloat* params ) override
  {
    glGetSamplerParameterfv( sampler, pname, params );
  }

  void VertexAttribDivisor( GLuint index, GLuint divisor ) override
  {
    glVertexAttribDivisor( index, divisor );
  }

  void BindTransformFeedback( GLenum target, GLuint id ) override
  {
    glBindTransformFeedback( target, id );
  }

  void DeleteTransformFeedbacks( GLsizei n, const GLuint* ids ) override
  {
    glDeleteTransformFeedbacks( n, ids );
  }

  void GenTransformFeedbacks( GLsizei n, GLuint* ids ) override
  {
    glGenTransformFeedbacks( n, ids );
  }

  GLboolean IsTransformFeedback( GLuint id ) override
  {
    return glIsTransformFeedback( id );
  }

  void PauseTransformFeedback( void ) override
  {
    glPauseTransformFeedback();
  }

  void ResumeTransformFeedback( void ) override
  {
    glResumeTransformFeedback();
  }

  void GetProgramBinary( GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary ) override
  {
    glGetProgramBinary( program, bufSize, length, binaryFormat, binary );
  }

  void ProgramBinary( GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length ) override
  {
    glProgramBinary( program, binaryFormat, binary, length );
  }

  void ProgramParameteri( GLuint program, GLenum pname, GLint value ) override
  {
    glProgramParameteri( program, pname, value );
  }

  void InvalidateFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments ) override
  {
    glInvalidateFramebuffer( target, numAttachments, attachments );
  }

  void InvalidateSubFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height ) override
  {
    glInvalidateSubFramebuffer( target, numAttachments, attachments, x, y, width, height );
  }

  void TexStorage2D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height ) override
  {
    glTexStorage2D( target, levels, internalformat, width, height );
  }

  void TexStorage3D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth ) override
  {
    glTexStorage3D( target, levels, internalformat, width, height, depth );
  }

  void GetInternalformativ( GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params ) override
  {
    glGetInternalformativ( target, internalformat, pname, bufSize, params );
  }
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GLES3_IMPLEMENTATION_H
