#ifndef DALI_GRAPHICS_GLES_SHADER_H
#define DALI_GRAPHICS_GLES_SHADER_H

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
 *
 */

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-shader-create-info.h>
#include <dali/graphics-api/graphics-shader.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"

namespace Dali::Graphics::GLES
{
class ShaderImpl
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo Valid createInfo structure
   * @param[in] controller Reference to the controller
   */
  ShaderImpl(const Graphics::ShaderCreateInfo& createInfo, Graphics::EglGraphicsController& controller);
  ~ShaderImpl();

  uint32_t Retain();

  uint32_t Release();

  [[nodiscard]] uint32_t GetRefCount() const;

  /**
   * Whilst unreferenced, increase the flush count and return it
   *
   * @return The new flush count
   */
  [[nodiscard]] uint32_t IncreaseFlushCount();

  /**
   * Get the flush count whilst unreferenced
   *
   * @return the flush count
   */
  [[nodiscard]] uint32_t GetFlushCount() const;

  /**
   * @brief Compiles shader
   *
   * @return True on success
   */
  [[nodiscard]] bool Compile() const;

  /**
   * @brief Destroys GL shader
   */
  void Destroy();

  uint32_t GetGLShader() const;

  [[nodiscard]] const ShaderCreateInfo& GetCreateInfo() const;

  [[nodiscard]] EglGraphicsController& GetController() const;

  /**
   * Strips legacy prefix fromt he GLSL source code if necessary
   * @param info valid ShaderCreateInfo strucutre
   * @param[out] startIndex Start index of the source code
   * @param[out] finalDataSize Size of trimmed data
   */
  static void StripLegacyCodeIfNeeded(const ShaderCreateInfo& info, size_t& startIndex, size_t& finalDataSize);

private:
  friend class Shader;
  struct Impl;
  std::unique_ptr<Impl> mImpl{nullptr};
};

class Shader : public Graphics::Shader
{
public:
  /**
   * @brief Constructor
   *
   * @param[in] impl Pointer to valid implementation
   */
  explicit Shader(ShaderImpl* impl)
  : mShader(impl)
  {
    mShader->Retain();
  }

  /**
   * @brief Destructor
   */
  ~Shader() override;

  [[nodiscard]] ShaderImpl* GetImplementation() const
  {
    return mShader;
  }

  [[nodiscard]] const ShaderCreateInfo& GetCreateInfo() const;

  bool operator==(const GLES::Shader& shader) const
  {
    return (shader.mShader == mShader);
  }

  bool operator==(const GLES::ShaderImpl* shaderImpl) const
  {
    return (shaderImpl == mShader);
  }

  bool operator!=(const GLES::Shader& shader) const
  {
    return (shader.mShader != mShader);
  }

  /**
   * @brief Called when UniquePtr<> on client-side dies.
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
  ShaderImpl* mShader{nullptr};
};

} // namespace Dali::Graphics::GLES

#endif
