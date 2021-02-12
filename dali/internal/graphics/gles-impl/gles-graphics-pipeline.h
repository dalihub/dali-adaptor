#ifndef DALI_GRAPHICS_GLES_PIPELINE_H
#define DALI_GRAPHICS_GLES_PIPELINE_H

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
#include <string.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"

namespace Dali::Graphics::GLES
{
using PipelineResource = Resource<Graphics::Pipeline, Graphics::PipelineCreateInfo>;

class Pipeline : public PipelineResource
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo valid TextureCreateInfo structure
   * @param[in] controller Reference to the Controller
   */
  Pipeline(const Graphics::PipelineCreateInfo& createInfo, Graphics::EglGraphicsController& controller);

  /**
   * @brief Destroys all the low-level resources used by the class
   */
  void DestroyResource() override;

  /**
   * @brief Initializes low-level resources
   *
   * @return Tron success
   */
  bool InitializeResource() override;

  /**
   * @brief Discards object
   */
  void DiscardResource() override;

  /**
   * @brief returns GL program id
   * @return GL program id
   */
  [[nodiscard]] uint32_t GetGLProgram() const;

  /**
   * @brief Binds pipeline
   *
   * Binds Pipeline by binding GL program and flushing state.
   *
   * If previous pipeline specified, it will be used in order to
   * avoid redundant state swiches.
   *
   * @param[in] prevPipeline previous pipeline
   */
  void Bind(GLES::Pipeline* prevPipeline);

  /**
   * Executes state change function if condition met
   */
  template<typename FUNC, typename STATE>
  void ExecuteStateChange(FUNC& func, const STATE* prevPipelineState, const STATE* thisPipelineState)
  {
    if(!prevPipelineState)
    {
      func();
    }
    else
    {
      // binary test and execute when different
      if(memcmp(prevPipelineState, thisPipelineState, sizeof(STATE)) != 0)
      {
        func();
      }
    }
  }

private:
  /**
   * @brief Helper function. Copies state if pointer is set
   */
  template<class T>
  void CopyStateIfSet(const T* sourceState, T& copyState, T** destState)
  {
    *destState = nullptr;
    if(sourceState)
    {
      copyState  = *sourceState;
      *destState = &copyState;
    }
  }

  /**
   * @brief Helper function. Copies const state if pointer is set
   */
  template<class T>
  void CopyStateIfSet(const T* sourceState, T& copyState, const T** destState)
  {
    *destState = nullptr;
    if(sourceState)
    {
      copyState  = *sourceState;
      *destState = &copyState;
    }
  }

  // Pipeline state is stored as a copy of create info
  // data.
  struct PipelineState;
  std::unique_ptr<PipelineState> mPipelineState{};

  uint32_t mGlProgram{0u};
};

} // namespace Dali::Graphics::GLES
#endif