#ifndef DALI_INTERNAL_GL_EXTENSION_H
#define DALI_INTERNAL_GL_EXTENSION_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <GLES3/gl3.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * GlExtensions class provides GL extensions support
 */
class GlExtensions
{
public:
  /**
   * Constructor
   */
  GlExtensions();

  /**
   * Destructor
   */
  ~GlExtensions();

public:
  /**
   * If the GL extension is available this function discards specified data in attachments
   * from being copied from the target to improve performance.
   *
   * Usage: GLenum attachments[] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
   *        DiscardFrameBufferEXT(GL_FRAMEBUFFER, 2, attachments);
   *
   * @param target is usually GL_FRAMEBUFFER
   * @param numAttachments is the count of attachments
   * @param attachments is a pointer to the attachments
   */
  void DiscardFrameBuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments);

  /**
   * GLES extension
   * Returns the program object's executable bytecode.
   * @param[in] program       The program object's name/id
   * @param[in] bufSize       The maximum number of bytes that may be written into binary
   * @param[out] length       The actual number of bytes written into binary
   * @param[out] binaryFormat The format of the program binary
   * @param[out] binary       The actual program bytecode
   */
  void GetProgramBinaryOES(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);

  /**
   * GLES extension
   * Loads a program object with a program binary previously returned from GetProgramBinaryOES
   * @param[in] program       The program object's name/id
   * @param[in] binaryFormat  The format of the program binary
   * @param[in] binary        The program bytecode
   * @param[in] length        The number of bytes in binary
   */
  void ProgramBinaryOES(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLint length);

  /**
   * KHR extension
   * Specify a boundary between passes when using advanced blend equations.
   */
  bool BlendBarrierKHR();

  /**
   * GLES extension
   * Establish data storage, format, dimensions and sample count of a renderbuffer object's image
   *
   * @param[in] target Specifies a binding to which the target of the allocation and must be GL_RENDERBUFFER.
   * @param[in] samples Specifies the number of samples to be used for the renderbuffer object's storage. Must not bigger than MAX_SAMPLES_EXT.
   * @param[in] internalformat Specifies the internal format to use for the renderbuffer object's image.
   * @param[in] width Specifies the width of the renderbuffer, in pixels.
   * @param[in] height Specifies the height of the renderbuffer, in pixels.
   */
  void RenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

  /**
   * GLES extension
   * Enables multisampled rendering into the images of a texture object.
   * If samples as 0, same as FramebufferTexture2D
   *
   * @param[in] target Specifies the framebuffer target. The symbolic constant must be GL_FRAMEBUFFER.
   * @param[in] attachment Specifies the attachment point to which an image from texture should be attached.
   * @param[in] textarget Specifies the texture target.
   * @param[in] texture Specifies the texture object whose image is to be attached.
   * @param[in] level Specifies the mipmap level of the texture image to be attached, which must be 0.
   * @param[in] samples The number of samples to the texture. Must not bigger than MAX_SAMPLES_EXT.
   */
  void FramebufferTexture2DMultisampleEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);

private:
  /**
   * Lazy Initialize extensions on first use
   */
  void Initialize();

#ifdef GL_EXT_discard_framebuffer
  PFNGLDISCARDFRAMEBUFFEREXTPROC mGlDiscardFramebuffer;
#endif

#ifdef GL_OES_get_program_binary
  PFNGLGETPROGRAMBINARYOESPROC mGlGetProgramBinaryOES;
  PFNGLPROGRAMBINARYOESPROC    mGlProgramBinaryOES;
#endif

#ifdef GL_KHR_blend_equation_advanced
  PFNGLBLENDBARRIERKHRPROC mBlendBarrierKHR;
#endif

#ifdef GL_EXT_multisampled_render_to_texture
  PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC  mGlRenderbufferStorageMultisampleEXT;
  PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC mGlFramebufferTexture2DMultisampleEXT;
#endif

  bool mInitialized;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif /* DALI_INTERNAL_GL_EXTENSION_H */
