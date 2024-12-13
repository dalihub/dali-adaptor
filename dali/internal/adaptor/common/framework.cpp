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
#include <dali/internal/adaptor/common/framework.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr auto NOT_SUPPORTED = "NOT_SUPPORTED";
} // namespace

Framework::Framework(Framework::Observer& observer, TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: mObserver(observer),
  mTaskObserver(taskObserver),
  mAbortHandler(MakeCallback(this, &Framework::AbortCallback)),
  mArgc(argc),
  mArgv(argv),
  mAbortCallBack(),
  mRunning(false)
{
}

Framework::~Framework()
{
}

std::string Framework::GetLanguage() const
{
  return NOT_SUPPORTED;
}

std::string Framework::GetRegion() const
{
  return NOT_SUPPORTED;
}

Any Framework::GetMainLoopContext() const
{
  return nullptr;
}

bool Framework::IsMainLoopRunning()
{
  return mRunning;
}

void Framework::AddAbortCallback(CallbackBase* callback)
{
  mAbortCallBack = std::unique_ptr<CallbackBase>(callback);
}

void Framework::AbortCallback()
{
  DALI_LOG_ERROR("AbortCallback comes!\n");
  // if an abort call back has been installed run it.
  if(mAbortCallBack)
  {
    CallbackBase::Execute(*mAbortCallBack);
  }
  else
  {
    Quit();
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
