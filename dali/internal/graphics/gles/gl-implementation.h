#ifndef DALI_INTERNAL_GL_IMPLEMENTATION_H
#define DALI_INTERNAL_GL_IMPLEMENTATION_H

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
#include <memory>
#include <cstdlib>
#include <GLES2/gl2.h>
#include <dali/integration-api/gl-abstraction.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/gles-abstraction.h>
#include <dali/internal/graphics/gles/gles2-implementation.h>
#include <dali/internal/graphics/gles/gles3-implementation.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * GlImplementation is a concrete implementation for GlAbstraction.
 * The class provides an OpenGL-ES 2.0 or 3.0 implementation.
 * The class is provided when creating the Integration::Core object.
 */
class GlImplementation : public Dali::Integration::GlAbstraction
{

public:
  GlImplementation()
    : mGlesVersion( 30 ),
      mIsSurfacelessContextSupported( false )
  {
    mImpl.reset( new Gles3Implementation() );
  }

  virtual ~GlImplementation() {}

  void PreRender()
  {
    /* Do nothing in main implementation */
  }

  void PostRender()
  {
    /* Do nothing in main implementation */
  }

  void SetGlesVersion( const int32_t glesVersion )
  {
    if( mGlesVersion != glesVersion )
    {
      mGlesVersion = glesVersion;
      if( mGlesVersion >= 30 )
      {
        mImpl.reset( new Gles3Implementation() );
      }
      else
      {
        mImpl.reset( new Gles2Implementation() );
      }
    }
  }

  void SetIsSurfacelessContextSupported( const bool isSupported )
  {
    mIsSurfacelessContextSupported = isSupported;
  }

  bool IsSurfacelessContextSupported() const
  {
    return mIsSurfacelessContextSupported;
  }

  bool TextureRequiresConverting( const GLenum imageGlFormat, const GLenum textureGlFormat, const bool isSubImage ) const
  {
    bool convert = ( ( imageGlFormat == GL_RGB ) && ( textureGlFormat == GL_RGBA ) );
    if( mGlesVersion >= 30 )
    {
      // Don't convert manually from RGB to RGBA if GLES >= 3.0 and a sub-image is uploaded.
      convert = ( convert && !isSubImage );
    }
    return convert;
  }

  /* OpenGL ES 2.0 */

  void ActiveTexture( GLenum texture )
  {
    glActiveTexture( texture );
  }

  void AttachShader( GLuint program, GLuint shader )
  {
    glAttachShader( program, shader );
  }

  void BindAttribLocation( GLuint program, GLuint index, const char* name )
  {
    glBindAttribLocation( program, index, name );
  }

  void BindBuffer( GLenum target, GLuint buffer )
  {
    glBindBuffer( target, buffer );
  }

  void BindFramebuffer( GLenum target, GLuint framebuffer )
  {
    glBindFramebuffer( target, framebuffer );
  }

  void BindRenderbuffer( GLenum target, GLuint renderbuffer )
  {
    glBindRenderbuffer( target, renderbuffer );
  }

  void BindTexture( GLenum target, GLuint texture )
  {
    glBindTexture( target, texture );
  }

  void BlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
  {
    glBlendColor( red, green, blue, alpha );
  }

  void BlendEquation( GLenum mode )
  {
    glBlendEquation( mode );
  }

  void BlendEquationSeparate( GLenum modeRGB, GLenum modeAlpha )
  {
    glBlendEquationSeparate( modeRGB, modeAlpha );
  }

  void BlendFunc( GLenum sfactor, GLenum dfactor )
  {
    glBlendFunc( sfactor, dfactor );
  }

  void BlendFuncSeparate( GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha )
  {
    glBlendFuncSeparate( srcRGB, dstRGB, srcAlpha, dstAlpha );
  }

  void BufferData( GLenum target, GLsizeiptr size, const void* data, GLenum usage )
  {
    glBufferData( target, size, data, usage );
  }

  void BufferSubData( GLenum target, GLintptr offset, GLsizeiptr size, const void* data )
  {
    glBufferSubData( target, offset, size, data );
  }

  GLenum CheckFramebufferStatus( GLenum target )
  {
    return glCheckFramebufferStatus( target );
  }

  void Clear( GLbitfield mask )
  {
    glClear( mask );
  }

  void ClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
  {
    glClearColor( red, green, blue, alpha );
  }

