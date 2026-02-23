/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/async-task-manager.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/async-task-manager-impl.h>

namespace Dali
{
AsyncTask::AsyncTask(CallbackBase* callback, PriorityType priority, ThreadType threadType)
: mCompletedCallback(UniquePtr<CallbackBase>(callback)),
  mPriorityType(priority),
  mThreadType(threadType)
{
}

CallbackBase* AsyncTask::GetCompletedCallback()
{
  return mCompletedCallback.Get();
}

AsyncTask::ThreadType AsyncTask::GetCallbackInvocationThread() const
{
  return mThreadType;
}

AsyncTask::PriorityType AsyncTask::GetPriorityType() const
{
  return mPriorityType;
}

void AsyncTask::NotifyToReady()
{
  Internal::Adaptor::AsyncTaskManager::NotifyManagerToTaskReady(AsyncTaskPtr(this));
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

void AsyncTaskManager::NotifyToTaskReady(AsyncTaskPtr task)
{
  GetImplementation(*this).NotifyToTaskReady(task);
}

AsyncTaskManager::TasksCompletedId AsyncTaskManager::SetCompletedCallback(CallbackBase* callback, AsyncTaskManager::CompletedCallbackTraceMask mask)
{
  return GetImplementation(*this).SetCompletedCallback(callback, mask);
}

bool AsyncTaskManager::RemoveCompletedCallback(AsyncTaskManager::TasksCompletedId tasksCompletedId)
{
  return GetImplementation(*this).RemoveCompletedCallback(tasksCompletedId);
}

AsyncTaskManager::AsyncTaskManager(Internal::Adaptor::AsyncTaskManager* impl)
: BaseHandle(impl)
{
}

} // namespace Dali
