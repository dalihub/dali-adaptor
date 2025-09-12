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

// CLASS HEADER
#include <dali/internal/system/common/async-task-manager-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/thread-settings.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/environment-variables.h>

#include <unordered_map>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr auto FORCE_TRIGGER_THRESHOLD = 128u; ///< Trigger TasksCompleted() forcely if the number of completed task contain too much.

constexpr auto DEFAULT_NUMBER_OF_ASYNC_THREADS = size_t{8u};

// The number of threads for low priority task.
constexpr auto DEFAULT_NUMBER_OF_LOW_PRIORITY_THREADS = size_t{6u};

size_t GetNumberOfThreads(size_t defaultValue)
{
  auto           numberString          = EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ASYNC_MANAGER_THREAD_POOL_SIZE);
  auto           numberOfThreads       = numberString ? std::strtoul(numberString, nullptr, 10) : 0;
  constexpr auto MAX_NUMBER_OF_THREADS = 16u;
  DALI_ASSERT_DEBUG(numberOfThreads <= MAX_NUMBER_OF_THREADS);
  return (numberOfThreads > 0 && numberOfThreads <= MAX_NUMBER_OF_THREADS) ? numberOfThreads : defaultValue;
}

size_t GetNumberOfLowPriorityThreads(size_t defaultValue, size_t maxValue)
{
  auto numberString    = EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ASYNC_MANAGER_LOW_PRIORITY_THREAD_POOL_SIZE);
  auto numberOfThreads = numberString ? std::strtoul(numberString, nullptr, 10) : 0;
  DALI_ASSERT_DEBUG(numberOfThreads <= maxValue);
  return (numberOfThreads > 0 && numberOfThreads <= maxValue) ? numberOfThreads : std::min(defaultValue, maxValue);
}

#if defined(DEBUG_ENABLED)
Debug::Filter* gAsyncTasksManagerLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_ASYNC_TASK_MANAGER");

uint32_t gThreadId = 0u; // Only for debug

/**
 * @brief Get the Task Name.
 * Note that we can get const char* from std::string_view as data() since it will be const class.
 *
 * @param task The task what we want to get the name.
 * @return The name of task, or (nil) if task is invalid.
 */
const char* GetTaskName(AsyncTaskPtr task)
{
  // Note
  return task ? task->GetTaskName().data() : "(nil)";
}

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
      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Thread[%u] Process task [%p][%s]\n", threadId, task.Get(), GetTaskName(task));
      task->Process();
      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Thread[%u] Complete task [%p][%s]\n", threadId, task.Get(), GetTaskName(task));
      if(!mDestroyThread)
      {
        mAsyncTaskManager.CompleteTask(std::move(task));
      }
    }
  }
}

// AsyncTaskManager::TasksCompletedImpl

