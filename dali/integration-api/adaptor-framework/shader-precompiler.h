#ifndef DALI_INTEGRATION_SHADER_PRECOMPILER_H
#define DALI_INTEGRATION_SHADER_PRECOMPILER_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/dali-adaptor-common.h>
#include <memory>
#include <mutex>
#include <string_view>

namespace Dali
{
/**
 * ShaderPreCompiler  is used to precompile shaders.
 * The precompiled shaders are stored in a file.
 * @SINCE_2_2.45
 */
class DALI_ADAPTOR_API ShaderPreCompiler : public BaseHandle
{
public:
  struct RawShaderData
  {
    uint32_t                 shaderCount;
    std::vector<std::string> vertexPrefix;
    std::vector<std::string> fragmentPrefix;
    std::vector<std::string> shaderName;
    std::string_view         vertexShader;
    std::string_view         fragmentShader;
    bool                     custom;
  };

  using RawShaderDataList = std::vector<RawShaderData>;
  using CompiledProgram   = Dali::Graphics::UniquePtr<Dali::Graphics::Program>;

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
  void AddPreCompiledProgram(CompiledProgram program);

  /**
   * @brief Gets the shader list to be precompiled.
   * @note Stored shader list will be cleared after calling this function.
   *
   * @param[out] shaders shader data for precompile
   */
  void GetPreCompileShaderList(RawShaderDataList& shaders);

  /**
   * @brief Save the shader list to be precompiled
   *
   * @param[in] shaders shader data for precompile
   */
  void SavePreCompileShaderList(RawShaderDataList&& shaders);

  /**
   * @brief Enable the feature of precompile
   *
   * @param[in] enable True if we need to enable precompile. False if we need to disable precompile.
   */
  void Enable(bool enable);

  /**
   * @brief Check the feature of precompile is enabled or not
   *
   * @return true if the feature of precompile is enabled
   */
  bool IsEnable() const;

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

  /**
   * @brief Remove all pre-compiled programs from cache.
   * @note It must be called before graphics shutdown.
   */
  void ClearPreCompiledPrograms();

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
  static std::unique_ptr<ShaderPreCompiler> mInstance;
  static std::once_flag                     mOnceFlag;

private:
  std::vector<CompiledProgram> mProgram; ///< Keep compiled programs so we can use cached program.

  RawShaderDataList mRawShaderList;
  ConditionalWait   mConditionalWait;
  Dali::Mutex       mMutex;

  bool mEnabled : 1;
  bool mNeedsSleep : 1;
};

} // namespace Dali

#endif // DALI_INTEGRATION_SHADER_PRECOMPILER_H
