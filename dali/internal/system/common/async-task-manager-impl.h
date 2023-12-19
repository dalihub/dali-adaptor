#ifndef DALI_INTERNAL_ASYNC_TASK_MANAGER_H
#define DALI_INTERNAL_ASYNC_TASK_MANAGER_H

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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/devel-api/threading/thread.h>
#include <dali/integration-api/adaptor-framework/log-factory-interface.h>
#include <dali/integration-api/adaptor-framework/trace-factory-interface.h>
#include <dali/integration-api/processor-interface.h>
#include <dali/public-api/common/list-wrapper.h>
#include <dali/public-api/object/base-object.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/async-task-manager.h>
#include <dali/public-api/adaptor-framework/round-robin-container-view.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class AsyncTaskManager;

/**
 * The worker thread for async process
 */
class AsyncTaskThread : public Thread
{
public:
  /**
   * Constructor.
   */
  AsyncTaskThread(AsyncTaskManager& asyncTaskManager);

  /**
   * Destructor.
   */
  ~AsyncTaskThread() override;

  /**
   * @brief Request the thread to process the task.
   * @return True if the request is successed, otherwise false.
   */
  bool Request();

protected:
  /**
   * The entry function of the worker thread.
   */
  void Run() override;

private:
  // Undefined
  AsyncTaskThread(const AsyncTaskThread& thread) = delete;

  // Undefined
  AsyncTaskThread& operator=(const AsyncTaskThread& thread) = delete;

private:
  ConditionalWait                    mConditionalWait;
  AsyncTaskManager&                  mAsyncTaskManager;
  const Dali::LogFactoryInterface&   mLogFactory;   ///< The log factory
  const Dali::TraceFactoryInterface& mTraceFactory; ///< The trace factory
  bool                               mDestroyThread;
  bool                               mIsThreadStarted;
  bool                               mIsThreadIdle;
};

/**
 * The manager for async task
 */
class AsyncTaskManager : public Dali::BaseObject, public Integration::Processor
{
public:
  /**
   * Singleton access
   *
   * @return The AsyncTaskManager object
   */
  static Dali::AsyncTaskManager Get();

  /**
   * Constructor.
   */
  AsyncTaskManager();

  /**
   * Destructor.
   */
  ~AsyncTaskManager() override;

  /**
   * @copydoc Dali::AsyncTaskManager::AddTask()
   */
  void AddTask(AsyncTaskPtr task);

  /**
   * @copydoc Dali::AsyncTaskManager::RemoveTask()
   */
  void RemoveTask(AsyncTaskPtr task);

  /**
   * @copydoc Dali::AsyncTaskManager::SetCompletedCallback()
   */
  Dali::AsyncTaskManager::TasksCompletedId SetCompletedCallback(CallbackBase* callback, Dali::AsyncTaskManager::CompletedCallbackTraceMask mask);

  /**
   * @copydoc Dali::AsyncTaskManager::RemoveCompletedCallback()
   */
  bool RemoveCompletedCallback(Dali::AsyncTaskManager::TasksCompletedId tasksCompletedId);

  /**
   * Pop the next task out from the completed queue, called by main thread.
   *
   * @return The next task in the completed queue.
   */
  AsyncTaskPtr PopNextCompletedTask();

  /**
   * @brief Register processor if we don't registered before.
   */
  void RegisterProcessor();

  /**
   * @brief Unregister a previously registered processor
   */
  void UnregisterProcessor();

  /**
   * Execute the callback registered by tasks in the completed queue
   */
  void TasksCompleted();

public: // Worker thread called method
  /**
   * Pop the next task out from the queue.
   *
   * @return The next task to be processed.
   */
  AsyncTaskPtr PopNextTaskToProcess();

  /**
   * Pop the next task out from the running queue and add this task to the completed queue.
   * @note After this function, task is invalidate.
   *
   * @param[in] task The task added to the queue.
   */
  void CompleteTask(AsyncTaskPtr&& task);

protected: // Implementation of Processor
  /**
   * @copydoc Dali::Integration::Processor::Process()
   */
  void Process(bool postProcessor) override;

  /**
   * @copydoc Dali::Integration::Processor::GetProcessorName()
   */
  std::string_view GetProcessorName() const override
  {
    return "AsyncTaskManager";
  }

private:
  /**
   * @brief Helper class to keep the relation between AsyncTaskThread and corresponding container
   */
  class TaskHelper
  {
  public:
    /**
     * @brief Create an TaskHelper.
     *
     * @param[in] asyncTaskManager Reference to the AsyncTaskManager
     */
    TaskHelper(AsyncTaskManager& asyncTaskManager);