struct AsyncTaskManager::TasksCompletedImpl
{
  TasksCompletedImpl(AsyncTaskManager& manager, EventThreadCallback* trigger)
  : mManager(manager),
    mTrigger(trigger),
    mEmitCompletedTaskTriggered(false)
  {
  }

public:
  /**
   * @brief Create new tasks completed id and.
   * @post AppendTaskTrace or CheckTasksCompletedCallbackCompleted should be called.
   * @param[in] callback The callback that want to be executed when we notify that all tasks completed.
   */
  Dali::AsyncTaskManager::TasksCompletedId GenerateTasksCompletedId(CallbackBase* callback)
  {
    // Lock while adding tasks completed callback list to the queue
    Mutex::ScopedLock lock(mTasksCompletedCallbacksMutex);

    auto id = mTasksCompletedCount++;
    DALI_ASSERT_ALWAYS(mTasksCompletedCallbackList.find(id) == mTasksCompletedCallbackList.end());

    mTasksCompletedCallbackList.insert({id, CallbackData(callback)});

    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "GenerateTasksCompletedId id[%u] callback[%p]\n", id, callback);
    return id;
  }

  /**
   * @brief Append task that will be trace.
   * @post RemoveTaskTrace should be called.
   * @param[in] id The id of tasks completed.
   * @param[in] task The task want to trace.
   */
  void AppendTaskTrace(Dali::AsyncTaskManager::TasksCompletedId id, AsyncTaskPtr task)
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "AppendTaskTrace id[%u] task[%p][%s]\n", id, task.Get(), GetTaskName(task));

    // Lock while adding tasks completed callback list to the queue
    Mutex::ScopedLock lock(mTasksCompletedCallbacksMutex);

    auto iter = mTasksCompletedCallbackList.find(id);
    if(iter == mTasksCompletedCallbackList.end())
    {
      // This task is already erased. Ignore.
      return;
    }

    auto& callbackData = iter->second;

    auto jter = callbackData.mTasks.find(task.Get());

    if(jter != callbackData.mTasks.end())
    {
      // Increase reference count.
      ++(jter->second);
    }
    else
    {
      callbackData.mTasks.insert({task.Get(), 1u});
    }
  }

  /**
   * @brief Remove all task that were traced.
   * @param[in] task The task want to remove trace.
   * @param[in] taskCount The number of tasks that will be removed.
   */
  void RemoveTaskTrace(AsyncTaskPtr task, uint32_t count = 1u)
  {
    if(count == 0u)
    {
      return;
    }
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "RemoveTaskTrace task[%p][%s] remove count[%u]\n", task.Get(), GetTaskName(task), count);

    // Lock while removing tasks completed callback list to the queue
    Mutex::ScopedLock lock(mTasksCompletedCallbacksMutex);

    for(auto iter = mTasksCompletedCallbackList.begin(); iter != mTasksCompletedCallbackList.end();)
    {
      auto& callbackData      = iter->second;
      bool  eraseCallbackData = false;

      auto jter = callbackData.mTasks.find(task.Get());

      if(jter != callbackData.mTasks.end())
      {
        DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "RemoveTaskTrace id[%u] task[%p][%s], current refcount[%u]\n", iter->first, task.Get(), GetTaskName(task), (jter->second));

        if(jter->second <= count)
        {
          callbackData.mTasks.erase(jter);

          DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "RemoveTaskTrace id[%u] task erased. remained tasks[%zu]", iter->first, callbackData.mTasks.size());

          if(callbackData.mTasks.empty())
          {
            eraseCallbackData = true;

            // Move callback base into list.
            // (To avoid task container changed during callback emit)
            RegisterTasksCompletedCallback(std::move(callbackData.mCallback), iter->first);

            DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "id[%u] completed!\n", iter->first);

            iter = mTasksCompletedCallbackList.erase(iter);
          }
        }
        else
        {
          jter->second -= count;
        }
      }

      if(!eraseCallbackData)
      {
        ++iter;
      }
    }
  }

  /**
   * @brief Check whether current TasksCompletedId completed or not.
   * @param[in] id The id of tasks completed.
   * @return True if all tasks are completed so we need to execute callback soon. False otherwise.
   */
  bool CheckTasksCompletedCallbackCompleted(Dali::AsyncTaskManager::TasksCompletedId id)
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "CheckTasksCompletedCallbackCompleted[%u]\n", id);

    // Lock while removing tasks completed callback list to the queue
    Mutex::ScopedLock lock(mTasksCompletedCallbacksMutex);

    auto iter = mTasksCompletedCallbackList.find(id);
    if(iter != mTasksCompletedCallbackList.end())
    {
      auto& callbackData = iter->second;
      if(callbackData.mTasks.empty())
      {
        // Move callback base into list.
        // (To avoid task container changed during callback emit)
        RegisterTasksCompletedCallback(std::move(callbackData.mCallback), iter->first);

        DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "id[%u] completed!\n", iter->first);

        iter = mTasksCompletedCallbackList.erase(iter);

        return true;
      }
    }

    return false;
  }

  /**
   * @brief Remove taskS completed callbacks by id.
   * @param[in] id The id of taskS completed.
   * @return True if taskS completed id removed. False otherwise.
   */
  bool RemoveTasksCompleted(Dali::AsyncTaskManager::TasksCompletedId id)
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "RemoveTasksCompleted[%u]\n", id);

    // Lock while removing taskS completed callback list to the queue
    Mutex::ScopedLock lock(mTasksCompletedCallbacksMutex);

    auto iter = mTasksCompletedCallbackList.find(id);
    if(iter == mTasksCompletedCallbackList.end())
    {
      // This task is already erased, or completed.
      // Erase from completed excute callback list.

      // Lock while removing excute callback list to the queue
      Mutex::ScopedLock lock(mExcuteCallbacksMutex);

      for(auto iter = mExcuteCallbackList.begin(); iter != mExcuteCallbackList.end();)
      {
        if(iter->second == id)
        {
          iter = mExcuteCallbackList.erase(iter);

          DALI_LOG_DEBUG_INFO("CompletedCallback id[%u] completed, but not execute callback.\n", id);
          return true;
        }
        else
        {
          ++iter;
        }
      }

      // This task is alread erased and completed. Ignore.
      DALI_LOG_DEBUG_INFO("CompletedCallback id[%u] is alread erased and completed.\n", id);
      return false;
    }

    uint32_t remainedTasksCount = 0u;
    for(const auto& taskPair : iter->second.mTasks)
    {
      remainedTasksCount += taskPair.second;
    }
    DALI_LOG_DEBUG_INFO("CompletedCallback id[%u] remain task count : %u.\n", id, remainedTasksCount);

    mTasksCompletedCallbackList.erase(iter);

    return true;
  }

  /**
   * @brief Emit all completed callbacks.
   * @note This API should be called at event thread.
   */
  void EmitCompletedTasks()
  {
    ExecuteCallbackContainer executeCallbackList;
    {
      // Lock while removing excute callback list to the queue
      Mutex::ScopedLock lock(mExcuteCallbacksMutex);

      mEmitCompletedTaskTriggered = false;

      // Copy callback lists, for let we execute callbacks out of mutex
      executeCallbackList = std::move(mExcuteCallbackList);
      mExcuteCallbackList.clear();
    }

    if(!executeCallbackList.empty())
    {
      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Excute callback count[%zu]\n", executeCallbackList.size());
      // Execute all callbacks
      for(auto&& callbackPair : executeCallbackList)
      {
        auto& callback = callbackPair.first;
        auto  id       = callbackPair.second;

        DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Excute taskS completed callback[%p] for id[%u]\n", callback.get(), id);

        Dali::CallbackBase::Execute(*callback, id);
      }

      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Excute callback end\n");
    }
  }

  /**
   * @brief Check whether there is some completed signal what we need to trace, or not.
   * @return True if mTasksCompletedCallbackList is not empty. False otherwise.
   */
  bool IsTasksCompletedCallbackExist()
  {
    Mutex::ScopedLock lock(mTasksCompletedCallbacksMutex);
    return !mTasksCompletedCallbackList.empty();
  }

  /**
   * @brief Check whether there is some completed signal what we need to execute, or not.
   * @return True if mExcuteCallbackList is not empty. False otherwise.
   */
  bool IsExecuteCallbackExist()
  {
    Mutex::ScopedLock lock(mExcuteCallbacksMutex);
    return !mExcuteCallbackList.empty();
  }

