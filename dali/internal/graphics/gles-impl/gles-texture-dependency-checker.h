#ifndef DALI_GLES_TEXTURE_DEPENDENCY_CHECKER_H
#define DALI_GLES_TEXTURE_DEPENDENCY_CHECKER_H

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
 */

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
  : mController(controller)
  {
  }

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
   * will perform a glWaitSync() (GPU side semaphore) if the texture
   * needs syncing.
   * @param[in] readContext The context that the texture is being read (drawn with)
   * @param[in] texture The texture being read
   */
  void CheckNeedsSync(const Context* readContext, const Texture* texture);

private:
  struct TextureDependency
  {
    std::vector<Texture*> textures;
    Context*              writeContext{nullptr};
    Framebuffer*          framebuffer{nullptr};
    AgingSyncObject*      agingSyncObject;
    bool                  syncing{false};
  };
  std::vector<TextureDependency> mTextureDependencies;
  EglGraphicsController&         mController;
};

} // namespace GLES
} // namespace Dali::Graphics

#endif //DALI_GLES_TEXTURE_DEPENDENCY_CHECKER_H