  void ClearDepthf( GLclampf depth )
  {
    glClearDepthf( depth );
  }

  void ClearStencil( GLint s )
  {
    glClearStencil( s );
  }

  void ColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
  {
    glColorMask( red, green, blue, alpha );
  }

  void CompileShader( GLuint shader )
  {
    glCompileShader( shader );
  }

  void CompressedTexImage2D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data )
  {
    glCompressedTexImage2D( target, level, internalformat, width, height, border, imageSize, data );
  }

  void CompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data )
  {
    glCompressedTexSubImage2D( target, level, xoffset, yoffset, width, height, format, imageSize, data );
  }

  void CopyTexImage2D( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border )
  {
    glCopyTexImage2D( target, level, internalformat, x, y, width, height, border );
  }

  void CopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
  {
    glCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height );
  }

  GLuint CreateProgram( void )
  {
    return glCreateProgram();
  }

  GLuint CreateShader( GLenum type )
  {
    return glCreateShader( type );
  }

  void CullFace( GLenum mode )
  {
    glCullFace( mode );
  }

  void DeleteBuffers( GLsizei n, const GLuint* buffers )
  {
    glDeleteBuffers( n, buffers );
  }

  void DeleteFramebuffers( GLsizei n, const GLuint* framebuffers )
  {
    glDeleteFramebuffers( n, framebuffers );
  }

  void DeleteProgram( GLuint program )
  {
    glDeleteProgram( program );
  }

  void DeleteRenderbuffers( GLsizei n, const GLuint* renderbuffers )
  {
    glDeleteRenderbuffers( n, renderbuffers );
  }

  void DeleteShader( GLuint shader )
  {
    glDeleteShader( shader );
  }

  void DeleteTextures( GLsizei n, const GLuint* textures )
  {
    glDeleteTextures( n, textures );
  }

  void DepthFunc( GLenum func )
  {
    glDepthFunc( func );
  }

  void DepthMask( GLboolean flag )
  {
    glDepthMask( flag );
  }

  void DepthRangef( GLclampf zNear, GLclampf zFar )
  {
    glDepthRangef( zNear, zFar );
  }

  void DetachShader( GLuint program, GLuint shader )
  {
    glDetachShader( program, shader );
  }

  void Disable( GLenum cap )
  {
    glDisable( cap );
  }

  void DisableVertexAttribArray( GLuint index )
  {
    glDisableVertexAttribArray( index );
  }

  void DrawArrays( GLenum mode, GLint first, GLsizei count )
  {
    glDrawArrays( mode, first, count );
  }

  void DrawElements( GLenum mode, GLsizei count, GLenum type, const void* indices )
  {
    glDrawElements( mode, count, type, indices );
  }

  void Enable( GLenum cap )
  {
    glEnable( cap );
  }

  void EnableVertexAttribArray( GLuint index )
  {
    glEnableVertexAttribArray( index );
  }

  void Finish( void )
  {
    glFinish();
  }

  void Flush( void )
  {
    glFlush();
  }

  void FramebufferRenderbuffer( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer )
  {
    glFramebufferRenderbuffer( target, attachment, renderbuffertarget, renderbuffer );
  }

  void FramebufferTexture2D( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level )
  {
    glFramebufferTexture2D( target, attachment, textarget, texture, level );
  }

  void FrontFace( GLenum mode )
  {
    glFrontFace( mode );
  }

  void GenBuffers( GLsizei n, GLuint* buffers )
  {
    glGenBuffers( n, buffers );
  }

  void GenerateMipmap( GLenum target )
  {
    glGenerateMipmap( target );
  }

  void GenFramebuffers( GLsizei n, GLuint* framebuffers )
  {
    glGenFramebuffers( n, framebuffers );
  }

  void GenRenderbuffers( GLsizei n, GLuint* renderbuffers )
  {
    glGenRenderbuffers( n, renderbuffers );
  }

  void GenTextures( GLsizei n, GLuint* textures )
  {
    glGenTextures( n, textures );
  }

  void GetActiveAttrib( GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name )
  {
    glGetActiveAttrib( program, index, bufsize, length, size, type, name );
  }

  void GetActiveUniform( GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name )
  {
    glGetActiveUniform( program, index, bufsize, length, size, type, name );
  }

  void GetAttachedShaders( GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders )
  {
    glGetAttachedShaders( program, maxcount, count, shaders );
  }

  int  GetAttribLocation( GLuint program, const char* name )
  {
    return glGetAttribLocation( program, name );
  }

  void GetBooleanv( GLenum pname, GLboolean* params )
  {
    glGetBooleanv( pname, params );
  }

  void GetBufferParameteriv( GLenum target, GLenum pname, GLint* params )
  {
    glGetBufferParameteriv( target, pname, params );
  }

  GLenum GetError( void )
  {
    return glGetError();
  }

  void GetFloatv( GLenum pname, GLfloat* params )
  {
    glGetFloatv( pname, params );
  }

  void GetFramebufferAttachmentParameteriv( GLenum target, GLenum attachment, GLenum pname, GLint* params )
  {
    glGetFramebufferAttachmentParameteriv( target, attachment, pname, params );
  }

  void GetIntegerv( GLenum pname, GLint* params )
  {
    glGetIntegerv( pname, params );
  }

  void GetProgramiv( GLuint program, GLenum pname, GLint* params )
  {
    glGetProgramiv( program, pname, params );
  }

  void GetProgramInfoLog( GLuint program, GLsizei bufsize, GLsizei* length, char* infolog )
  {
    glGetProgramInfoLog( program, bufsize, length, infolog );
  }

  void GetRenderbufferParameteriv( GLenum target, GLenum pname, GLint* params )
  {
    glGetRenderbufferParameteriv( target, pname, params );
  }

  void GetShaderiv( GLuint shader, GLenum pname, GLint* params )
  {
    glGetShaderiv( shader, pname, params );
  }

  void GetShaderInfoLog( GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog )
  {
    glGetShaderInfoLog( shader, bufsize, length, infolog );
  }

  void GetShaderPrecisionFormat( GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision )
  {
    glGetShaderPrecisionFormat( shadertype, precisiontype, range, precision );
  }

  void GetShaderSource( GLuint shader, GLsizei bufsize, GLsizei* length, char* source )
  {
    glGetShaderSource( shader, bufsize, length, source );
  }

  const GLubyte* GetString( GLenum name )
  {
    return glGetString( name );
  }

  void GetTexParameterfv( GLenum target, GLenum pname, GLfloat* params )
  {
    glGetTexParameterfv( target, pname, params );
  }

  void GetTexParameteriv( GLenum target, GLenum pname, GLint* params )
  {
    glGetTexParameteriv( target, pname, params );
  }

  void GetUniformfv( GLuint program, GLint location, GLfloat* params )
  {
    glGetUniformfv( program, location, params );
  }

  void GetUniformiv( GLuint program, GLint location, GLint* params )
  {
    glGetUniformiv( program, location, params );
  }

  int  GetUniformLocation( GLuint program, const char* name )
  {
    return glGetUniformLocation( program, name );
  }

  void GetVertexAttribfv( GLuint index, GLenum pname, GLfloat* params )
  {
    glGetVertexAttribfv( index, pname, params );
  }

  void GetVertexAttribiv( GLuint index, GLenum pname, GLint* params )
  {
    glGetVertexAttribiv( index, pname, params );
  }

  void GetVertexAttribPointerv( GLuint index, GLenum pname, void** pointer )
  {
    glGetVertexAttribPointerv( index, pname, pointer );
  }

  void Hint( GLenum target, GLenum mode )
  {
    glHint( target, mode );
  }

  GLboolean IsBuffer( GLuint buffer )
  {
    return glIsBuffer( buffer );
  }

  GLboolean IsEnabled( GLenum cap )
  {
    return glIsEnabled( cap );
  }

  GLboolean IsFramebuffer( GLuint framebuffer )
  {
    return glIsFramebuffer( framebuffer );
  }

  GLboolean IsProgram( GLuint program )
  {
    return glIsProgram( program );
  }

  GLboolean IsRenderbuffer( GLuint renderbuffer )
  {
    return glIsRenderbuffer( renderbuffer );
  }

  GLboolean IsShader( GLuint shader )
  {
    return glIsShader( shader );
  }

  GLboolean IsTexture( GLuint texture )
  {
    return glIsTexture( texture );
  }

  void LineWidth( GLfloat width )
  {
    glLineWidth( width );
  }

  void LinkProgram( GLuint program )
  {
    glLinkProgram( program );
  }

  void PixelStorei( GLenum pname, GLint param )
  {
    glPixelStorei( pname, param );
  }

  void PolygonOffset( GLfloat factor, GLfloat units )
  {
    glPolygonOffset( factor, units );
  }

  void ReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels )
  {
    glReadPixels( x, y, width, height, format, type, pixels );
  }

  void ReleaseShaderCompiler( void )
  {
    glReleaseShaderCompiler();
  }

  void RenderbufferStorage( GLenum target, GLenum internalformat, GLsizei width, GLsizei height )
  {
    glRenderbufferStorage( target, internalformat, width, height );
  }

  void SampleCoverage( GLclampf value, GLboolean invert )
  {
    glSampleCoverage( value, invert );
  }

  void Scissor( GLint x, GLint y, GLsizei width, GLsizei height )
  {
    glScissor( x, y, width, height );
  }

  void ShaderBinary( GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length )
  {
    glShaderBinary( n, shaders, binaryformat, binary, length );
  }

  void ShaderSource( GLuint shader, GLsizei count, const char** string, const GLint* length )
  {
    glShaderSource( shader, count, string, length );
  }

  void StencilFunc( GLenum func, GLint ref, GLuint mask )
  {
    glStencilFunc( func, ref, mask );
  }

  void StencilFuncSeparate( GLenum face, GLenum func, GLint ref, GLuint mask )
  {
    glStencilFuncSeparate( face, func, ref, mask );
  }

  void StencilMask( GLuint mask )
  {
    glStencilMask( mask );
  }

  void StencilMaskSeparate( GLenum face, GLuint mask )
  {
    glStencilMaskSeparate( face, mask );
  }

  void StencilOp( GLenum fail, GLenum zfail, GLenum zpass )
  {
    glStencilOp( fail, zfail, zpass );
  }

  void StencilOpSeparate( GLenum face, GLenum fail, GLenum zfail, GLenum zpass )
  {
    glStencilOpSeparate( face, fail, zfail, zpass );
  }

  void TexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels )
  {
    glTexImage2D( target, level, internalformat, width, height, border, format, type, pixels );
  }

  void TexParameterf( GLenum target, GLenum pname, GLfloat param )
  {
    glTexParameterf( target, pname, param );
  }

  void TexParameterfv( GLenum target, GLenum pname, const GLfloat* params )
  {
    glTexParameterfv( target, pname, params );
  }

  void TexParameteri( GLenum target, GLenum pname, GLint param )
  {
    glTexParameteri( target, pname, param );
  }

  void TexParameteriv( GLenum target, GLenum pname, const GLint* params )
  {
    glTexParameteriv( target, pname, params );
  }

  void TexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels )
  {
    glTexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, pixels );
  }

  void Uniform1f( GLint location, GLfloat x )
  {
    glUniform1f( location, x );
  }

  void Uniform1fv( GLint location, GLsizei count, const GLfloat* v )
  {
    glUniform1fv( location, count, v );
  }

  void Uniform1i( GLint location, GLint x )
  {
    glUniform1i( location, x );
  }

  void Uniform1iv( GLint location, GLsizei count, const GLint* v )
  {
    glUniform1iv( location, count, v );
  }

  void Uniform2f( GLint location, GLfloat x, GLfloat y )
  {
    glUniform2f( location, x, y );
  }

  void Uniform2fv( GLint location, GLsizei count, const GLfloat* v )
  {
    glUniform2fv( location, count, v );
  }

  void Uniform2i( GLint location, GLint x, GLint y )
  {
    glUniform2i( location, x, y );
  }

  void Uniform2iv( GLint location, GLsizei count, const GLint* v )
  {
    glUniform2iv( location, count, v );
  }

  void Uniform3f( GLint location, GLfloat x, GLfloat y, GLfloat z )
  {
    glUniform3f( location, x, y, z );
  }

  void Uniform3fv( GLint location, GLsizei count, const GLfloat* v )
  {
    glUniform3fv( location, count, v );
  }

  void Uniform3i( GLint location, GLint x, GLint y, GLint z )
  {
    glUniform3i( location, x, y, z );
  }

  void Uniform3iv( GLint location, GLsizei count, const GLint* v )
  {
    glUniform3iv( location, count, v );
  }

  void Uniform4f( GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w )
  {
    glUniform4f( location, x, y, z, w );
  }

  void Uniform4fv( GLint location, GLsizei count, const GLfloat* v )
  {
    glUniform4fv( location, count, v );
  }

  void Uniform4i( GLint location, GLint x, GLint y, GLint z, GLint w )
  {
    glUniform4i( location, x, y, z, w );
  }

  void Uniform4iv( GLint location, GLsizei count, const GLint* v )
  {
    glUniform4iv( location, count, v );
  }

  void UniformMatrix2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value )
  {
    glUniformMatrix2fv( location, count, transpose, value );
  }

  void UniformMatrix3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value )
  {
    glUniformMatrix3fv( location, count, transpose, value );
  }

  void UniformMatrix4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value )
  {
    glUniformMatrix4fv( location, count, transpose, value );
  }

  void UseProgram( GLuint program )
  {
    glUseProgram( program );
  }

  void ValidateProgram( GLuint program )
  {
    glValidateProgram( program );
  }

  void VertexAttrib1f( GLuint indx, GLfloat x )
  {
    glVertexAttrib1f( indx, x );
  }

  void VertexAttrib1fv( GLuint indx, const GLfloat* values )
  {
    glVertexAttrib1fv( indx, values );
  }

  void VertexAttrib2f( GLuint indx, GLfloat x, GLfloat y )
  {
    glVertexAttrib2f( indx, x, y );
  }

  void VertexAttrib2fv( GLuint indx, const GLfloat* values )
  {
    glVertexAttrib2fv( indx, values );
  }

  void VertexAttrib3f( GLuint indx, GLfloat x, GLfloat y, GLfloat z )
  {
    glVertexAttrib3f( indx, x, y, z );
  }

  void VertexAttrib3fv( GLuint indx, const GLfloat* values )
  {
    glVertexAttrib3fv( indx, values );
  }

  void VertexAttrib4f( GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w )
  {
    glVertexAttrib4f( indx, x, y, z, w );
  }

  void VertexAttrib4fv( GLuint indx, const GLfloat* values )
  {
    glVertexAttrib4fv( indx, values );
  }

  void VertexAttribPointer( GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr )
  {
    glVertexAttribPointer( indx, size, type, normalized, stride, ptr );
  }

  void Viewport( GLint x, GLint y, GLsizei width, GLsizei height )
  {
    glViewport( x, y, width, height );
  }

  /* OpenGL ES 3.0 */

  void ReadBuffer( GLenum mode )
  {
    mImpl->ReadBuffer( mode );
  }

  void DrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices )
  {
    mImpl->DrawRangeElements( mode, start, end, count, type, indices );
  }

  void TexImage3D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels )
  {
    mImpl->TexImage3D( target, level, internalformat, width, height, depth, border, format, type, pixels );
  }

  void TexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels )
  {
    mImpl->TexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels );
  }

  void CopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height )
  {
    mImpl->CopyTexSubImage3D( target, level, xoffset, yoffset, zoffset, x, y, width, height );
  }

  void CompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data )
  {
    mImpl->CompressedTexImage3D( target, level, internalformat, width, height, depth, border, imageSize, data );
  }

  void CompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data )
  {
    mImpl->CompressedTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data );
  }

  void GenQueries( GLsizei n, GLuint* ids )
  {
    mImpl->GenQueries( n, ids );
  }

  void DeleteQueries( GLsizei n, const GLuint* ids )
  {
    mImpl->DeleteQueries( n, ids );
  }

  GLboolean IsQuery( GLuint id )
  {
    return mImpl->IsQuery( id );
  }

  void BeginQuery( GLenum target, GLuint id )
  {
    mImpl->BeginQuery( target, id );
  }

  void EndQuery( GLenum target )
  {
    mImpl->EndQuery( target );
  }

  void GetQueryiv( GLenum target, GLenum pname, GLint* params )
  {
    mImpl->GetQueryiv( target, pname, params );
  }

  void GetQueryObjectuiv( GLuint id, GLenum pname, GLuint* params )
  {
    mImpl->GetQueryObjectuiv( id, pname, params );
  }

  GLboolean UnmapBuffer( GLenum target )
  {
    return mImpl->UnmapBuffer( target );
  }

  void GetBufferPointerv( GLenum target, GLenum pname, GLvoid** params )
  {
    mImpl->GetBufferPointerv( target, pname, params );
  }

  void DrawBuffers( GLsizei n, const GLenum* bufs )
  {
    mImpl->DrawBuffers( n, bufs );
  }

  void UniformMatrix2x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value )
  {
    mImpl->UniformMatrix2x3fv( location, count, transpose, value );
  }

  void UniformMatrix3x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value )
  {
    mImpl->UniformMatrix3x2fv( location, count, transpose, value );
  }

  void UniformMatrix2x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value )
  {
    mImpl->UniformMatrix2x4fv( location, count, transpose, value );
  }

  void UniformMatrix4x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value )
  {
    mImpl->UniformMatrix4x2fv( location, count, transpose, value );
  }

  void UniformMatrix3x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value )
  {
    mImpl->UniformMatrix3x4fv( location, count, transpose, value );
  }

  void UniformMatrix4x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value )
  {
    mImpl->UniformMatrix4x3fv( location, count, transpose, value );
  }

  void BlitFramebuffer( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter )
  {
    mImpl->BlitFramebuffer( srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter );
  }

  void RenderbufferStorageMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height )
  {
    mImpl->RenderbufferStorageMultisample( target, samples, internalformat, width, height );
  }

  void FramebufferTextureLayer( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer )
  {
    mImpl->FramebufferTextureLayer( target, attachment, texture, level, layer );
  }

  GLvoid* MapBufferRange( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access )
  {
    return mImpl->MapBufferRange( target, offset, length, access );
  }

  void FlushMappedBufferRange( GLenum target, GLintptr offset, GLsizeiptr length )
  {
    mImpl->FlushMappedBufferRange( target, offset, length );
  }

  void BindVertexArray( GLuint array )
  {
    mImpl->BindVertexArray( array );
  }

  void DeleteVertexArrays( GLsizei n, const GLuint* arrays )
  {
    mImpl->DeleteVertexArrays( n, arrays );
  }

  void GenVertexArrays( GLsizei n, GLuint* arrays )
  {
    mImpl->GenVertexArrays( n, arrays );
  }

  GLboolean IsVertexArray( GLuint array )
  {
    return mImpl->IsVertexArray( array );
  }

  void GetIntegeri_v( GLenum target, GLuint index, GLint* data )
  {
    mImpl->GetIntegeri_v( target, index, data );
  }

  void BeginTransformFeedback( GLenum primitiveMode )
  {
    mImpl->BeginTransformFeedback( primitiveMode );
  }

  void EndTransformFeedback( void )
  {
    mImpl->EndTransformFeedback();
  }

  void BindBufferRange( GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size )
  {
    mImpl->BindBufferRange( target, index, buffer, offset, size );
  }

  void BindBufferBase( GLenum target, GLuint index, GLuint buffer )
  {
    mImpl->BindBufferBase( target, index, buffer );
  }

  void TransformFeedbackVaryings( GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode )
  {
    mImpl->TransformFeedbackVaryings( program, count, varyings, bufferMode );
  }

  void GetTransformFeedbackVarying( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name )
  {
    mImpl->GetTransformFeedbackVarying( program, index, bufSize, length, size, type, name );
  }

  void VertexAttribIPointer( GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer )
  {
    mImpl->VertexAttribIPointer( index, size, type, stride, pointer );
  }

  void GetVertexAttribIiv( GLuint index, GLenum pname, GLint* params )
  {
    mImpl->GetVertexAttribIiv( index, pname, params );
  }

  void GetVertexAttribIuiv( GLuint index, GLenum pname, GLuint* params )
  {
    mImpl->GetVertexAttribIuiv( index, pname, params );
  }

  void VertexAttribI4i( GLuint index, GLint x, GLint y, GLint z, GLint w )
  {
    mImpl->VertexAttribI4i( index, x, y, z, w );
  }

  void VertexAttribI4ui( GLuint index, GLuint x, GLuint y, GLuint z, GLuint w )
  {
    mImpl->VertexAttribI4ui( index, x, y, z, w );
  }

  void VertexAttribI4iv( GLuint index, const GLint* v )
  {
    mImpl->VertexAttribI4iv( index, v );
  }

  void VertexAttribI4uiv( GLuint index, const GLuint* v )
  {
    mImpl->VertexAttribI4uiv( index, v );
  }

  void GetUniformuiv( GLuint program, GLint location, GLuint* params )
  {
    mImpl->GetUniformuiv( program, location, params );
  }

  GLint GetFragDataLocation( GLuint program, const GLchar *name )
  {
    return mImpl->GetFragDataLocation( program, name );
  }

  void Uniform1ui( GLint location, GLuint v0 )
  {
    mImpl->Uniform1ui( location, v0 );
  }

  void Uniform2ui( GLint location, GLuint v0, GLuint v1 )
  {
    mImpl->Uniform2ui( location, v0, v1 );
  }

  void Uniform3ui( GLint location, GLuint v0, GLuint v1, GLuint v2 )
  {
    mImpl->Uniform3ui( location, v0, v1, v2 );
  }

  void Uniform4ui( GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3 )
  {
    mImpl->Uniform4ui( location, v0, v1, v2, v3 );
  }

  void Uniform1uiv( GLint location, GLsizei count, const GLuint* value )
  {
    mImpl->Uniform1uiv( location, count, value );
  }

  void Uniform2uiv( GLint location, GLsizei count, const GLuint* value )
  {
    mImpl->Uniform2uiv( location, count, value );
  }

  void Uniform3uiv( GLint location, GLsizei count, const GLuint* value )
  {
    mImpl->Uniform3uiv( location, count, value );
  }

  void Uniform4uiv( GLint location, GLsizei count, const GLuint* value )
  {
    mImpl->Uniform4uiv( location, count, value );
  }

  void ClearBufferiv( GLenum buffer, GLint drawbuffer, const GLint* value )
  {
    mImpl->ClearBufferiv( buffer, drawbuffer, value );
  }

  void ClearBufferuiv( GLenum buffer, GLint drawbuffer, const GLuint* value )
  {
    mImpl->ClearBufferuiv( buffer, drawbuffer, value );
  }

  void ClearBufferfv( GLenum buffer, GLint drawbuffer, const GLfloat* value )
  {
    mImpl->ClearBufferfv( buffer, drawbuffer, value );
  }

  void ClearBufferfi( GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil )
  {
    mImpl->ClearBufferfi( buffer, drawbuffer, depth, stencil );
  }

  const GLubyte* GetStringi( GLenum name, GLuint index )
  {
    return mImpl->GetStringi( name, index );
  }

  void CopyBufferSubData( GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size )
  {
    mImpl->CopyBufferSubData( readTarget, writeTarget, readOffset, writeOffset, size );
  }

  void GetUniformIndices( GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices )
  {
    mImpl->GetUniformIndices( program, uniformCount, uniformNames, uniformIndices );
  }

  void GetActiveUniformsiv( GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params )
  {
    mImpl->GetActiveUniformsiv( program, uniformCount, uniformIndices, pname, params );
  }

  GLuint GetUniformBlockIndex( GLuint program, const GLchar* uniformBlockName )
  {
    return mImpl->GetUniformBlockIndex( program, uniformBlockName );
  }

  void GetActiveUniformBlockiv( GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params )
  {
    mImpl->GetActiveUniformBlockiv( program, uniformBlockIndex, pname, params );
  }

  void GetActiveUniformBlockName( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName )
  {
    mImpl->GetActiveUniformBlockName( program, uniformBlockIndex, bufSize, length, uniformBlockName );
  }

  void UniformBlockBinding( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding )
  {
    mImpl->UniformBlockBinding( program, uniformBlockIndex, uniformBlockBinding );
  }

  void DrawArraysInstanced( GLenum mode, GLint first, GLsizei count, GLsizei instanceCount )
  {
    mImpl->DrawArraysInstanced( mode, first, count, instanceCount );
  }

  void DrawElementsInstanced( GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount )
  {
    mImpl->DrawElementsInstanced( mode, count, type, indices, instanceCount );
  }

  GLsync FenceSync( GLenum condition, GLbitfield flags )
  {
    return mImpl->FenceSync( condition, flags );
  }

  GLboolean IsSync( GLsync sync )
  {
    return mImpl->IsSync( sync );
  }

  void DeleteSync( GLsync sync )
  {
    mImpl->DeleteSync( sync );
  }

  GLenum ClientWaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout )
  {
    return mImpl->ClientWaitSync( sync, flags, timeout );
  }

  void WaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout )
  {
    mImpl->WaitSync( sync, flags, timeout );
  }

  void GetInteger64v( GLenum pname, GLint64* params )
  {
    mImpl->GetInteger64v( pname, params );
  }

  void GetSynciv( GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values )
  {
    mImpl->GetSynciv( sync, pname, bufSize, length, values );
  }

  void GetInteger64i_v( GLenum target, GLuint index, GLint64* data )
  {
    mImpl->GetInteger64i_v( target, index, data );
  }

  void GetBufferParameteri64v( GLenum target, GLenum pname, GLint64* params )
  {
    mImpl->GetBufferParameteri64v( target, pname, params );
  }

  void GenSamplers( GLsizei count, GLuint* samplers )
  {
    mImpl->GenSamplers( count, samplers );
  }

  void DeleteSamplers( GLsizei count, const GLuint* samplers )
  {
    mImpl->DeleteSamplers( count, samplers );
  }

  GLboolean IsSampler( GLuint sampler )
  {
    return mImpl->IsSampler( sampler );
  }

  void BindSampler( GLuint unit, GLuint sampler )
  {
    mImpl->BindSampler( unit, sampler );
  }

  void SamplerParameteri( GLuint sampler, GLenum pname, GLint param )
  {
    mImpl->SamplerParameteri( sampler, pname, param );
  }

  void SamplerParameteriv( GLuint sampler, GLenum pname, const GLint* param )
  {
    mImpl->SamplerParameteriv( sampler, pname, param );
  }

  void SamplerParameterf( GLuint sampler, GLenum pname, GLfloat param )
  {
    mImpl->SamplerParameterf( sampler, pname, param );
  }

  void SamplerParameterfv( GLuint sampler, GLenum pname, const GLfloat* param )
  {
    mImpl->SamplerParameterfv( sampler, pname, param );
  }

  void GetSamplerParameteriv( GLuint sampler, GLenum pname, GLint* params )
  {
    mImpl->GetSamplerParameteriv( sampler, pname, params );
  }

  void GetSamplerParameterfv( GLuint sampler, GLenum pname, GLfloat* params )
  {
    mImpl->GetSamplerParameterfv( sampler, pname, params );
  }

  void VertexAttribDivisor( GLuint index, GLuint divisor )
  {
    mImpl->VertexAttribDivisor( index, divisor );
  }

  void BindTransformFeedback( GLenum target, GLuint id )
  {
    mImpl->BindTransformFeedback( target, id );
  }

  void DeleteTransformFeedbacks( GLsizei n, const GLuint* ids )
  {
    mImpl->DeleteTransformFeedbacks( n, ids );
  }

  void GenTransformFeedbacks( GLsizei n, GLuint* ids )
  {
    mImpl->GenTransformFeedbacks( n, ids );
  }

  GLboolean IsTransformFeedback( GLuint id )
  {
    return mImpl->IsTransformFeedback( id );
  }

  void PauseTransformFeedback( void )
  {
    mImpl->PauseTransformFeedback();
  }

  void ResumeTransformFeedback( void )
  {
    mImpl->ResumeTransformFeedback();
  }

  void GetProgramBinary( GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary )
  {
    mImpl->GetProgramBinary( program, bufSize, length, binaryFormat, binary );
  }

  void ProgramBinary( GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length )
  {
    mImpl->ProgramBinary( program, binaryFormat, binary, length );
  }

  void ProgramParameteri( GLuint program, GLenum pname, GLint value )
  {
    mImpl->ProgramParameteri( program, pname, value );
  }

  void InvalidateFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments )
  {
    mImpl->InvalidateFramebuffer( target, numAttachments, attachments );
  }

  void InvalidateSubFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height )
  {
    mImpl->InvalidateSubFramebuffer( target, numAttachments, attachments, x, y, width, height );
  }

  void TexStorage2D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height )
  {
    mImpl->TexStorage2D( target, levels, internalformat, width, height );
  }

  void TexStorage3D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth )
  {
    mImpl->TexStorage3D( target, levels, internalformat, width, height, depth );
  }

  void GetInternalformativ( GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params )
  {
    mImpl->GetInternalformativ( target, internalformat, pname, bufSize, params );
  }

private:
  int32_t mGlesVersion;
  bool mIsSurfacelessContextSupported;
  std::unique_ptr<GlesAbstraction> mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GL_IMPLEMENTATION_H