private:
  void RegisterTasksCompletedCallback(std::unique_ptr<CallbackBase> callback, Dali::AsyncTaskManager::TasksCompletedId id)
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "TasksCompleted[%u] need to be execute with callback[%p]\n", id, callback.get());

    // Lock while adding excute callback list to the queue
    Mutex::ScopedLock lock(mExcuteCallbacksMutex);
    DALI_LOG_DEBUG_INFO("CompletedCallback id[%u] need to be execute.\n", id);

    mExcuteCallbackList.emplace_back(std::move(callback), id);

    if(!mEmitCompletedTaskTriggered)
    {
      mEmitCompletedTaskTriggered = true;

      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Trigger processor\n");
      mTrigger->Trigger();
    }
  }

private:
  struct CallbackData
  {
  public:
    CallbackData(CallbackBase* callback)
    : mCallback(callback),
      mTasks()
    {
    }

    CallbackData(CallbackData&& rhs) noexcept
    : mCallback(std::move(rhs.mCallback)),
      mTasks(std::move(rhs.mTasks))
    {
    }

    CallbackData& operator=(CallbackData&& rhs) noexcept
    {
      if(this != &rhs)
      {
        mCallback = std::move(rhs.mCallback);
        mTasks    = std::move(rhs.mTasks);
      }

      return *this;
    }

  private:
    // Delete copy operator.
    CallbackData(const CallbackData& rhs)            = delete;
    CallbackData& operator=(const CallbackData& rhs) = delete;

  public:
    std::unique_ptr<CallbackBase>                  mCallback;
    std::unordered_map<const AsyncTask*, uint32_t> mTasks;
  };

