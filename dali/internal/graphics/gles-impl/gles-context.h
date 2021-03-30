#ifndef DALI_GRAPHICS_GLES_CONTEXT_H
#define DALI_GRAPHICS_GLES_CONTEXT_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/graphics-api/graphics-command-buffer.h>
#include "gles-graphics-types.h"

namespace Dali::Graphics
{
class EglGraphicsController;
namespace GLES
{
class Pipeline;
class Texture;

/**
 * @brief Context represents single GLES context
 */
class Context
{
public:
  explicit Context(EglGraphicsController& controller);

  ~Context();

  /**
   * @brief Flushes the context
   *
   * Flushes the context by issuing GL calls to set the required
   * state.
   *
   * @param[in] reset If true then state is reset unconditionally
   */
  void Flush(bool reset, const GLES::DrawCallDescriptor& drawCall);

  /**
   * @brief Returns context Id
   * @return
   */
  uint32_t GetContextId() const
  {
    return 0;
  }

  /**
   * @brief Binds textures to the context
   *
   * Bindings are merged, they are resolved at later time
   * when the draw call is issued. Then the texture binding
   * slots (GL active texture binding) are enabled/disabled accordingly
   * to the uniform layout.
   *
   * @param[in] bindings List of bindings
   *
   */
  void BindTextures(const std::vector<Graphics::TextureBinding>& bindings);

  /**
   * @brief Vertex buffers to bind
   *
   * The bindings are taken from a command buffer being
   * currently processed and copied into the local storage.
   */

  void BindVertexBuffers(const std::vector<GLES::VertexBufferBindingDescriptor>& bindings);

  /**
   * @brief Binds index buffer
   *
   * @param indexBufferBinding
   */
  void BindIndexBuffer(const IndexBufferBindingDescriptor& indexBufferBinding);

  /**
   * @brief Binds pipeline to the context
   * @param newPipeline
   */
  void BindPipeline(const GLES::Pipeline* newPipeline);

  /**
   * @brief Binds uniform buffers to the context
   *
   * @param[in] uboBindings real UBO binfins
   * @param[in] standaloneBindings emulated (legacy) UBO object
   */
  void BindUniformBuffers(const std::vector<UniformBufferBindingDescriptor>& uboBindings, const UniformBufferBindingDescriptor& standaloneBindings);

  /**
   * @brief Resolves blend state on the currently attached pipeline
   */
  void ResolveBlendState();

  /**
   * @brief Resolves rasterization state on the currently attached pipeline
   */
  void ResolveRasterizationState();

  /**
   * @brief Resolves uniform buffers and binds data to the pipeline
   */
  void ResolveUniformBuffers();

  /**
   * @brief Special usecase for legacy shaders, called by ResolveUniformBuffers()
   */
  void ResolveStandaloneUniforms();

private:
  /**
   * @brief Clear current state
   */
  void ClearState();

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};
} // namespace GLES
} // namespace Dali::Graphics
#endif //DALI_GRAPHICS_GLES_CONTEXT_H
