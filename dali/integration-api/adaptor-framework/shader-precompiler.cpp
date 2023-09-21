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
 */

// CLASS HEADER
#include <dali/integration-api/adaptor-framework/shader-precompiler.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{
std::unique_ptr<ShaderPreCompiler> ShaderPreCompiler::mInstance = nullptr;
std::once_flag ShaderPreCompiler::mOnceFlag;

ShaderPreCompiler::ShaderPreCompiler()
: mRawShaderList(),
  mPrecompiled(false),
  mEnabled(false)
{
}

ShaderPreCompiler& ShaderPreCompiler::Get()
{
  std::call_once(mOnceFlag, []()
                 { mInstance.reset(new ShaderPreCompiler); });

  return *(mInstance.get());
}

void ShaderPreCompiler::AddPreCompiledProgram(Graphics::UniquePtr<Dali::Graphics::Program> program)
{
  mProgram.push_back(move(program));
}

void ShaderPreCompiler::GetPreCompileShaderList(std::vector<RawShaderData>& shaderList)
{
  // move shader list
  shaderList = mRawShaderList;
}

void ShaderPreCompiler::SavePreCompileShaderList(std::vector<RawShaderData>& shaderList)
{
  mRawShaderList = shaderList;
  mPrecompiled = true;
  Awake();
}

bool ShaderPreCompiler::IsReady() const
{
  return mPrecompiled;
}

void ShaderPreCompiler::Enable()
{
  mEnabled = true;
}

bool ShaderPreCompiler::IsEnable()
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

  mConditionalWait.Wait(lock);
}

void ShaderPreCompiler::Awake()
{
  ConditionalWait::ScopedLock lock(mConditionalWait);
  Dali::Mutex::ScopedLock mutexLock(mMutex);
  mNeedsSleep = false;
  mConditionalWait.Notify(lock);
}
} // namespace Dali
