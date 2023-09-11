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
#include <dali/internal/system/common/async-task-manager-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/thread-settings.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>

#include <unordered_map>

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

// The number of threads for low priority task.
constexpr auto DEFAULT_NUMBER_OF_LOW_PRIORITY_THREADS = size_t{6u};
constexpr auto NUMBER_OF_LOW_PRIORITY_THREADS_ENV     = "DALI_ASYNC_MANAGER_LOW_PRIORITY_THREAD_POOL_SIZE";

size_t GetNumberOfThreads(const char* environmentVariable, size_t defaultValue)
{
  auto           numberString          = EnvironmentVariable::GetEnvironmentVariable(environmentVariable);
  auto           numberOfThreads       = numberString ? std::strtoul(numberString, nullptr, 10) : 0;
  constexpr auto MAX_NUMBER_OF_THREADS = 16u;
  DALI_ASSERT_DEBUG(numberOfThreads <= MAX_NUMBER_OF_THREADS);
  return (numberOfThreads > 0 && numberOfThreads <= MAX_NUMBER_OF_THREADS) ? numberOfThreads : defaultValue;
}

size_t GetNumberOfLowPriorityThreads(const char* environmentVariable, size_t defaultValue, size_t maxValue)
{
  auto numberString    = EnvironmentVariable::GetEnvironmentVariable(environmentVariable);
  auto numberOfThreads = numberString ? std::strtoul(numberString, nullptr, 10) : 0;
  DALI_ASSERT_DEBUG(numberOfThreads <= maxValue);
  return (numberOfThreads > 0 && numberOfThreads <= maxValue) ? numberOfThreads : std::min(defaultValue, maxValue);
}

#if defined(DEBUG_ENABLED)
Debug::Filter* gAsyncTasksManagerLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_ASYNC_TASK_MANAGER");

uint32_t gThreadId = 0u; // Only for debug
#endif

} // unnamed namespace

// AsyncTaskThread

AsyncTaskThread::AsyncTaskThread(AsyncTaskManager& asyncTaskManager)
: mConditionalWait(),
  mAsyncTaskManager(asyncTaskManager),
  mLogFactory(Dali::Adaptor::Get().GetLogFactory()),
  mTraceFactory(Dali::Adaptor::Get().GetTraceFactory()),
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
#if defined(DEBUG_ENABLED)
  uint32_t threadId = gThreadId++;
  {
    char temp[100];
    snprintf(temp, 100, "AsyncTaskThread[%u]", threadId);
    SetThreadName(temp);
  }
#else
  SetThreadName("AsyncTaskThread");
#endif
  mLogFactory.InstallLogFunction();
  mTraceFactory.InstallTraceFunction();

  while(!mDestroyThread)
  {
    AsyncTaskPtr task = mAsyncTaskManager.PopNextTaskToProcess();
    if(!task)
    {
      ConditionalWait::ScopedLock lock(mConditionalWait);
      if(!mDestroyThread)
      {
        mIsThreadIdle = true;
        DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Thread[%u] wait\n", threadId);
        mConditionalWait.Wait(lock);
        DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Thread[%u] awake\n", threadId);
      }
    }
    else
    {
      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Thread[%u] Process task [%p]\n", threadId, task.Get());
      task->Process();
      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Thread[%u] Complete task [%p]\n", threadId, task.Get());
      if(!mDestroyThread)
      {
        mAsyncTaskManager.CompleteTask(std::move(task));
      }
    }
  }
}

// AsyncTaskManager::CacheImpl

struct AsyncTaskManager::CacheImpl
{
  CacheImpl(AsyncTaskManager& manager)
  : mManager(manager)
  {
  }

public:
  // Insert / Erase task cache API.

  /**
   * @brief Insert cache that input task.
   * @pre Mutex be locked.
   */
  template<typename CacheContainer, typename Iterator>
  static void InsertTaskCache(CacheContainer& cacheMap, AsyncTaskPtr task, Iterator iterator)
  {
    auto& cacheContainer = cacheMap[task.Get()]; // Get or Create cache container.
    cacheContainer.insert(cacheContainer.end(), iterator);
  }