    /**
     * @brief Request the thread to process the task.
     * @return True if the request succeeds, otherwise false.
     */
    bool Request();

  public:
    TaskHelper(const TaskHelper&) = delete;
    TaskHelper& operator=(const TaskHelper&) = delete;

    TaskHelper(TaskHelper&& rhs);
    TaskHelper& operator=(TaskHelper&& rhs) = delete;

  private:
    /**
     * @brief Main constructor that used by all other constructors
     */
    TaskHelper(std::unique_ptr<AsyncTaskThread> processor, AsyncTaskManager& asyncTaskManager);

  private:
    std::unique_ptr<AsyncTaskThread> mProcessor;
    AsyncTaskManager&                mAsyncTaskManager;
  };

  /**
   * @brief State of running task
   */
  enum class RunningTaskState
  {
    RUNNING  = 0, ///< Running task
    CANCELED = 1, ///< Canceled by user
  };

  /**
   * @brief State of complete task
   */
  enum class CompletedTaskState
  {
    REQUIRE_CALLBACK = 0, ///< Need to execute callback when completed task process.
    SKIP_CALLBACK    = 1, ///< Do not execute callback
  };

private:
  // Undefined
  AsyncTaskManager(const AsyncTaskManager& manager);

  // Undefined
  AsyncTaskManager& operator=(const AsyncTaskManager& manager);

private:
  // Keep Task as list since we take tasks by FIFO as default.
  using AsyncTaskContainer = std::list<AsyncTaskPtr>;

  using AsyncRunningTaskPair      = std::pair<AsyncTaskPtr, RunningTaskState>;
  using AsyncRunningTaskContainer = std::list<AsyncRunningTaskPair>;

  using AsyncCompletedTaskPair      = std::pair<AsyncTaskPtr, CompletedTaskState>;
  using AsyncCompletedTaskContainer = std::list<AsyncCompletedTaskPair>;

  AsyncTaskContainer          mWaitingTasks;   ///< The queue of the tasks waiting to async process. Must be locked under mWaitingTasksMutex.
  AsyncRunningTaskContainer   mRunningTasks;   ///< The queue of the running tasks. Must be locked under mRunningTasksMutex.
  AsyncCompletedTaskContainer mCompletedTasks; ///< The queue of the tasks with the async process. Must be locked under mCompletedTasksMutex.

  RoundRobinContainerView<TaskHelper> mTasks;

  uint32_t mAvaliableLowPriorityTaskCounts; ///< The number of tasks that can be processed for priority type LOW.
                                            ///< Be used to select next wating task determining algorithm.
                                            ///< Note : For thread safety, Please set/get this value under mRunningTasksMutex scope.
  uint32_t mWaitingHighProirityTaskCounts;  ///< The number of tasks that waiting now for priority type HIGH.
                                            ///< Be used to select next wating task determining algorithm.
                                            ///< Note : For thread safety, Please set/get this value under mWaitingTasksMutex scope.

  Dali::Mutex mWaitingTasksMutex;   ///< Mutex for mWaitingTasks. We can lock mRunningTasksMutex and mCompletedTasksMutex under this scope.
  Dali::Mutex mRunningTasksMutex;   ///< Mutex for mRunningTasks. We can lock mCompletedTasksMutex under this scope.
  Dali::Mutex mCompletedTasksMutex; ///< Mutex for mCompletedTasks. We cannot lock any mutex under this scope.

  std::unique_ptr<EventThreadCallback> mTrigger;

  struct TasksCompletedImpl;
  std::unique_ptr<TasksCompletedImpl> mTasksCompletedImpl; ///< TaskS completed signal interface for AsyncTaskManager.

  struct CacheImpl;
  std::unique_ptr<CacheImpl> mCacheImpl; ///< Cache interface for AsyncTaskManager.

  bool mProcessorRegistered : 1;
};

} // namespace Adaptor

} // namespace Internal

inline Internal::Adaptor::AsyncTaskManager& GetImplementation(Dali::AsyncTaskManager& obj)
{
  DALI_ASSERT_ALWAYS(obj && "AsyncTaskManager is empty");

  Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<Internal::Adaptor::AsyncTaskManager&>(handle);
}

inline const Internal::Adaptor::AsyncTaskManager& GetImplementation(const Dali::AsyncTaskManager& obj)
{
  DALI_ASSERT_ALWAYS(obj && "AsyncTaskManager is empty");

  const Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<const Internal::Adaptor::AsyncTaskManager&>(handle);
}

} // namespace Dali

#endif