private:
  AsyncTaskManager&    mManager; ///< Owner of this CacheImpl.
  EventThreadCallback* mTrigger; ///< EventThread callback trigger. (Not owned.)

  Dali::AsyncTaskManager::TasksCompletedId mTasksCompletedCount{0u};

  using TasksCompletedContainer = std::unordered_map<Dali::AsyncTaskManager::TasksCompletedId, CallbackData>;
  TasksCompletedContainer mTasksCompletedCallbackList;

  using ExecuteCallbackContainer = std::vector<std::pair<std::unique_ptr<CallbackBase>, Dali::AsyncTaskManager::TasksCompletedId>>;
  ExecuteCallbackContainer mExcuteCallbackList;

  Dali::Mutex mTasksCompletedCallbacksMutex; ///< Mutex for mTasksCompletedCallbackList. We can lock mExcuteCallbacksMutex under this scope.
  Dali::Mutex mExcuteCallbacksMutex;         ///< Mutex for mExcuteCallbackList.

  bool mEmitCompletedTaskTriggered : 1;
};

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
  TaskCacheContainer          mNotReadyTasksCache;  ///< The cache of tasks and iterator for waiting for ready to async process. Must be locked under mWaitingTasksMutex.
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
: mTasks(GetNumberOfThreads(DEFAULT_NUMBER_OF_ASYNC_THREADS), [&]()
{ return TaskHelper(*this); }),
  mAvaliableLowPriorityTaskCounts(GetNumberOfLowPriorityThreads(DEFAULT_NUMBER_OF_LOW_PRIORITY_THREADS, mTasks.GetElementCount())),
  mWaitingHighProirityTaskCounts(0u),
  mTrigger(new EventThreadCallback(MakeCallback(this, &AsyncTaskManager::TasksCompleted))),
  mTasksCompletedImpl(new TasksCompletedImpl(*this, mTrigger.get())),
  mCacheImpl(new CacheImpl(*this)),
  mProcessorRegistered(false)
{
  DALI_LOG_DEBUG_INFO("AsyncTaskManager Trigger Id(%d)\n", mTrigger->GetId());
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

  // Remove task completed impl and cache impl after all threads are join.
  mTasksCompletedImpl.reset();
  mCacheImpl.reset();

  // Remove tasks after CacheImpl removed
  mWaitingTasks.clear();
  mRunningTasks.clear();
  mCompletedTasks.clear();
}

/// Main + Worker thread called
void AsyncTaskManager::AddTask(AsyncTaskPtr task)
{
  if(task)
  {
    // Lock while adding task to the queue
    Mutex::ScopedLock lock(mWaitingTasksMutex);

    // Keep this value as stack memory, for thread safety
    const bool isReady = task->IsReady();
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "AddTask [%p][%s], IsReady(%d)\n", task.Get(), GetTaskName(task), isReady);

    if(DALI_LIKELY(isReady))
    {
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
    else
    {
      // push back into waiting queue.
      auto notReadyIter = mNotReadyTasks.insert(mNotReadyTasks.end(), task);
      CacheImpl::InsertTaskCache(mCacheImpl->mNotReadyTasksCache, task, notReadyIter);
      return;
    }
  }

  {
    Mutex::ScopedLock lock(mTasksMutex);
    size_t            count = mTasks.GetElementCount();
    size_t            index = 0;
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
  }

  // Register Process (Since mTrigger execute too late timing if event thread running a lots of events.)
  RegisterProcessor();

  return;
}