  /**
   * @brief Erase cache that input task.
   * @pre Mutex be locked.
   */
  template<typename CacheContainer, typename Iterator>
  static void EraseTaskCache(CacheContainer& cacheMap, AsyncTaskPtr task, Iterator iterator)
  {
    auto mapIter = cacheMap.find(task.Get());
    if(mapIter != cacheMap.end())
    {
      auto& cacheContainer = (*mapIter).second;
      auto  cacheIter      = std::find(cacheContainer.begin(), cacheContainer.end(), iterator);

      if(cacheIter != cacheContainer.end())
      {
        cacheContainer.erase(cacheIter);
        if(cacheContainer.empty())
        {
          cacheMap.erase(mapIter);
        }
      }
    }
  }

  /**
   * @brief Erase all cache that input task.
   * @pre Mutex be locked.
   */
  template<typename CacheContainer>
  static void EraseAllTaskCache(CacheContainer& cacheMap, AsyncTaskPtr task)
  {
    auto mapIter = cacheMap.find(task.Get());
    if(mapIter != cacheMap.end())
    {
      cacheMap.erase(mapIter);
    }
  }

public:
  AsyncTaskManager& mManager; ///< Owner of this CacheImpl.

  // Keep cache iterators as list since we take tasks by FIFO as default.
  using TaskCacheContainer          = std::unordered_map<const AsyncTask*, std::list<AsyncTaskContainer::iterator>>;
  using RunningTaskCacheContainer   = std::unordered_map<const AsyncTask*, std::list<AsyncRunningTaskContainer::iterator>>;
  using CompletedTaskCacheContainer = std::unordered_map<const AsyncTask*, std::list<AsyncCompletedTaskContainer::iterator>>;

  TaskCacheContainer          mWaitingTasksCache;   ///< The cache of tasks and iterator for waiting to async process. Must be locked under mWaitingTasksMutex.
  RunningTaskCacheContainer   mRunningTasksCache;   ///< The cache of tasks and iterator for running tasks. Must be locked under mRunningTasksMutex.
  CompletedTaskCacheContainer mCompletedTasksCache; ///< The cache of tasks and iterator for completed async process. Must be locked under mCompletedTasksMutex.
};

// AsyncTaskManager

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
  mAvaliableLowPriorityTaskCounts(GetNumberOfLowPriorityThreads(NUMBER_OF_LOW_PRIORITY_THREADS_ENV, DEFAULT_NUMBER_OF_LOW_PRIORITY_THREADS, mTasks.GetElementCount())),
  mWaitingHighProirityTaskCounts(0u),
  mCacheImpl(new CacheImpl(*this)),
  mTrigger(new EventThreadCallback(MakeCallback(this, &AsyncTaskManager::TasksCompleted))),
  mProcessorRegistered(false)
{
}

AsyncTaskManager::~AsyncTaskManager()
{
  if(mProcessorRegistered && Dali::Adaptor::IsAvailable())
  {
    mProcessorRegistered = false;
    Dali::Adaptor::Get().UnregisterProcessor(*this);
  }

  // Join all threads.
  mTasks.Clear();

  // Remove cache impl after all threads are join.
  mCacheImpl.reset();

  // Remove tasks after CacheImpl removed
  mWaitingTasks.clear();
  mRunningTasks.clear();
  mCompletedTasks.clear();
}

