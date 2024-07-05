#ifndef DALI_GRAPHICS_GLES_PROGRAM_H
#define DALI_GRAPHICS_GLES_PROGRAM_H

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
#include <dali/graphics-api/graphics-program-create-info.h>
#include <dali/graphics-api/graphics-program.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"

namespace Dali::Graphics::GLES
{
class Reflection;

/**
 * @brief Program implementation
 *
 * Program implementation is owned only by the PipelineCache
 *
 * Like pipeline, it's created and managed by the PipelineCache
 */
class ProgramImpl
{
public:
  /**
   * @brief Constructor
   *
   * @param[in] createInfo  Valid create info structure
   * @param[in] controller Valid reference to the controller object
   */
  ProgramImpl(const Graphics::ProgramCreateInfo& createInfo, Graphics::EglGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~ProgramImpl();

  /**
   * @brief Destroys GL resources associated with the Program
   *
   * @return True on success
   */
  bool Destroy();

  /**
   * @brief Creates GL resource for this Program
   *
   * @return True on success
   */
  bool Create();

  /**
   * @brief Preprocesses shaders
   */
  void Preprocess();

  /**
   * @brief Returns GL program id
   *
   * @return GL program id
   */
  [[nodiscard]] uint32_t GetGlProgram() const;

  /**
   * @brief Increases ref count
   *
   * @return new refcount
   */
  uint32_t Retain();

  /**
   * @brief Decreases ref count
   *
   * @return New refcount
   */
  uint32_t Release();

  /**
   * @brief Retrieves ref count
   * @return Refcount value
   */
  [[nodiscard]] uint32_t GetRefCount() const;

  /**
   * @brief Returns reflection
   *
   * @return Valid reflection associated with the Program
   */
  [[nodiscard]] const GLES::Reflection& GetReflection() const;

  /**
   * @brief Returns controller
   *
   * @return Valid Controller object
   */
  [[nodiscard]] EglGraphicsController& GetController() const;

  /**
   * @brief Returns create info structure
   *
   * @return Reference to valid create info structure
   */
  [[nodiscard]] const ProgramCreateInfo& GetCreateInfo() const;

  /**
   * @brief Returns parameter value specified by parameterId
   *
   * This function can be used as a backdoor into the implementation
   * used to retrieve internal data.
   *
   * @param[in] parameterId Integer parameter id
   * @param[out] out Pointer to write to
   *
   * @return True on success
   */
  bool GetParameter(uint32_t parameterId, void* out);

  /**
   * @brief Updates standalone uniforms
   *
   * Updates standalone uniforms (issues the GL calls) and
   * updates internal uniform cache
   *
   * @param[in] ptr Valid pointer to the uniform block memory
   */
  void UpdateStandaloneUniformBlock(const char* ptr);

  /**
   * @brief Builds standalone uniform cache
   *
   * This function allocates cache memory and
   * gathers a list of GL functions per uniform type.
   */
  void BuildStandaloneUniformCache();

private:
  friend class Program;

  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

///////////////////////////////////////////////////////////////

/**
 * @brief Wrapper for the program implementation
 *
 * This object is returned back to the client-side
 */
class Program : public Graphics::Program
{
public:
  /**
   * @brief Constructor
   *
   * @param[in] impl Pointer to valid implementation
   */
  explicit Program(ProgramImpl* impl)
  : mProgram(impl)
  {
    mProgram->Retain();
  }

  /**
   * @brief Destructor
   */
  ~Program() override;

  /**
   * @brief Returns reference to the Reflection object

   * @return Reflection
   */
  [[nodiscard]] const GLES::Reflection& GetReflection() const;

  /**
   * @brief Retrieves internal program implementation
   *
   * @return Valid pointer to the ProgramImpl object
   */
  [[nodiscard]] ProgramImpl* GetImplementation() const
  {
    return mProgram;
  }

  /**
   * @brief Returns controller
   *
   * @return controller
   */
  [[nodiscard]] EglGraphicsController& GetController() const;

  /**
   * @brief Returns create info structure
   *
   * @return create info structure
   */
  [[nodiscard]] const ProgramCreateInfo& GetCreateInfo() const;

  bool operator==(const GLES::Program& program) const
  {
    return (program.mProgram == mProgram);
  }

  bool operator==(const GLES::ProgramImpl* programImpl) const
  {
    return (programImpl == mProgram);
  }

  bool operator!=(const GLES::Program& program) const
  {
    return (program.mProgram != mProgram);
  }

  /**
   * @brief Run by UniquePtr to discard resource
   */
  void DiscardResource();

  /**
   * @brief Destroying GL resources
   *
   * This function is kept for compatibility with Resource<> class
   * so can the object can be use with templated functions.
   */
  void DestroyResource()
  {
    // nothing to do here
  }

private:
  ProgramImpl* mProgram{nullptr};
};
} // namespace Dali::Graphics::GLES

#endif //DALI_GRAPHICS_GLES_PROGRAM_H
