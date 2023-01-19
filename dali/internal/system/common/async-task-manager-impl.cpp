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
#include "async-task-manager-impl.h"

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/thread-settings.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr auto DEFAULT_NUMBER_OF_ASYNC_THREADS = size_t{8u};
constexpr auto NUMBER_OF_ASYNC_THREADS_ENV     = "DALI_ASYNC_MANAGER_THREAD_POOL_SIZE";

size_t GetNumberOfThreads(const char* environmentVariable, size_t defaultValue)
{
  auto           numberString          = EnvironmentVariable::GetEnvironmentVariable(environmentVariable);
  auto           numberOfThreads       = numberString ? std::strtoul(numberString, nullptr, 10) : 0;
  constexpr auto MAX_NUMBER_OF_THREADS = 10u;
  DALI_ASSERT_DEBUG(numberOfThreads < MAX_NUMBER_OF_THREADS);
  return (numberOfThreads > 0 && numberOfThreads < MAX_NUMBER_OF_THREADS) ? numberOfThreads : defaultValue;
}

#if defined(DEBUG_ENABLED)
Debug::Filter* gAsyncTasksManagerLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_ASYNC_TASK_MANAGER");
#endif

} // unnamed namespace

AsyncTaskThread::AsyncTaskThread(AsyncTaskManager& asyncTaskManager)
: mConditionalWait(),
  mAsyncTaskManager(asyncTaskManager),
  mLogFactory(Dali::Adaptor::Get().GetLogFactory()),
  mDestroyThread(false),
  mIsThreadStarted(false),
  mIsThreadIdle(true)
{
}

AsyncTaskThread::~AsyncTaskThread()
{
  // Stop the thread
  {
    ConditionalWait::ScopedLock lock(mConditionalWait);
    mDestroyThread = true;
    mConditionalWait.Notify(lock);
  }

  Join();
}

bool AsyncTaskThread::Request()
{
  if(!mIsThreadStarted)
  {
    Start();
    mIsThreadStarted = true;
  }

  {
    // Lock while adding task to the queue
    ConditionalWait::ScopedLock lock(mConditionalWait);

    if(mIsThreadIdle)
    {
      mIsThreadIdle = false;

      // wake up the thread
      mConditionalWait.Notify(lock);
      return true;
    }
  }

  return false;
}

void AsyncTaskThread::Run()
{
  SetThreadName("AsyncTaskThread");
  mLogFactory.InstallLogFunction();

  while(!mDestroyThread)
  {
    AsyncTaskPtr task = mAsyncTaskManager.PopNextTaskToProcess();
    if(!task)
    {
      ConditionalWait::ScopedLock lock(mConditionalWait);
      if(!mDestroyThread)
      {
        mIsThreadIdle = true;
        mConditionalWait.Wait(lock);
      }
    }
    else
    {
      task->Process();
      mAsyncTaskManager.CompleteTask(task);
    }
  }
}

Dali::AsyncTaskManager AsyncTaskManager::Get()
{
  Dali::AsyncTaskManager manager;
  SingletonService       singletonService(SingletonService::Get());
  if(singletonService)
  {
    // Check whether the async task manager is already created
    Dali::BaseHandle handle = singletonService.GetSingleton(typeid(Dali::AsyncTaskManager));
    if(handle)
    {
      // If so, downcast the handle of singleton
      manager = Dali::AsyncTaskManager(dynamic_cast<Internal::Adaptor::AsyncTaskManager*>(handle.GetObjectPtr()));
    }

    if(!manager)
    {
      // If not, create the async task manager and register it as a singleton
      Internal::Adaptor::AsyncTaskManager* internalAsyncTaskManager = new Internal::Adaptor::AsyncTaskManager();
      manager                                                       = Dali::AsyncTaskManager(internalAsyncTaskManager);
      singletonService.Register(typeid(manager), manager);
    }
  }
  return manager;
}

AsyncTaskManager::AsyncTaskManager()
: mTasks(GetNumberOfThreads(NUMBER_OF_ASYNC_THREADS_ENV, DEFAULT_NUMBER_OF_ASYNC_THREADS), [&]() { return TaskHelper(*this); }),
  mTrigger(new EventThreadCallback(MakeCallback(this, &AsyncTaskManager::TasksCompleted))),
  mProcessorRegistered(false)
{
}

AsyncTaskManager::~AsyncTaskManager()
{
  if(mProcessorRegistered && Dali::Adaptor::IsAvailable())
  {
    Dali::Adaptor::Get().UnregisterProcessor(*this);
  }

  mTasks.Clear();
}

void AsyncTaskManager::AddTask(AsyncTaskPtr task)
{
  {
    // Lock while adding task to the queue
    Mutex::ScopedLock lock(mMutex);
    mWaitingTasks.push_back(task);

    // Finish all Running threads are working
    if(mRunningTasks.size() >= mTasks.GetElementCount())
    {
      return;
    }
  }

  size_t count = mTasks.GetElementCount();
  size_t index = 0;
  while(index++ < count)
  {
    auto processHelperIt = mTasks.GetNext();
    DALI_ASSERT_ALWAYS(processHelperIt != mTasks.End());
    if(processHelperIt->Request())
    {
      break;
    }
    // If all threads are busy, then it's ok just to push the task because they will try to get the next job.
  }

  if(!mProcessorRegistered && Dali::Adaptor::IsAvailable())
  {
    Dali::Adaptor::Get().RegisterProcessor(*this);
    mProcessorRegistered = true;
  }

  return;
}