void AsyncTaskManager::AddTask(AsyncTaskPtr task)
{
  if(task)
  {
    // Lock while adding task to the queue
    Mutex::ScopedLock lock(mWaitingTasksMutex);

    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "AddTask [%p]\n", task.Get());

    // push back into waiting queue.
    auto waitingIter = mWaitingTasks.insert(mWaitingTasks.end(), task);
    CacheImpl::InsertTaskCache(mCacheImpl->mWaitingTasksCache, task, waitingIter);

    if(task->GetPriorityType() == AsyncTask::PriorityType::HIGH)
    {
      // Increase the number of waiting tasks for high priority.
      ++mWaitingHighProirityTaskCounts;
    }

    {
      // For thread safety
      Mutex::ScopedLock lock(mRunningTasksMutex); // We can lock this mutex under mWaitingTasksMutex.

      // Finish all Running threads are working
      if(mRunningTasks.size() >= mTasks.GetElementCount())
      {
        return;
      }
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

  // Register Process (Since mTrigger execute too late timing if event thread running a lots of events.)
  if(!mProcessorRegistered && Dali::Adaptor::IsAvailable())
  {
    Dali::Adaptor::Get().RegisterProcessor(*this);
    mProcessorRegistered = true;
  }

  return;
}

void AsyncTaskManager::RemoveTask(AsyncTaskPtr task)
{
  if(task)
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "RemoveTask [%p]\n", task.Get());

    // Check whether we need to unregister processor.
    // If there is some non-empty queue exist, we don't need to unregister processor.
    bool needCheckUnregisterProcessor = true;

    {
      // Lock while remove task from the queue
      Mutex::ScopedLock lock(mWaitingTasksMutex);

      auto mapIter = mCacheImpl->mWaitingTasksCache.find(task.Get());
      if(mapIter != mCacheImpl->mWaitingTasksCache.end())
      {
        for(auto& iterator : mapIter->second)
        {
          DALI_ASSERT_DEBUG((*iterator) == task);
          if((*iterator)->GetPriorityType() == AsyncTask::PriorityType::HIGH)
          {
            // Decrease the number of waiting tasks for high priority.
            --mWaitingHighProirityTaskCounts;
          }
          mWaitingTasks.erase(iterator);
        }
        CacheImpl::EraseAllTaskCache(mCacheImpl->mWaitingTasksCache, task);
      }

      if(!mWaitingTasks.empty())
      {
        needCheckUnregisterProcessor = false;
      }
    }

    {
      // Lock while remove task from the queue
      Mutex::ScopedLock lock(mRunningTasksMutex);

      auto mapIter = mCacheImpl->mRunningTasksCache.find(task.Get());
      if(mapIter != mCacheImpl->mRunningTasksCache.end())
      {
        for(auto& iterator : mapIter->second)
        {
          DALI_ASSERT_DEBUG((*iterator).first == task);
          // We cannot erase container. Just mark as canceled.
          // Note : mAvaliableLowPriorityTaskCounts will be increased after process finished.
          (*iterator).second = RunningTaskState::CANCELED;
        }
      }

      if(!mRunningTasks.empty())
      {
        needCheckUnregisterProcessor = false;
      }
    }

    {
      // Lock while remove task from the queue
      Mutex::ScopedLock lock(mCompletedTasksMutex);

      auto mapIter = mCacheImpl->mCompletedTasksCache.find(task.Get());
      if(mapIter != mCacheImpl->mCompletedTasksCache.end())
      {
        for(auto& iterator : mapIter->second)
        {
          DALI_ASSERT_DEBUG(iterator->first == task);
          mCompletedTasks.erase(iterator);
        }
        CacheImpl::EraseAllTaskCache(mCacheImpl->mCompletedTasksCache, task);
      }

      if(!mCompletedTasks.empty())
      {
        needCheckUnregisterProcessor = false;
      }
    }

    // UnregisterProcessor required to lock mutex. Call this API only if required.
    if(needCheckUnregisterProcessor)
    {
      UnregisterProcessor();
    }
  }
}

AsyncTaskPtr AsyncTaskManager::PopNextCompletedTask()
{
  // Lock while popping task out from the queue
  Mutex::ScopedLock lock(mCompletedTasksMutex);

  AsyncTaskPtr nextCompletedTask = nullptr;

  while(!mCompletedTasks.empty())
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "PopNextCompletedTask, completed task count : [%zu]\n", mCompletedTasks.size());

    auto               next      = mCompletedTasks.begin();
    AsyncTaskPtr       nextTask  = next->first;
    CompletedTaskState taskState = next->second;
    CacheImpl::EraseTaskCache(mCacheImpl->mCompletedTasksCache, nextTask, next);
    mCompletedTasks.erase(next);

    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Completed task [%p] (callback required? : %d)\n", nextTask.Get(), taskState == CompletedTaskState::REQUIRE_CALLBACK);

    if(taskState == CompletedTaskState::REQUIRE_CALLBACK)
    {
      nextCompletedTask = nextTask;
      break;
    }
  }

  DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Pickup completed [%p]\n", nextCompletedTask.Get());

  return nextCompletedTask;
}

