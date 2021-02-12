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

/**
 * @brief PipelineWrapper is the object
 * returned to the client-side
 */
class PipelineImpl
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo valid TextureCreateInfo structure
   * @param[in] controller Reference to the Controller
   */
  PipelineImpl(const Graphics::PipelineCreateInfo& createInfo, Graphics::EglGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~PipelineImpl();

  /**
   * @brief Destroys all the low-level resources used by the class
   */
  void DestroyResource();

  /**
   * @brief Initializes low-level resources
   *
   * @return Tron success
   */
  bool InitializeResource();

  /**
   * @brief Discards object
   */
  void DiscardResource();

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
  void Bind(GLES::PipelineImpl* prevPipeline);

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

  void Retain();

  void Release();

  [[nodiscard]] uint32_t GetRefCount() const;

  [[nodiscard]] const PipelineCreateInfo& GetCreateInfo() const;

  [[nodiscard]] auto& GetController() const;

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
  std::unique_ptr<PipelineState> mPipelineState;

  EglGraphicsController& mController;
  PipelineCreateInfo     mCreateInfo;

  uint32_t mGlProgram{0u};

  uint32_t mRefCount{0u};
};

/**
 * @brief Pipeline class wraps a unique pipeline object
 *
 */
class Pipeline : public Graphics::Pipeline
{
public:
  Pipeline() = delete;

  explicit Pipeline(GLES::PipelineImpl& pipeline)
  : mPipeline(pipeline)
  {
    // increase refcount
    mPipeline.Retain();
  }

  ~Pipeline() override
  {
    // decrease refcount
    mPipeline.Release();
  }

  [[nodiscard]] auto& GetPipeline() const
  {
    return mPipeline;
  }

  [[nodiscard]] const PipelineCreateInfo& GetCreateInfo() const;

  [[nodiscard]] EglGraphicsController& GetController() const;

private:
  GLES::PipelineImpl& mPipeline;
};

} // namespace Dali::Graphics::GLES
#endif