void AsyncTaskManager::RemoveTask(AsyncTaskPtr task)
{
  {
    // Lock while remove task from the queue
    Mutex::ScopedLock lock(mMutex);
    if(!mWaitingTasks.empty())
    {
      for(std::vector<AsyncTaskPtr>::iterator it = mWaitingTasks.begin(); it != mWaitingTasks.end();)
      {
        if((*it) && (*it) == task)
        {
          it = mWaitingTasks.erase(it);
        }
        else
        {
          it++;
        }
      }
    }

    if(!mRunningTasks.empty())
    {
      for(auto iter = mRunningTasks.begin(), endIter = mRunningTasks.end(); iter != endIter; ++iter)
      {
        if((*iter).first == task)
        {
          (*iter).second = true;
        }
      }
    }

    if(!mCompletedTasks.empty())
    {
      for(std::vector<AsyncTaskPtr>::iterator it = mCompletedTasks.begin(); it != mCompletedTasks.end();)
      {
        if((*it) && (*it) == task)
        {
          it = mCompletedTasks.erase(it);
        }
        else
        {
          it++;
        }
      }
    }
  }

  UnregisterProcessor();
}

AsyncTaskPtr AsyncTaskManager::PopNextTaskToProcess()
{
  // Lock while popping task out from the queue
  Mutex::ScopedLock lock(mMutex);

  // pop out the next task from the queue
  AsyncTaskPtr nextTask = nullptr;

  for(auto iter = mWaitingTasks.begin(), endIter = mWaitingTasks.end(); iter != endIter; ++iter)
  {
    if((*iter)->IsReady())
    {
      nextTask = *iter;

      // Add Running queue
      mRunningTasks.push_back(std::make_pair(nextTask, false));
      mWaitingTasks.erase(iter);
      break;
    }
  }

  return nextTask;
}

AsyncTaskPtr AsyncTaskManager::PopNextCompletedTask()
{
  // Lock while popping task out from the queue
  Mutex::ScopedLock lock(mMutex);

  if(mCompletedTasks.empty())
  {
    return AsyncTaskPtr();
  }

  std::vector<AsyncTaskPtr>::iterator next     = mCompletedTasks.begin();
  AsyncTaskPtr                        nextTask = *next;
  mCompletedTasks.erase(next);

  return nextTask;
}

void AsyncTaskManager::CompleteTask(AsyncTaskPtr task)
{
  // Lock while adding task to the queue
  {
    Mutex::ScopedLock lock(mMutex);
    for(auto iter = mRunningTasks.begin(), endIter = mRunningTasks.end(); iter != endIter; ++iter)
    {
      if((*iter).first == task)
      {
        if(!(*iter).second)
        {
          if(task->GetCallbackInvocationThread() == AsyncTask::ThreadType::MAIN_THREAD)
          {
            mCompletedTasks.push_back(task);
          }
        }

        // Delete this task in running queue
        mRunningTasks.erase(iter);
        break;
      }
    }
  }

  // wake up the main thread
  if(task->GetCallbackInvocationThread() == AsyncTask::ThreadType::MAIN_THREAD)
  {
    mTrigger->Trigger();
  }
  else
  {
    CallbackBase::Execute(*(task->GetCompletedCallback()), task);
  }
}

void AsyncTaskManager::UnregisterProcessor()
{
  if(mProcessorRegistered && Dali::Adaptor::IsAvailable())
  {
    Mutex::ScopedLock lock(mMutex);
    if(mWaitingTasks.empty() && mCompletedTasks.empty() && mRunningTasks.empty())
    {
      Dali::Adaptor::Get().UnregisterProcessor(*this);
      mProcessorRegistered = false;
    }
  }
}

void AsyncTaskManager::TasksCompleted()
{
  while(AsyncTaskPtr task = PopNextCompletedTask())
  {
    CallbackBase::Execute(*(task->GetCompletedCallback()), task);
  }

  UnregisterProcessor();
}

void AsyncTaskManager::Process(bool postProcessor)
{
  TasksCompleted();
}

AsyncTaskManager::TaskHelper::TaskHelper(AsyncTaskManager& asyncTaskManager)
: TaskHelper(std::unique_ptr<AsyncTaskThread>(new AsyncTaskThread(asyncTaskManager)), asyncTaskManager)
{
}

AsyncTaskManager::TaskHelper::TaskHelper(TaskHelper&& rhs)
: TaskHelper(std::move(rhs.mProcessor), rhs.mAsyncTaskManager)
{
}

AsyncTaskManager::TaskHelper::TaskHelper(std::unique_ptr<AsyncTaskThread> processor, AsyncTaskManager& asyncTaskManager)
: mProcessor(std::move(processor)),
  mAsyncTaskManager(asyncTaskManager)
{
}

bool AsyncTaskManager::TaskHelper::Request()
{
  return mProcessor->Request();
}
} // namespace Adaptor

} // namespace Internal

} // namespace Dali