void AsyncTaskManager::UnregisterProcessor()
{
  if(mProcessorRegistered && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "UnregisterProcessor begin\n");
    // Keep processor at least 1 task exist.
    // Please be careful the order of mutex, to avoid dead lock.
    // TODO : Should we lock all mutex rightnow?
    Mutex::ScopedLock lockWait(mWaitingTasksMutex);
    if(mWaitingTasks.empty())
    {
      Mutex::ScopedLock lockRunning(mRunningTasksMutex); // We can lock this mutex under mWaitingTasksMutex.
      if(mRunningTasks.empty())
      {
        Mutex::ScopedLock lockComplete(mCompletedTasksMutex); // We can lock this mutex under mWaitingTasksMutex and mRunningTasksMutex.
        if(mCompletedTasks.empty())
        {
          mProcessorRegistered = false;
          Dali::Adaptor::Get().UnregisterProcessor(*this);
        }
      }
    }
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "UnregisterProcessor end (registed? %d)\n", mProcessorRegistered);
  }
}

void AsyncTaskManager::TasksCompleted()
{
  DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "TasksCompleted begin\n");
  while(AsyncTaskPtr task = PopNextCompletedTask())
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Execute callback [%p]\n", task.Get());
    CallbackBase::Execute(*(task->GetCompletedCallback()), task);
  }

  UnregisterProcessor();
  DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "TasksCompleted end\n");
}

void AsyncTaskManager::Process(bool postProcessor)
{
  TasksCompleted();
}

/// Worker thread called
AsyncTaskPtr AsyncTaskManager::PopNextTaskToProcess()
{
  // Lock while popping task out from the queue
  Mutex::ScopedLock lock(mWaitingTasksMutex);

  DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "PopNextTaskToProcess, waiting task count : [%zu]\n", mWaitingTasks.size());

  // pop out the next task from the queue
  AsyncTaskPtr nextTask = nullptr;

  // Fast cut if all waiting tasks are LOW priority, and we cannot excute low task anymore.
  if(mWaitingHighProirityTaskCounts == 0u && !mWaitingTasks.empty())
  {
    // For thread safety
    Mutex::ScopedLock lock(mRunningTasksMutex); // We can lock this mutex under mWaitingTasksMutex.

    if(mAvaliableLowPriorityTaskCounts == 0u)
    {
      // There are no avaliabe tasks to run now. Return nullptr.
      return nextTask;
    }
  }

  for(auto iter = mWaitingTasks.begin(), endIter = mWaitingTasks.end(); iter != endIter; ++iter)
  {
    if((*iter)->IsReady())
    {
      const auto priorityType  = (*iter)->GetPriorityType();
      bool       taskAvaliable = priorityType == AsyncTask::PriorityType::HIGH; // Task always valid if it's priority is high
      if(!taskAvaliable)
      {
        // For thread safety
        Mutex::ScopedLock lock(mRunningTasksMutex); // We can lock this mutex under mWaitingTasksMutex.

        taskAvaliable = (mAvaliableLowPriorityTaskCounts > 0u); // priority is low, but we can use it.
      }

      if(taskAvaliable)
      {
        nextTask = *iter;

        // Add Running queue
        {
          // Lock while popping task out from the queue
          Mutex::ScopedLock lock(mRunningTasksMutex); // We can lock this mutex under mWaitingTasksMutex.

          DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Waiting -> Running [%p]\n", nextTask.Get());

          auto runningIter = mRunningTasks.insert(mRunningTasks.end(), std::make_pair(nextTask, RunningTaskState::RUNNING));
          CacheImpl::InsertTaskCache(mCacheImpl->mRunningTasksCache, nextTask, runningIter);

          CacheImpl::EraseTaskCache(mCacheImpl->mWaitingTasksCache, nextTask, iter);
          mWaitingTasks.erase(iter);

          // Decrease avaliable task counts if it is low priority
          if(priorityType == AsyncTask::PriorityType::LOW)
          {
            // We are under running task mutex. We can decrease it.
            --mAvaliableLowPriorityTaskCounts;
          }
        }

        if(priorityType == AsyncTask::PriorityType::HIGH)
        {
          // Decrease the number of waiting tasks for high priority.
          --mWaitingHighProirityTaskCounts;
        }
        break;
      }
    }
  }

  DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Pickup process [%p]\n", nextTask.Get());

  return nextTask;
}