void AsyncTaskManager::RemoveTask(AsyncTaskPtr task)
{
  if(task)
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "RemoveTask [%p][%s]\n", task.Get(), GetTaskName(task));

    // Check whether we need to unregister processor.
    // If there is some non-empty queue exist, we don't need to unregister processor.
    bool needCheckUnregisterProcessor = true;

    uint32_t removedCount = 0u;

    {
      // Lock while remove task from the queue
      Mutex::ScopedLock lock(mWaitingTasksMutex);

      auto mapIter = mCacheImpl->mWaitingTasksCache.find(task.Get());
      if(mapIter != mCacheImpl->mWaitingTasksCache.end())
      {
        for(auto& iterator : mapIter->second)
        {
          DALI_ASSERT_DEBUG((*iterator) == task);
          if((*iterator)->GetPriorityType() == AsyncTask::PriorityType::HIGH && mWaitingHighProirityTaskCounts > 0u)
          {
            // Decrease the number of waiting tasks for high priority.
            --mWaitingHighProirityTaskCounts;
          }
          mWaitingTasks.erase(iterator);
          ++removedCount;
        }
        CacheImpl::EraseAllTaskCache(mCacheImpl->mWaitingTasksCache, task);
      }

      auto mapIter2 = mCacheImpl->mNotReadyTasksCache.find(task.Get());
      if(mapIter2 != mCacheImpl->mNotReadyTasksCache.end())
      {
        for(auto& iterator : mapIter2->second)
        {
          DALI_ASSERT_DEBUG((*iterator) == task);
          mNotReadyTasks.erase(iterator);
          ++removedCount;
        }
        CacheImpl::EraseAllTaskCache(mCacheImpl->mNotReadyTasksCache, task);
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
          if((*iterator).second == RunningTaskState::RUNNING)
          {
            (*iterator).second = RunningTaskState::CANCELED;
            ++removedCount;
          }
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
          DALI_ASSERT_DEBUG((*iterator).first == task);
          if((*iterator).second == CompletedTaskState::REQUIRE_CALLBACK)
          {
            ++removedCount;
          }
          mCompletedTasks.erase(iterator);
        }
        CacheImpl::EraseAllTaskCache(mCacheImpl->mCompletedTasksCache, task);
      }

      if(!mCompletedTasks.empty())
      {
        needCheckUnregisterProcessor = false;
      }
    }

    // Remove TasksCompleted callback trace
    if(removedCount > 0u && mTasksCompletedImpl->IsTasksCompletedCallbackExist())
    {
      mTasksCompletedImpl->RemoveTaskTrace(task, removedCount);
    }

    // UnregisterProcessor required to lock mutex. Call this API only if required.
    if(needCheckUnregisterProcessor)
    {
      UnregisterProcessor();
    }
  }
}

/// Main + Worker thread called
void AsyncTaskManager::NotifyToTaskReady(AsyncTaskPtr task)
{
  if(task)
  {
    // Lock while adding task to the queue
    Mutex::ScopedLock lock(mWaitingTasksMutex);
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "NotifyToTaskReady [%p][%s]\n", task.Get(), GetTaskName(task));

    auto mapIter = mCacheImpl->mNotReadyTasksCache.find(task.Get());
    if(mapIter != mCacheImpl->mNotReadyTasksCache.end())
    {
      uint32_t removedCount = 0u;
      for(auto& iterator : mapIter->second)
      {
        DALI_ASSERT_DEBUG((*iterator) == task);
        mNotReadyTasks.erase(iterator);
        ++removedCount;
      }
      CacheImpl::EraseAllTaskCache(mCacheImpl->mNotReadyTasksCache, task);

      // push back into waiting queue.
      while(removedCount > 0u)
      {
        --removedCount;
        auto waitingIter = mWaitingTasks.insert(mWaitingTasks.end(), task);
        CacheImpl::InsertTaskCache(mCacheImpl->mWaitingTasksCache, task, waitingIter);

        if(task->GetPriorityType() == AsyncTask::PriorityType::HIGH)
        {
          // Increase the number of waiting tasks for high priority.
          ++mWaitingHighProirityTaskCounts;
        }
      }
    }
    else
    {
      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Already ready. Ignore [%p][%s]\n", task.Get(), GetTaskName(task));
      // Already waiting now. Ignore.
      return;
    }
  }

  {
    Mutex::ScopedLock lock(mTasksMutex);
    size_t            count = mTasks.GetElementCount();
    size_t            index = 0;
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
  }
}

