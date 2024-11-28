#ifndef DALI_GLES_TEXTURE_DEPENDENCY_CHECKER_H
#define DALI_GLES_TEXTURE_DEPENDENCY_CHECKER_H

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
 */

// EXTERNAL INCLUDES
#include <cstddef>
#include <cstdint>
#include <unordered_set>

// INTERNAL INCLUDES
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali::Graphics
{
class EglGraphicsController;

namespace GLES
{
class Context;
class Framebuffer;
class Texture;
class AgingSyncObject;

/**
 * Class to handle dependency checks between textures on different
 * GL Contexts.
 *
 * We have a shared (resource) context for writing to offscreen framebuffers,
 * and separate contexts for each window/scene.
 * If a framebuffer attachment is used in a scene, then it needs a sync point
 * in the GPU in order to ensure that the first context finishes writing to the
 * texture before it is read in the scene context.
 */
class TextureDependencyChecker
{
public:
  explicit TextureDependencyChecker(EglGraphicsController& controller)
  : mController(controller),
    mCurrentNativeTextureDependencyIndex(0u),
    mPreviousNativeTextureDependencyIndex(1u),
    mIsFirstPreparedNativeTextureDependency(true)
  {
  }

  ~TextureDependencyChecker();

  /**
   * Clear all the textures. Call at the start of a frame
   */
  void Reset();

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
    return mFramebufferTextureDependencies.size();
  }

  /**
   * Get the number of (offscreen) textures for dependency checking
   */
  size_t GetNativeTextureCount() const
  {
    return mNativeTextureDependencies[mCurrentNativeTextureDependencyIndex].size();
  }

public: ///< For NativeTexture dependency checker
  /**
   * @brief Add prepared native image texture to dependency list
   */
  void MarkNativeTexturePrepared(const Texture* texture);

  /**
   * @brief Remove native image texture from dependency list.
   * It will be called at discarding texture.
   */
  void DiscardNativeTexture(const Texture* texture);

  /**
   * @brief Create Sync object for native images.
   * It will be called at EndRenderPass.
   */
  void CreateNativeTextureSync(const Context* writeContext);

private:
  struct FramebufferTextureDependency
  {
    std::vector<Texture*> textures;
    Context*              writeContext{nullptr};
    Framebuffer*          framebuffer{nullptr};
    AgingSyncObject*      agingSyncObject{nullptr};
    bool                  syncing{false};
  };
  std::vector<FramebufferTextureDependency> mFramebufferTextureDependencies;

  struct NativeTextureDependency
  {
    std::unordered_set<const Texture*> textures;
    AgingSyncObject*                   agingSyncObject{nullptr};
    bool                               synced{false};
  };
  std::vector<NativeTextureDependency> mNativeTextureDependencies[2];

  EglGraphicsController& mController;

  uint32_t mCurrentNativeTextureDependencyIndex;  // 0 or 1, toggled every frame
  uint32_t mPreviousNativeTextureDependencyIndex; // 0 or 1, toggled every frame

  bool mIsFirstPreparedNativeTextureDependency : 1;
};

} // namespace GLES
} // namespace Dali::Graphics

#endif //DALI_GLES_TEXTURE_DEPENDENCY_CHECKER_H
