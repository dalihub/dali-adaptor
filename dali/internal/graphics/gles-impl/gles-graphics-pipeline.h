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
#include "gles-graphics-reflection.h"
#include "gles-graphics-resource.h"

namespace Dali::Graphics::GLES
{
class PipelineCache;
class Program;

/**
 * @brief PipelineImpl is the implementation of Pipeline
 *
 * PipelineImpl is owned by the pipeline cache. The client-side
 * will receive Graphics::Pipeline objects which are only
 * wrappers for this implementation. The lifecycle of
 * PipelineImpl is managed by the PipelineCache.
 */
class PipelineImpl
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo valid TextureCreateInfo structure
   * @param[in] controller Reference to the Controller
   * @param[in] pipelineCache Reference to valid pipeline cache
   */
  PipelineImpl(const Graphics::PipelineCreateInfo& createInfo, Graphics::EglGraphicsController& controller, PipelineCache& pipelineCache);

  /**
   * @brief Destructor
   */
  ~PipelineImpl();

  /**
   * @brief Binds pipeline
   *
   * Binds Pipeline by binding GL program and flushing state.
   *
   * @param[in] glProgram The GL program to be bound
   */
  void Bind(const uint32_t glProgram) const;

  /**
   * @brief Increases ref count
   */
  void Retain();

  /**
   * @brief Decreases ref count
   */
  void Release();

  /**
   * @brief Retrieves ref count
   * @return Refcount value
   */
  [[nodiscard]] uint32_t GetRefCount() const;

  /**
   * @brief Returns PipelineCreateInfo structure
   *
   * @return PipelineCreateInfo structure
   */
  [[nodiscard]] const PipelineCreateInfo& GetCreateInfo() const;

  /**
   * @brief Returns controller
   *
   * @return Reference to the Controller
   */
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

  // Pipeline state is store locally so any assigned pointers on a
  // client-side may go safely out of scope.
  struct PipelineState;
  std::unique_ptr<PipelineState> mPipelineState;

  EglGraphicsController& mController;
  PipelineCreateInfo     mCreateInfo;

  uint32_t mRefCount{0u};
};

/**
 * @brief Pipeline class wraps the PipelineImpl
 */
class Pipeline : public Graphics::Pipeline
{
public:
  Pipeline() = delete;

  /**
   * @brief Constructor
   * @param pipeline Pipeline implementation
   */
  explicit Pipeline(GLES::PipelineImpl& pipeline)
  : mPipeline(pipeline)
  {
    // increase refcount
    mPipeline.Retain();
  }

  /**
   * @brief Destructor
   */
  ~Pipeline() override;

  /**
   * @brief Returns pipeline implementation
   *
   * @return Valid pipeline implementation
   */
  [[nodiscard]] auto& GetPipeline() const
  {
    return mPipeline;
  }

  /**
   * @brief Returns create info structure
   *
   * @return Valid create info structure
   */
  [[nodiscard]] const PipelineCreateInfo& GetCreateInfo() const;

  /**
   * @brief Returns controller
   *
   * @return reference to Controller
   */
  [[nodiscard]] EglGraphicsController& GetController() const;

  bool operator==(const PipelineImpl* impl) const
  {
    return &mPipeline == impl;
  }

  /**
   * @brief Run by UniquePtr to discard resource
   */
  void DiscardResource();

  /**
   * @brief Destroy resource
   *
   * Despite this class doesn't inherit Resource it must provide
   * (so it won't duplicate same data) same set of functions
   * so it can work with resource management functions of Controller.
   */
  void DestroyResource()
  {
    // Nothing to do here
  }

private:
  GLES::PipelineImpl& mPipeline;
};

} // namespace Dali::Graphics::GLES
#endif