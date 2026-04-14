#ifndef DALI_GLES_TEXTURE_DEPENDENCY_CHECKER_H
#define DALI_GLES_TEXTURE_DEPENDENCY_CHECKER_H

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
 */

// EXTERNAL INCLUDES
#include <cstddef>
#include <cstdint>
#include <unordered_map>

// INTERNAL INCLUDES
#include <dali/devel-api/common/vector-wrapper.h>

namespace Dali::Graphics
{
class EglGraphicsController;

namespace GLES
{
class Context;
class Framebuffer;
class Texture;

/**
 * Class to handle dependency checks between textures on different
 * GL Contexts.
 *
 * We have a shared (resource) context for writing to offscreen framebuffers,
 * and separate contexts for each window/scene.
 * If a framebuffer attachment is used in a scene, then it needs a sync point
 * in the GPU in order to ensure that the first context finishes writing to the
 * texture before it is read in the scene context.
 *
 * Additionally, this class handles synchronization for the case where a texture
 * is read on one context and then written to on another context in a subsequent frame.
 * This prevents flickering issues with FBOs when:
 * - Context A writes to FBO (texture T)
 * - Context B reads texture T and swaps buffer
 * - Context A writes to FBO (texture T) again - needs to wait for B's read to complete
 */
class TextureDependencyChecker
{
public:
  explicit TextureDependencyChecker(EglGraphicsController& controller)
  : mController(controller)
  {
  }

  ~TextureDependencyChecker();

  /**
   * Clear all the textures. Call at the start of a frame
   */
  void Reset();

  /**
   * @brief Remove texture from all dependency lists.
   * @param[in] texture The texture to discard
   */
  void DiscardTexture(const Texture* texture);

public: ///< FBO write -> Surface read synchronization (Forward)
  /**
   * Add Texture dependencies
   *
   * @param[in] writeContext The context of the framebuffer's render pass
   * @param[in] framebuffer The framebuffer to collect textures from
   */
  void AddTextures(const Context* writeContext, const Framebuffer* framebuffer);

  /**
   * Check if the given texture needs syncing before being read.  This
   * will perform either a glWaitSync() (GPU side semaphore), or a
   * glClientWaitSync(CPU fence) if the texture needs syncing.
   * @param[in] readContext The context that the texture is being read (drawn with)
   * @param[in] texture The texture being read
   * @param[in] cpuSync True if glClientWaitSync should be used instead of glWaitSync
   */
  void CheckNeedsSync(const Context* readContext, const Texture* texture, bool cpuSync = false);

  /**
   * Get the number of (offscreen) textures for dependency checking
   */
  size_t GetFramebufferTextureCount() const
  {
    return mForwardDependencies.size();
  }

public: ///< Surface read -> FBO write next frame synchronization (Backward)
  /**
   * @brief Records textures that were read during this frame and creates a sync object.
   * Call this at the end of rendering to a window surface (before eglSwapBuffer).
   * The sync ensures subsequent FBO writes to these textures wait for the read to complete.
   * @param[in] readContext The context that read the textures
   * @return True if sync object created.
   */
  bool MarkFramebufferTextureRead(const Context* readContext);

  /**
   * @brief Checks if FBO attachments were read in previous frames (or current frame at another window surface), and waits on syncs if needed.
   * Call this before writing to an FBO to ensure any prior reads have completed.
   * @param[in] writeContext The context writing to the framebuffer
   * @param[in] framebuffer The framebuffer being written to
   */
  void CheckFramebufferNeedsSync(const Context* writeContext, const Framebuffer* framebuffer);

public: ///< NativeTexture dependency checker
  /**
   * @brief Add prepared native image texture to dependency list
   */
  void MarkNativeTexturePrepared(const Context* context, const Framebuffer* framebuffer, const Texture* texture);

  /**
   * @brief Create a sync for the native image texture
   * @return True if sync object created.
   */
  bool CreateNativeTextureSync(const Context* context, const Framebuffer* framebuffer);

private:
  /**
   * @brief Remove native image texture from dependency list.
   * It will be called at discarding texture.
   */
  void DiscardNativeTexture(const Texture* texture);

public:                          // Type definitions. Make it public so unnamed namespace could use it.
  using SyncObjectId = uint32_t; ///< Note: Must match Dali::Graphics::GLES::SyncPool::SyncObjectId

  static constexpr SyncObjectId INVALID_SYNC_OBJECT_ID = 0u; ///< Note: Must match Dali::Graphics::GLES::SyncPool::INVALID_SYNC_OBJECT_ID

  // Dependencies for FBO write -> Surface read synchronization
  struct FramebufferTextureForwardDependency
  {
    std::vector<Texture*> textures;
    Context*              writeContext{nullptr};
    Framebuffer*          framebuffer{nullptr};
    SyncObjectId          agingSyncObjectId{INVALID_SYNC_OBJECT_ID};
    bool                  syncing{false};
  };
  using ForwardDependenciesList = std::vector<FramebufferTextureForwardDependency>;

  // Dependency info for textures that were read (Surface read -> FBO write sync)
  struct FramebufferTextureBackwardDependency
  {
    const Context* readContext{nullptr};
    SyncObjectId   agingSyncObjectId{INVALID_SYNC_OBJECT_ID};
  };
  using BackwardDependenciesList = std::vector<FramebufferTextureBackwardDependency>;
  using BackwardDependenciesMap  = std::unordered_map<const GLES::Texture*, BackwardDependenciesList>;

  using NativeTextureDependencyKey    = std::pair<const Context*, const Framebuffer*>;
  using NativeTextureDependenciesList = std::vector<std::pair<NativeTextureDependencyKey, const Texture*>>;

private:
  // Single-buffered map: track wrote textures at current frame to sync subsequent Surface read.
  ForwardDependenciesList mForwardDependencies;

  // Textures read during current frame (used for Surface read -> FBO write sync)
  std::vector<const GLES::Texture*> mReadTextures;

  // Double-buffered map: tracks read textures per frame to sync subsequent FBO writes
  // Index 0/1 swapped each frame; previous frame's syncs are freed after use
  BackwardDependenciesMap mBackwardDependencies[2];

  NativeTextureDependenciesList mNativeTextureDependencies;

  EglGraphicsController& mController;

  uint8_t mCurrentFrameIndex{0u}; ///< Index into mBackwardDependencies for current frame
  uint8_t mPrevFrameIndex{1u};    ///< Index into mBackwardDependencies for previous frame
};

} // namespace GLES
} // namespace Dali::Graphics

#endif //DALI_GLES_TEXTURE_DEPENDENCY_CHECKER_H