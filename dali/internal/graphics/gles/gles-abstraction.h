#ifndef DALI_INTERNAL_GLES_ABSTRACTION_H
#define DALI_INTERNAL_GLES_ABSTRACTION_H

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

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class GlesAbstraction
{

public:

  /**
   * Destructor
   */
  virtual ~GlesAbstraction() {};

  virtual void ReadBuffer( GLenum mode ) = 0;

  virtual void DrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices ) = 0;

  virtual void TexImage3D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels ) = 0;

  virtual void TexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels ) = 0;

  virtual void CopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height ) = 0;

  virtual void CompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data ) = 0;

  virtual void CompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data ) = 0;

  virtual void GenQueries( GLsizei n, GLuint* ids ) = 0;

  virtual void DeleteQueries( GLsizei n, const GLuint* ids ) = 0;

  virtual GLboolean IsQuery( GLuint id ) = 0;

  virtual void BeginQuery( GLenum target, GLuint id ) = 0;

  virtual void EndQuery( GLenum target ) = 0;

  virtual void GetQueryiv( GLenum target, GLenum pname, GLint* params ) = 0;

  virtual void GetQueryObjectuiv( GLuint id, GLenum pname, GLuint* params ) = 0;

  virtual GLboolean UnmapBuffer( GLenum target ) = 0;

  virtual void GetBufferPointerv( GLenum target, GLenum pname, GLvoid** params ) = 0;

  virtual void DrawBuffers( GLsizei n, const GLenum* bufs ) = 0;

  virtual void UniformMatrix2x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) = 0;

  virtual void UniformMatrix3x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) = 0;

  virtual void UniformMatrix2x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) = 0;

  virtual void UniformMatrix4x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) = 0;

  virtual void UniformMatrix3x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) = 0;

  virtual void UniformMatrix4x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) = 0;

  virtual void BlitFramebuffer( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter ) = 0;

  virtual void RenderbufferStorageMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ) = 0;

  virtual void FramebufferTextureLayer( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer ) = 0;

  virtual GLvoid* MapBufferRange( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access ) = 0;

  virtual void FlushMappedBufferRange( GLenum target, GLintptr offset, GLsizeiptr length ) = 0;

  virtual void BindVertexArray( GLuint array ) = 0;

  virtual void DeleteVertexArrays( GLsizei n, const GLuint* arrays ) = 0;

  virtual void GenVertexArrays( GLsizei n, GLuint* arrays ) = 0;

  virtual GLboolean IsVertexArray( GLuint array ) = 0;

  virtual void GetIntegeri_v( GLenum target, GLuint index, GLint* data ) = 0;

  virtual void BeginTransformFeedback( GLenum primitiveMode ) = 0;

  virtual void EndTransformFeedback( void ) = 0;

  virtual void BindBufferRange( GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size ) = 0;

  virtual void BindBufferBase( GLenum target, GLuint index, GLuint buffer ) = 0;

  virtual void TransformFeedbackVaryings( GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode ) = 0;

  virtual void GetTransformFeedbackVarying( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name ) = 0;

  virtual void VertexAttribIPointer( GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer ) = 0;

  virtual void GetVertexAttribIiv( GLuint index, GLenum pname, GLint* params ) = 0;

  virtual void GetVertexAttribIuiv( GLuint index, GLenum pname, GLuint* params ) = 0;

  virtual void VertexAttribI4i( GLuint index, GLint x, GLint y, GLint z, GLint w ) = 0;

  virtual void VertexAttribI4ui( GLuint index, GLuint x, GLuint y, GLuint z, GLuint w ) = 0;

  virtual void VertexAttribI4iv( GLuint index, const GLint* v ) = 0;

  virtual void VertexAttribI4uiv( GLuint index, const GLuint* v ) = 0;

  virtual void GetUniformuiv( GLuint program, GLint location, GLuint* params ) = 0;

  virtual GLint GetFragDataLocation( GLuint program, const GLchar *name ) = 0;

  virtual void Uniform1ui( GLint location, GLuint v0 ) = 0;

  virtual void Uniform2ui( GLint location, GLuint v0, GLuint v1 ) = 0;

  virtual void Uniform3ui( GLint location, GLuint v0, GLuint v1, GLuint v2 ) = 0;

  virtual void Uniform4ui( GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3 ) = 0;

  virtual void Uniform1uiv( GLint location, GLsizei count, const GLuint* value ) = 0;

  virtual void Uniform2uiv( GLint location, GLsizei count, const GLuint* value ) = 0;

  virtual void Uniform3uiv( GLint location, GLsizei count, const GLuint* value ) = 0;

  virtual void Uniform4uiv( GLint location, GLsizei count, const GLuint* value ) = 0;

  virtual void ClearBufferiv( GLenum buffer, GLint drawbuffer, const GLint* value ) = 0;

  virtual void ClearBufferuiv( GLenum buffer, GLint drawbuffer, const GLuint* value ) = 0;

  virtual void ClearBufferfv( GLenum buffer, GLint drawbuffer, const GLfloat* value ) = 0;

  virtual void ClearBufferfi( GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil ) = 0;

  virtual const GLubyte* GetStringi( GLenum name, GLuint index ) = 0;

  virtual void CopyBufferSubData( GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size ) = 0;

  virtual void GetUniformIndices( GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices ) = 0;

  virtual void GetActiveUniformsiv( GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params ) = 0;

  virtual GLuint GetUniformBlockIndex( GLuint program, const GLchar* uniformBlockName ) = 0;

  virtual void GetActiveUniformBlockiv( GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params ) = 0;

  virtual void GetActiveUniformBlockName( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName ) = 0;

  virtual void UniformBlockBinding( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding ) = 0;

  virtual void DrawArraysInstanced( GLenum mode, GLint first, GLsizei count, GLsizei instanceCount ) = 0;

  virtual void DrawElementsInstanced( GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount ) = 0;

  virtual GLsync FenceSync( GLenum condition, GLbitfield flags ) = 0;

  virtual GLboolean IsSync( GLsync sync ) = 0;

  virtual void DeleteSync( GLsync sync ) = 0;

  virtual GLenum ClientWaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout ) = 0;

  virtual void WaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout ) = 0;

  virtual void GetInteger64v( GLenum pname, GLint64* params ) = 0;

  virtual void GetSynciv( GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values ) = 0;

  virtual void GetInteger64i_v( GLenum target, GLuint index, GLint64* data ) = 0;

  virtual void GetBufferParameteri64v( GLenum target, GLenum pname, GLint64* params ) = 0;

  virtual void GenSamplers( GLsizei count, GLuint* samplers ) = 0;

  virtual void DeleteSamplers( GLsizei count, const GLuint* samplers ) = 0;

  virtual GLboolean IsSampler( GLuint sampler ) = 0;

  virtual void BindSampler( GLuint unit, GLuint sampler ) = 0;

  virtual void SamplerParameteri( GLuint sampler, GLenum pname, GLint param ) = 0;

  virtual void SamplerParameteriv( GLuint sampler, GLenum pname, const GLint* param ) = 0;

  virtual void SamplerParameterf( GLuint sampler, GLenum pname, GLfloat param ) = 0;

  virtual void SamplerParameterfv( GLuint sampler, GLenum pname, const GLfloat* param ) = 0;

  virtual void GetSamplerParameteriv( GLuint sampler, GLenum pname, GLint* params ) = 0;

  virtual void GetSamplerParameterfv( GLuint sampler, GLenum pname, GLfloat* params ) = 0;

  virtual void VertexAttribDivisor( GLuint index, GLuint divisor ) = 0;

  virtual void BindTransformFeedback( GLenum target, GLuint id ) = 0;

  virtual void DeleteTransformFeedbacks( GLsizei n, const GLuint* ids ) = 0;

  virtual void GenTransformFeedbacks( GLsizei n, GLuint* ids ) = 0;

  virtual GLboolean IsTransformFeedback( GLuint id ) = 0;

  virtual void PauseTransformFeedback( void ) = 0;

  virtual void ResumeTransformFeedback( void ) = 0;

  virtual void GetProgramBinary( GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary ) = 0;

  virtual void ProgramBinary( GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length ) = 0;

  virtual void ProgramParameteri( GLuint program, GLenum pname, GLint value ) = 0;

  virtual void InvalidateFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments ) = 0;

  virtual void InvalidateSubFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height ) = 0;

  virtual void TexStorage2D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height ) = 0;

  virtual void TexStorage3D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth ) = 0;

  virtual void GetInternalformativ( GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params ) = 0;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GLES_ABSTRACTION_H
