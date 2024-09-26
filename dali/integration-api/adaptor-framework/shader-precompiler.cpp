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

// CLASS HEADER
#include <dali/integration-api/adaptor-framework/shader-precompiler.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>

namespace
{
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_PERFORMANCE_MARKER, false);
}

namespace Dali
{
std::unique_ptr<ShaderPreCompiler> ShaderPreCompiler::mInstance = nullptr;
std::once_flag                     ShaderPreCompiler::mOnceFlag;

ShaderPreCompiler::ShaderPreCompiler()
: mRawShaderList(),
  mEnabled(false),
  mNeedsSleep(true)
{
}

ShaderPreCompiler& ShaderPreCompiler::Get()
{
  std::call_once(mOnceFlag, []() { mInstance.reset(new ShaderPreCompiler); });

  return *(mInstance.get());
}

void ShaderPreCompiler::AddPreCompiledProgram(ShaderPreCompiler::CompiledProgram program)
{
  mProgram.push_back(move(program));
}

void ShaderPreCompiler::GetPreCompileShaderList(ShaderPreCompiler::RawShaderDataList& shaderList)
{
  // move shader list
  shaderList = std::move(mRawShaderList);

  // Call clear, to avoid SVACE false alarm.
  mRawShaderList.clear();
}

void ShaderPreCompiler::SavePreCompileShaderList(ShaderPreCompiler::RawShaderDataList&& shaderList)
{
  mRawShaderList = std::move(shaderList);
  DALI_LOG_RELEASE_INFO("Precompile shader list is saved! Precompile available now\n");
  Awake();
}

void ShaderPreCompiler::Enable(bool enable)
{
  mEnabled = enable;
}

bool ShaderPreCompiler::IsEnable() const
{
  return mEnabled;
}

void ShaderPreCompiler::Wait()
{
  ConditionalWait::ScopedLock lock(mConditionalWait);
  {
    Dali::Mutex::ScopedLock mutexLock(mMutex);
    if(!mNeedsSleep)
    {
      return;
    }
  }

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_SHADER_PRECOMPILE_WAIT");
  mConditionalWait.Wait(lock);
  DALI_TRACE_END(gTraceFilter, "DALI_SHADER_PRECOMPILE_WAIT");
}

void ShaderPreCompiler::Awake()
{
  ConditionalWait::ScopedLock lock(mConditionalWait);
  Dali::Mutex::ScopedLock     mutexLock(mMutex);
  mNeedsSleep = false;
  mConditionalWait.Notify(lock);
}

void ShaderPreCompiler::ClearPreCompiledPrograms()
{
  mProgram.clear();
}

} // namespace Dali