Dali::AsyncTaskManager::TasksCompletedId AsyncTaskManager::SetCompletedCallback(CallbackBase* callback, Dali::AsyncTaskManager::CompletedCallbackTraceMask mask)
{
  // mTasksCompletedImpl will take ownership of callback.
  Dali::AsyncTaskManager::TasksCompletedId tasksCompletedId = mTasksCompletedImpl->GenerateTasksCompletedId(callback);

  uint32_t addedTaskCount = 0u; ///< Count the number of task tracing now.

  DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "SetCompletedCallback id : %u, mask : %d\n", tasksCompletedId, static_cast<int32_t>(mask));

  // Please be careful the order of mutex, to avoid dead lock.
  {
    Mutex::ScopedLock lockWait(mWaitingTasksMutex);
    {
      Mutex::ScopedLock lockRunning(mRunningTasksMutex); // We can lock this mutex under mWaitingTasksMutex.
      {
        Mutex::ScopedLock lockComplete(mCompletedTasksMutex); // We can lock this mutex under mWaitingTasksMutex and mRunningTasksMutex.

        // Collect all tasks from waiting tasks
        for(auto& task : mWaitingTasks)
        {
          auto checkMask = (task->GetCallbackInvocationThread() == Dali::AsyncTask::ThreadType::MAIN_THREAD ? Dali::AsyncTaskManager::CompletedCallbackTraceMask::THREAD_MASK_MAIN : Dali::AsyncTaskManager::CompletedCallbackTraceMask::THREAD_MASK_WORKER) |
                           (task->GetPriorityType() == Dali::AsyncTask::PriorityType::HIGH ? Dali::AsyncTaskManager::CompletedCallbackTraceMask::PRIORITY_MASK_HIGH : Dali::AsyncTaskManager::CompletedCallbackTraceMask::PRIORITY_MASK_LOW);

          if((checkMask & mask) == checkMask)
          {
            ++addedTaskCount;
            mTasksCompletedImpl->AppendTaskTrace(tasksCompletedId, task);
          }
        }

        // Collect all tasks from not ready waiting tasks
        for(auto& task : mNotReadyTasks)
        {
          auto checkMask = (task->GetCallbackInvocationThread() == Dali::AsyncTask::ThreadType::MAIN_THREAD ? Dali::AsyncTaskManager::CompletedCallbackTraceMask::THREAD_MASK_MAIN : Dali::AsyncTaskManager::CompletedCallbackTraceMask::THREAD_MASK_WORKER) |
                           (task->GetPriorityType() == Dali::AsyncTask::PriorityType::HIGH ? Dali::AsyncTaskManager::CompletedCallbackTraceMask::PRIORITY_MASK_HIGH : Dali::AsyncTaskManager::CompletedCallbackTraceMask::PRIORITY_MASK_LOW);

          if((checkMask & mask) == checkMask)
          {
            ++addedTaskCount;
            mTasksCompletedImpl->AppendTaskTrace(tasksCompletedId, task);
          }
        }

        // Collect all tasks from running tasks
        for(auto& taskPair : mRunningTasks)
        {
          // Trace only if it is running now.
          if(taskPair.second == RunningTaskState::RUNNING)
          {
            auto& task      = taskPair.first;
            auto  checkMask = (task->GetCallbackInvocationThread() == Dali::AsyncTask::ThreadType::MAIN_THREAD ? Dali::AsyncTaskManager::CompletedCallbackTraceMask::THREAD_MASK_MAIN : Dali::AsyncTaskManager::CompletedCallbackTraceMask::THREAD_MASK_WORKER) |
                             (task->GetPriorityType() == Dali::AsyncTask::PriorityType::HIGH ? Dali::AsyncTaskManager::CompletedCallbackTraceMask::PRIORITY_MASK_HIGH : Dali::AsyncTaskManager::CompletedCallbackTraceMask::PRIORITY_MASK_LOW);

            if((checkMask & mask) == checkMask)
            {
              ++addedTaskCount;
              mTasksCompletedImpl->AppendTaskTrace(tasksCompletedId, task);
            }
          }
        }

        // Collect all tasks from complete tasks
        for(auto& taskPair : mCompletedTasks)
        {
          // Trace only if it is need callback.
          // Note : There are two CompletedTaskState::SKIP_CALLBACK cases, worker thread invocation and canceled cases.
          //        If worker thread invocation, than it already remove trace at completed timing.
          //        If canceled cases, we don't append trace at running tasks already.
          //        So, we don't need to trace for SKIP_CALLBACK cases.
          if(taskPair.second == CompletedTaskState::REQUIRE_CALLBACK)
          {
            auto& task      = taskPair.first;
            auto  checkMask = (task->GetCallbackInvocationThread() == Dali::AsyncTask::ThreadType::MAIN_THREAD ? Dali::AsyncTaskManager::CompletedCallbackTraceMask::THREAD_MASK_MAIN : Dali::AsyncTaskManager::CompletedCallbackTraceMask::THREAD_MASK_WORKER) |
                             (task->GetPriorityType() == Dali::AsyncTask::PriorityType::HIGH ? Dali::AsyncTaskManager::CompletedCallbackTraceMask::PRIORITY_MASK_HIGH : Dali::AsyncTaskManager::CompletedCallbackTraceMask::PRIORITY_MASK_LOW);

            if((checkMask & mask) == checkMask)
            {
              ++addedTaskCount;
              mTasksCompletedImpl->AppendTaskTrace(tasksCompletedId, task);
            }
          }
        }
      }
    }
  }

  // If there is nothing to check task, just excute callback right now.
  if(addedTaskCount == 0u)
  {
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "CompletedCallback id[%u] executed now due to no task exist\n", tasksCompletedId);

    mTasksCompletedImpl->CheckTasksCompletedCallbackCompleted(tasksCompletedId);
  }
  DALI_LOG_DEBUG_INFO("CompletedCallback id[%u] wait %u tasks completed\n", tasksCompletedId, addedTaskCount);
  return tasksCompletedId;
}