/// Worker thread called
void AsyncTaskManager::CompleteTask(AsyncTaskPtr&& task)
{
  bool notify = false;

  if(task)
  {
    const bool needTrigger = task->GetCallbackInvocationThread() == AsyncTask::ThreadType::MAIN_THREAD;

    // Lock while check validation of task.
    {
      Mutex::ScopedLock lock(mRunningTasksMutex);

      auto mapIter = mCacheImpl->mRunningTasksCache.find(task.Get());
      if(mapIter != mCacheImpl->mRunningTasksCache.end())
      {
        const auto cacheIter = mapIter->second.begin();
        DALI_ASSERT_ALWAYS(cacheIter != mapIter->second.end());

        const auto iter = *cacheIter;
        DALI_ASSERT_DEBUG(iter->first == task);
        if(iter->second == RunningTaskState::RUNNING)
        {
          // This task is valid.
          notify = true;
        }
      }

      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "CompleteTask [%p] (is notify? : %d)\n", task.Get(), notify);
    }

    // We should execute this tasks complete callback out of mutex
    if(notify && task->GetCallbackInvocationThread() == AsyncTask::ThreadType::WORKER_THREAD)
    {
      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Execute callback on worker thread [%p]\n", task.Get());
      CallbackBase::Execute(*(task->GetCompletedCallback()), task);
    }

    // Lock while adding task to the queue
    {
      Mutex::ScopedLock lock(mRunningTasksMutex);

      auto mapIter = mCacheImpl->mRunningTasksCache.find(task.Get());
      if(mapIter != mCacheImpl->mRunningTasksCache.end())
      {
        const auto cacheIter = mapIter->second.begin();
        DALI_ASSERT_ALWAYS(cacheIter != mapIter->second.end());

        const auto iter         = *cacheIter;
        const auto priorityType = iter->first->GetPriorityType();
        // Increase avaliable task counts if it is low priority
        if(priorityType == AsyncTask::PriorityType::LOW)
        {
          // We are under running task mutex. We can increase it.
          ++mAvaliableLowPriorityTaskCounts;
        }

        // Move task into completed, for ensure that AsyncTask destroy at main thread.
        {
          Mutex::ScopedLock lock(mCompletedTasksMutex); // We can lock this mutex under mRunningTasksMutex.

          const bool callbackRequired = notify && (task->GetCallbackInvocationThread() == AsyncTask::ThreadType::MAIN_THREAD);

          DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Running -> Completed [%p] (callback required? : %d)\n", task.Get(), callbackRequired);

          auto completedIter = mCompletedTasks.insert(mCompletedTasks.end(), std::make_pair(task, callbackRequired ? CompletedTaskState::REQUIRE_CALLBACK : CompletedTaskState::SKIP_CALLBACK));
          CacheImpl::InsertTaskCache(mCacheImpl->mCompletedTasksCache, task, completedIter);

          CacheImpl::EraseTaskCache(mCacheImpl->mRunningTasksCache, task, iter);
          mRunningTasks.erase(iter);

          // Now, task is invalidate.
          task.Reset();
        }
      }
    }

    // Wake up the main thread
    if(needTrigger)
    {
      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Trigger main thread\n");
      mTrigger->Trigger();
    }
  }
}

// AsyncTaskManager::TaskHelper

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
