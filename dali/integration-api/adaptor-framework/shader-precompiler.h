#ifndef DALI_INTEGRATION_SHADER_PRECOMPILER_H
#define DALI_INTEGRATION_SHADER_PRECOMPILER_H

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

// INTERNAL HEADER
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/object/base-handle.h>

// EXTERNAL HEDAER
#include <dali/graphics-api/graphics-program.h>
#include <dali/graphics-api/graphics-types.h>
#include <memory>
#include <mutex>
#include <string_view>

namespace Dali
{
struct RawShaderData
{
  int                           shaderCount;
  std::vector<std::string_view> vertexPrefix;
  std::vector<std::string_view> fragmentPrefix;
  std::vector<std::string_view> shaderName;
  std::string_view              vertexShader;
  std::string_view              fragmentShader;
  bool custom;
};

/**
 * ShaderPreCompiler  is used to precompile shaders.
 * The precompiled shaders are stored in a file.
 * @SINCE_2_2.45
 */
class DALI_CORE_API ShaderPreCompiler : public BaseHandle
{
public:
  /**
   * @brief Gets the singleton of ShaderPreCompiler object.
   *
   * @return A handle to the ShaderPreCompiler
   */
  static ShaderPreCompiler& Get();

  /**
   * @brief Add a precompiled program to the ShaderPreCompiler
   *
   * @param[in] program precompiled program
   */
  void AddPreCompiledProgram(Graphics::UniquePtr<Dali::Graphics::Program> program);

  /**
   * @brief Gets the shader list to be precompiled
   *
   *  @param[in] shaders shader data for precompile
   */
  void GetPreCompileShaderList(std::vector<RawShaderData>& shaders);

  /**
   * @brief Save the shader list to be precompiled
   *
   * @param[in] shaders shader data for precompile
   */
  void SavePreCompileShaderList(std::vector<RawShaderData>& shaders);

  /**
   * @brief Checks whether the precompiled list is ready or not
   *
   * @return true if precompile list is ready
   */
  bool IsReady() const;

  /**
   * @brief Enable the feature of precompile
   *
   */
  void Enable();

  /**
   * @brief Check the feature of precompile is enabled or not
   *
   * @return true if the feature of precompile is enabled
  */
  bool IsEnable();

  /**
   * @brief Waiting for a list of shaders to be precompiled
   *
   */
  void Wait();

  /**
   * @brief Awake waiting for a list of shaders to be precompiled
   *
   */
  void Awake();

private:
  /**
   * Construct a new ShaderPreCompiler.
   */
  ShaderPreCompiler();

  // Undefined
  ShaderPreCompiler(const ShaderPreCompiler&) = delete;

  // Undefined
  ShaderPreCompiler& operator=(const ShaderPreCompiler& rhs) = delete;

private:
  std::vector<Graphics::UniquePtr<Dali::Graphics::Program>> mProgram;
  static std::unique_ptr<ShaderPreCompiler>                 mInstance;
  static std::once_flag                                     mOnceFlag;
  std::vector<RawShaderData>                                mRawShaderList;
  ConditionalWait                                           mConditionalWait;
  Dali::Mutex                                               mMutex;
  bool                                                      mPrecompiled;
  bool                                                      mEnabled;
  bool                                                      mNeedsSleep{true};
};

} // namespace Dali

#endif // DALI_INTEGRATION_SHADER_PRECOMPILER_H