bool AsyncTaskManager::RemoveCompletedCallback(Dali::AsyncTaskManager::TasksCompletedId tasksCompletedId)
{
  return mTasksCompletedImpl->RemoveTasksCompleted(tasksCompletedId);
}

AsyncTaskPtr AsyncTaskManager::PopNextCompletedTask()
{
  std::vector<AsyncTaskPtr> ignoredTaskList; ///< To keep asyncTask reference so we can ensure that destructor called out of mutex.

  AsyncTaskPtr nextCompletedTask = nullptr;
  {
    // Lock while popping task out from the queue
    Mutex::ScopedLock lock(mCompletedTasksMutex);

    while(!mCompletedTasks.empty())
    {
      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "PopNextCompletedTask, completed task count : [%zu]\n", mCompletedTasks.size());

      auto               next      = mCompletedTasks.begin();
      AsyncTaskPtr       nextTask  = next->first;
      CompletedTaskState taskState = next->second;
      CacheImpl::EraseTaskCache(mCacheImpl->mCompletedTasksCache, nextTask, next);
      mCompletedTasks.erase(next);

      DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Completed task [%p][%s] (callback required? : %d)\n", nextTask.Get(), GetTaskName(nextTask), taskState == CompletedTaskState::REQUIRE_CALLBACK);

      if(taskState == CompletedTaskState::REQUIRE_CALLBACK)
      {
        nextCompletedTask = nextTask;
        break;
      }

      ignoredTaskList.push_back(nextTask);
    }

    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Pickup completed [%p][%s]\n", nextCompletedTask.Get(), GetTaskName(nextCompletedTask));
  }

  return nextCompletedTask;
}

