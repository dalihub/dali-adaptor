#ifndef DALI_INTERNAL_ASYNC_TASK_MANAGER_H
#define DALI_INTERNAL_ASYNC_TASK_MANAGER_H

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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/devel-api/threading/thread.h>
#include <dali/integration-api/adaptor-framework/log-factory-interface.h>
#include <dali/integration-api/processor-interface.h>
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
  ConditionalWait                  mConditionalWait;
  AsyncTaskManager&                mAsyncTaskManager;
  const Dali::LogFactoryInterface& mLogFactory; ///< The log factory
  bool                             mDestroyThread;
  bool                             mIsThreadStarted;
  bool                             mIsThreadIdle;
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
   * Pop the next task out from the queue.
   *
   * @return The next task to be processed.
   */
  AsyncTaskPtr PopNextTaskToProcess();

  /**
   * Pop the next task out from the completed queue, called by main thread.
   *
   * @return The next task in the completed queue.
   */
  AsyncTaskPtr PopNextCompletedTask();

  /**
   * Pop the next task out from the running queue and add this task to the completed queue.
   *
   * @param[in] task The task added to the queue.
   */
  void CompleteTask(AsyncTaskPtr task);

  /**
   * @brief Unregister a previously registered processor
   */
  void UnregisterProcessor();

  /**
   * Execute the callback registered by tasks in the completed queue
   */
  void TasksCompleted();

  /**
   * @copydoc Dali::Integration::Processor::Process()
   */
  void Process(bool postProcessor) override;

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

private:
  // Undefined
  AsyncTaskManager(const AsyncTaskManager& manager);

  // Undefined
  AsyncTaskManager& operator=(const AsyncTaskManager& manager);

private:
  std::vector<AsyncTaskPtr> mWaitingTasks;   //The queue of the tasks waiting to async process
  std::vector<AsyncTaskPtr> mCompletedTasks; //The queue of the tasks with the async process

  using AsyncTaskPair = std::pair<AsyncTaskPtr, bool>;
  std::vector<AsyncTaskPair> mRunningTasks; ///< The queue of the running tasks

  RoundRobinContainerView<TaskHelper> mTasks;

  Dali::Mutex                          mMutex;
  std::unique_ptr<EventThreadCallback> mTrigger;
  bool                                 mProcessorRegistered;
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