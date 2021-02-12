#ifndef DALI_GRAPHICS_GLES_PIPELINE_CACHE_H
#define DALI_GRAPHICS_GLES_PIPELINE_CACHE_H

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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-pipeline-create-info.h>
#include <dali/graphics-api/graphics-pipeline.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"

namespace Dali::Graphics
{
class EglGraphicsController;
namespace GLES
{
class Pipeline;
class PipelineImpl;

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

private:
  /**
   * @brief Finds pipeline implementation based on the spec
   * @param[in] info Valid create info structure
   * @return Returns pointer to pipeline or nullptr
   */
  PipelineImpl* FindPipelineImpl(const PipelineCreateInfo& info);

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};
} // namespace GLES
} // namespace Dali::Graphics
#endif