void AsyncTaskManager::RegisterProcessor()
{
  if(!mProcessorRegistered && Dali::Adaptor::IsAvailable())
  {
    Dali::Adaptor::Get().RegisterProcessor(*this);
    mProcessorRegistered = true;
  }
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
    DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Execute callback [%p][%s]\n", task.Get(), GetTaskName(task));
    CallbackBase::Execute(*(task->GetCompletedCallback()), task);

    // Remove TasksCompleted callback trace
    if(mTasksCompletedImpl->IsTasksCompletedCallbackExist())
    {
      mTasksCompletedImpl->RemoveTaskTrace(task);
    }
  }

  UnregisterProcessor();
  DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "TasksCompleted end\n");

  mTasksCompletedImpl->EmitCompletedTasks();
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
    if(DALI_LIKELY((*iter)->IsReady()))
    {
      const auto priorityType  = (*iter)->GetPriorityType();
      bool       taskAvaliable = priorityType == AsyncTask::PriorityType::HIGH; // Task always valid if it's priority is high
      if(!taskAvaliable)
      {
        // For thread safety
        Mutex::ScopedLock lock(mRunningTasksMutex); // We can lock this mutex under mWaitingTasksMutex.

        taskAvaliable = (mAvaliableLowPriorityTaskCounts > 0u); // priority is low, but we can use it.
      }

      // Check whether we try to running same task at multiple threads.
      if(taskAvaliable)
      {
        Mutex::ScopedLock lock(mRunningTasksMutex); // We can lock this mutex under mWaitingTasksMutex.
        auto              mapIter = mCacheImpl->mRunningTasksCache.find((*iter).Get());
        if(mapIter != mCacheImpl->mRunningTasksCache.end())
        {
          if(!mapIter->second.empty())
          {
            // Some other thread running this tasks now. Ignore it.
            DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Some other thread running this task [%p][%s]\n", (*iter).Get(), GetTaskName(*iter));
            taskAvaliable = false;
          }
        }
      }

      if(taskAvaliable)
      {
        nextTask = *iter;

        // Add Running queue
        {
          // Lock while popping task out from the queue
          Mutex::ScopedLock lock(mRunningTasksMutex); // We can lock this mutex under mWaitingTasksMutex.

          DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Waiting -> Running [%p][%s]\n", nextTask.Get(), GetTaskName(nextTask));

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

        if(priorityType == AsyncTask::PriorityType::HIGH && mWaitingHighProirityTaskCounts > 0u)
        {
          // Decrease the number of waiting tasks for high priority.
          --mWaitingHighProirityTaskCounts;
        }
        break;
      }
    }
    else
    {
      DALI_LOG_ERROR("Not ready task is in wating queue! Something wrong!\n");
    }
  }

  DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::General, "Pickup process [%p][%s]\n", nextTask.Get(), GetTaskName(nextTask));

  return nextTask;
}

/// Worker thread called
void AsyncTaskManager::CompleteTask(AsyncTaskPtr&& task)
{
  if(task)
  {
    bool needTrigger = false;

    // Check now whether we need to execute callback or not, for worker thread cases.
    if(task->GetCallbackInvocationThread() == AsyncTask::ThreadType::WORKER_THREAD)
    {
      bool notify = false;

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

        DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "CompleteTask [%p][%s] (is notify? : %d)\n", task.Get(), GetTaskName(task), notify);
      }

      // We should execute this tasks complete callback out of mutex
      if(notify)
      {
        DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Execute callback on worker thread [%p][%s]\n", task.Get(), GetTaskName(task));
        CallbackBase::Execute(*(task->GetCompletedCallback()), task);

        // We need to remove task trace now.
        if(mTasksCompletedImpl->IsTasksCompletedCallbackExist())
        {
          mTasksCompletedImpl->RemoveTaskTrace(task);

          if(mTasksCompletedImpl->IsExecuteCallbackExist())
          {
            // We need to call EmitCompletedTasks(). Trigger main thread.
            needTrigger = true;
          }
        }
      }
    }

    // Lock while adding task to the queue
    {
      bool notify = false;

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

          needTrigger |= callbackRequired;

          DALI_LOG_INFO(gAsyncTasksManagerLogFilter, Debug::Verbose, "Running -> Completed [%p][%s] (callback required? : %d)\n", task.Get(), GetTaskName(task), callbackRequired);

          auto completedIter = mCompletedTasks.insert(mCompletedTasks.end(), std::make_pair(task, callbackRequired ? CompletedTaskState::REQUIRE_CALLBACK : CompletedTaskState::SKIP_CALLBACK));
          CacheImpl::InsertTaskCache(mCacheImpl->mCompletedTasksCache, task, completedIter);

          CacheImpl::EraseTaskCache(mCacheImpl->mRunningTasksCache, task, iter);
          mRunningTasks.erase(iter);

          if(!needTrigger)
          {
            needTrigger |= (mCompletedTasks.size() >= FORCE_TRIGGER_THRESHOLD);
          }

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

AsyncTaskManager::TaskHelper::TaskHelper(TaskHelper&& rhs) noexcept
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
