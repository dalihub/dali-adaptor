#ifndef DALI_INTERNAL_EGL_IMAGE_EXTENSIONS_LINUX_DMA_BUF_H
#define DALI_INTERNAL_EGL_IMAGE_EXTENSIONS_LINUX_DMA_BUF_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-namespace.h>
#include <cstdint>
#include <string>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
class EglImplementation;

/**
 * Describes a single-plane Linux dma_buf so it can be imported as an EGLImage.
 */
struct DmaBufDescriptor
{
  int      fd{-1};    ///< The dma_buf file descriptor, still owned by the caller
  uint32_t width{0};  ///< Width in pixels
  uint32_t height{0}; ///< Height in pixels
  uint32_t fourcc{0}; ///< DRM_FORMAT_* fourcc describing the pixel layout
  uint32_t stride{0}; ///< Bytes per row, which may exceed width times the pixel size
  uint32_t offset{0}; ///< Byte offset of the first pixel within the dma_buf
};

/**
 * The EGL image extensions as reached through a Linux dma_buf.
 *
 * Imports memory the CPU can write, rather than a window-system type, so it
 * needs a target and an attribute list that EglImageExtensions does not take.
 * The image is bound to GL_TEXTURE_EXTERNAL_OES, not GL_TEXTURE_2D, so callers
 * must report that target from NativeImageInterface::GetTextureTarget().
 *
 * All static: nothing is held beyond the entry points resolved on first use, so
 * callers pass the EGL implementation whose display the images belong to.
 *
 * @note Render thread only, except GetRenderNodePath(): it reads what
 *       OnDisplayInitialized() remembered, so a producer thread can ask.
 */
class EglImageExtensionsLinuxDmaBuf
{
public:
  /**
   * @brief Whether dma_buf import is available at all.
   *
   * False where the EGL implementation lacks EGL_EXT_image_dma_buf_import, in
   * which case nothing else here does anything.
   *
   * @param[in] impl The EGL implementation to query
   * @return True if dma_bufs can be imported
   */
  static bool IsSupported(EglImplementation& impl);

  /**
   * @brief Called once the EGL display is available.
   *
   * Resolves and remembers the DRM render node EGL renders on, which nothing can
   * be asked about before there is a display. dma_bufs have to be allocated on
   * that device: a machine with more than one GPU offers several render nodes,
   * and guessing picks the wrong one.
   *
   * @param[in] impl The EGL implementation to query, which must have a display
   *
   * @note Render thread only.
   */
  static void OnDisplayInitialized(EglImplementation& impl);

  /**
   * @brief Returns what OnDisplayInitialized() found.
   *
   * Reads the remembered answer rather than asking EGL, so it is safe to call
   * from a producer thread that has no EGL of its own.
   *
   * @return The render node path, or an empty string if it is not known yet or
   *         EGL would not say - which includes having no DRM device at all
   */
  static std::string GetRenderNodePath();

  /**
   * @brief Imports a dma_buf as an EGLImage.
   *
   * @param[in] impl The EGL implementation whose display the image belongs to
   * @param[in] descriptor Describes the dma_buf to import
   * @return An object holding an EGLImageKHR, or nullptr on failure
   */
  static void* CreateImage(EglImplementation& impl, const DmaBufDescriptor& descriptor);

  /**
   * @brief Destroys an image returned by CreateImage().
   *
   * @param[in] impl The EGL implementation the image was created against
   * @param[in] eglImage The image to destroy
   */
  static void DestroyImage(EglImplementation& impl, void* eglImage);

  /**
   * @brief Attaches an image to the bound external texture.
   *
   * The caller must have GL_TEXTURE_EXTERNAL_OES bound, and must report that
   * target from NativeImageInterface::GetTextureTarget() so the two agree.
   *
   * @param[in] eglImage The image to attach
   */
  static void TargetExternalTexture(void* eglImage);

  EglImageExtensionsLinuxDmaBuf()                                                = delete;
  EglImageExtensionsLinuxDmaBuf(const EglImageExtensionsLinuxDmaBuf&)            = delete;
  EglImageExtensionsLinuxDmaBuf& operator=(const EglImageExtensionsLinuxDmaBuf&) = delete;
};

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_EGL_IMAGE_EXTENSIONS_LINUX_DMA_BUF_H
