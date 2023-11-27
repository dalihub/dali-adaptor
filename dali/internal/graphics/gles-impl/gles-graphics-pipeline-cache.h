#ifndef DALI_GRAPHICS_GLES_PIPELINE_CACHE_H
#define DALI_GRAPHICS_GLES_PIPELINE_CACHE_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/graphics-api/graphics-pipeline-create-info.h>
#include <dali/graphics-api/graphics-pipeline.h>
#include <dali/graphics-api/graphics-program-create-info.h>
#include <dali/graphics-api/graphics-program.h>
#include <dali/graphics-api/graphics-shader-create-info.h>
#include <dali/graphics-api/graphics-shader.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"

namespace Dali::Graphics
{
class EglGraphicsController;
namespace GLES
{
class Pipeline;
class PipelineImpl;
class Program;
class ProgramImpl;
class Shader;
class ShaderImpl;

/**
 * @brief PipelineCache manages pipeline and program
 * objects so there are no duplicates created.
 */
class PipelineCache
{
public:
  /**
   * @brief Constructor
   */
  explicit PipelineCache(EglGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~PipelineCache();

  /**
   * @brief Retrieves pipeline matching the spec
   *
   * Function returns either existing pipeline if one is found
   * in the cache or creates new one.
   *
   * @param[in] pipelineCreateInfo Valid PipelineCreateInfo structure
   * @param[in] oldPipeline previous pipeline object
   * @return Pipeline object
   */
  Graphics::UniquePtr<Graphics::Pipeline> GetPipeline(const PipelineCreateInfo& pipelineCreateInfo, Graphics::UniquePtr<Graphics::Pipeline>&& oldPipeline);

  /**
   * @brief Retrieves program matching the spec
   *
   * Function returns either existing program if one is found
   * in the cache or creates new one.
   *
   * @param[in] programCreateInfo Valid ProgramCreateInfo structure
   * @param[in] oldProgram previous program object
   * @return Program object
   */
  Graphics::UniquePtr<Graphics::Program> GetProgram(const ProgramCreateInfo& pipelineCreateInfo, Graphics::UniquePtr<Graphics::Program>&& oldProgram);

  /**
   * @brief Retrieves shader matching the spec
   *
   * Function returns either existing shader if one is found
   * in the cache or creates new one.
   *
   * @param[in] shaderCreateInfo Valid ShaderCreateInfo structure
   * @param[in] oldShader previous shader object
   * @return Shader object
   */
  Graphics::UniquePtr<Graphics::Shader> GetShader(const ShaderCreateInfo& shaderCreateInfo, Graphics::UniquePtr<Graphics::Shader>&& oldShader);

  /**
   * @brief Flushes pipeline and program cache
   *
   * Removes cached items when they are no longer needed. This function
   * should be called at the very end of Controller render loop iteration.
   */
  void FlushCache();

  /**
   * @brief Set true if we can flush cached pipeline / program / shader.
   * If we make it false, we can keep shader / program instance during app running.
   * But it might have sightly panalty for memory.
   * Default is True.
   *
   * @param enabled True if we can flush the caches. False when we don't want to flush caches.
   */
  void EnableCacheFlush(bool enabled);

  /**
   * @brief Notify that we need to flush pipeline cache next FlushCache API.
   */
  void MarkPipelineCacheFlushRequired();

  /**
   * @brief Notify that we need to flush program cache next FlushCache API.
   */
  void MarkProgramCacheFlushRequired();

  /**
   * @brief Notify that we need to flush shader cache next FlushCache API.
   */
  void MarkShaderCacheFlushRequired();

private:
  /**
   * @brief Finds pipeline implementation based on the spec
   * @param[in] info Valid create info structure
   * @return Returns pointer to pipeline or nullptr
   */
  PipelineImpl* FindPipelineImpl(const PipelineCreateInfo& info);

  /**
   * @brief Finds program implementation based on the spec
   * @param[in] info Valid create info structure
   * @return Returns pointer to program or nullptr
   */
  ProgramImpl* FindProgramImpl(const ProgramCreateInfo& info);

  /**
   * @brief Finds shader implementation based on create info
   *
   * @param[in] shadercreateinfo Valid create info structure
   * @return Returns pointer to shader or nullptr
   */
  ShaderImpl* FindShaderImpl(const ShaderCreateInfo& shaderCreateInfo);

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};
} // namespace GLES
} // namespace Dali::Graphics
#endif
