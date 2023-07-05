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
 *
 */

// CLASS HEADER
#include <dali/internal/system/windows/callback-manager-win.h>

// EXTERNAL INCLUDES
#include <Windows.h>

// Need to undef the following constants as they are defined in one of the headers in Windows.h but used in DALi (via debug.h)
#undef TRANSPARENT // Used in constants.h
#undef CopyMemory  // Used in dali-vector.h

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
WinCallbackManager::WinCallbackManager()
: mRunning(false)
{
}

void WinCallbackManager::Start()
{
  DALI_ASSERT_DEBUG(mRunning == false);
  mRunning = true;
}

void WinCallbackManager::Stop()
{
  // make sure we're not called twice
  DALI_ASSERT_DEBUG(mRunning == true);

  mRunning = false;
}

bool WinCallbackManager::AddIdleCallback(CallbackBase* callback, bool hasReturnValue)
{
  if(!mRunning)
  {
    return false;
  }

  mCallbacks.insert(callback);

  WindowsPlatform::PostWinThreadMessage(WIN_CALLBACK_EVENT, reinterpret_cast<uint64_t>(callback), 0);

  return true;
}

void WinCallbackManager::RemoveIdleCallback(CallbackBase* callback)
{
  //Wait for deal
}

bool WinCallbackManager::ProcessIdle()
{
  const bool idleProcessed = !mCallbacks.empty();

  for(CallbackBase* cb : mCallbacks)
  {
    Dali::CallbackBase::Execute(*cb);
  }
  mCallbacks.clear();

  return idleProcessed;
}

void WinCallbackManager::ClearIdleCallbacks()
{
  mCallbacks.clear();
}

bool WinCallbackManager::AddIdleEntererCallback(CallbackBase* callback)
{
  return AddIdleCallback(callback, true);
}

void WinCallbackManager::RemoveIdleEntererCallback(CallbackBase* callback)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
