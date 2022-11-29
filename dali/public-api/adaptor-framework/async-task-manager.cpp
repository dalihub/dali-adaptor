/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include "async-task-manager.h"

// INTERNAL INCLUDES
#include <dali/internal/system/common/async-task-manager-impl.h>

namespace Dali
{

AsyncTask::AsyncTask(CallbackBase* callback)
: mCompletedCallback(std::unique_ptr<CallbackBase>(callback))
{
}

CallbackBase* AsyncTask::GetCompletedCallback()
{
  return mCompletedCallback.get();
}

AsyncTaskManager::AsyncTaskManager() = default;

AsyncTaskManager::~AsyncTaskManager() = default;

AsyncTaskManager AsyncTaskManager::Get()
{
  return Internal::Adaptor::AsyncTaskManager::Get();
}

void AsyncTaskManager::AddTask(AsyncTaskPtr task)
{
  GetImplementation(*this).AddTask(task);
}

void AsyncTaskManager::RemoveTask(AsyncTaskPtr task)
{
  GetImplementation(*this).RemoveTask(task);
}

AsyncTaskManager::AsyncTaskManager(Internal::Adaptor::AsyncTaskManager* impl)
: BaseHandle(impl)
{
}

} // namespace Dali
