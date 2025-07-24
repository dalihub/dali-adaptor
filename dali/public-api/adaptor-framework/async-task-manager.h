#ifndef DALI_ASYNC_TASK_MANAGER_H
#define DALI_ASYNC_TASK_MANAGER_H

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
 */

// EXTERNAL INCLUDES
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/callback.h>
#include <memory>
#include <string_view>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class AsyncTaskManager;
}
} // namespace Internal DALI_INTERNAL

class AsyncTask;
using AsyncTaskPtr = IntrusivePtr<AsyncTask>;

/**
 * The async tasks to be processed in the worker thread.
 * @SINCE_2_2.3
 */
class DALI_ADAPTOR_API AsyncTask : public RefObject
{
public:
  // The Type of invocation thread
  enum class ThreadType
  {
    MAIN_THREAD,
    WORKER_THREAD
  };

  /**
   * @brief The priority of this task what user think.
   * To avoid long term tasks (like remote image download) block whole threads,
   * Let user set the priority type of this task.
   *
   * Low priority task means it doesn't need to process by FIFO logic.
   * So we make that Low priority don't take whole threads.
   *
   * Task selection algorithm defined internally.
   *
   * @note Task cannot change the priority type after create.
   *
   * @SINCE_2_2.17
   */
  enum class PriorityType
  {
    HIGH = 0, ///< Highest priority to process task. @SINCE_2_2.17
    LOW  = 1, ///< Lowest priority to process task. @SINCE_2_2.17

    PRIORITY_COUNT, ///< The number of priority type. @SINCE_2_2.17

    DEFAULT = HIGH, ///< Default priority value if nothing defined. @SINCE_2_2.17
  };

  /**
   * Constructor
   * @SINCE_2_2.3
   * @param[in] callback The callback to invoke on task completion, either on the main thread on the worker thread. The ownership of callback is taken by this class.
   * @param[in] priority The proirity type of this task.
   * @param[in] threadType The thread type of invocation callback.
   */
  AsyncTask(CallbackBase* callback, PriorityType priority = PriorityType::DEFAULT, ThreadType threadType = AsyncTask::ThreadType::MAIN_THREAD);

  /**
   * Get the complated callback
   * @SINCE_2_2.3
   */
  CallbackBase* GetCompletedCallback();

  /**
   * Get the thread of the invocation callback
   * @SINCE_2_2.9
   * @return the type of invocation callback.
   */
  ThreadType GetCallbackInvocationThread() const;

  /**
   * Get the priority of this task
   * @SINCE_2_2.17
   * @return the type of priority.
   */
  PriorityType GetPriorityType() const;

  /**
   * Notify to task become ready.
   * @note IsReady always return true before and after this API.
   * @SINCE_2_4.29
   */
  void NotifyToReady();

  /**
   * Destructor.
   * @SINCE_2_2.3
   */
  virtual ~AsyncTask() = default;

  /**
   * Process the task
   * @SINCE_2_2.3
   */
  virtual void Process() = 0;

  /**
   * Whether the task is ready to process.
   * @SINCE_2_2.3
   * @return True if the task is ready to process.
   */
  virtual bool IsReady()
  {
    return true;
  }

  /**
   * Get the name of this task if we setup.
   * @SINCE_2_3.4
   * @return The name of this task.
   */
  virtual std::string_view GetTaskName() const
  {
    Dali::DaliPrintBackTrace();
    return "";
  }

private:
  std::unique_ptr<CallbackBase> mCompletedCallback;
  const PriorityType            mPriorityType;
  ThreadType                    mThreadType;

  // Undefined
  AsyncTask(const AsyncTask& task) = delete;

  // Undefined
  AsyncTask& operator=(const AsyncTask& task) = delete;
};

/**
 * The manager for async task
 * @SINCE_2_2.3
 */
class DALI_ADAPTOR_API AsyncTaskManager : public BaseHandle
{
public:
  /**
   * Constructor.
   * @SINCE_2_2.3
   */
  AsyncTaskManager();

  /**
   * Destructor.
   * @SINCE_2_2.3
   */
  ~AsyncTaskManager();

  /**
   * @brief Gets the singleton of AsyncTaskManager object.
   *
   * @SINCE_2_2.3
   * @return A handle to the AsyncTaskManager
   */
  static AsyncTaskManager Get();

  /**
   * @brief Add the async task into the waiting queue, called by main thread.
   *
   * @SINCE_2_2.3
   * @param[in] task The task added to the queue.
   */
  void AddTask(AsyncTaskPtr task);

  /**
   * @brief Remove the task from the waiting queue, called by main thread.
   *
   * @SINCE_2_2.3
   * @param[in] task The task pointer.
   */
  void RemoveTask(AsyncTaskPtr task);

  /**
   * @brief Notify to task was ready, called by any thread.
   *
   * @SINCE_2_4.29
   * @param[in] task The task pointer.
   */
  void NotifyToTaskReady(AsyncTaskPtr task);

public:
  using TasksCompletedId = uint32_t;

  enum CompletedCallbackTraceMask
  {
    THREAD_MASK_MAIN   = 1u << 0, ///< Trace only main thread tasks.
    THREAD_MASK_WORKER = 1u << 1, ///< Trace only worker thread tasks.

    PRIORITY_MASK_HIGH = 1u << 2, ///< Trace only high priority tasks.
    PRIORITY_MASK_LOW  = 1u << 3, ///< Trace only low priority tasks.

    THREAD_MASK_ALL   = THREAD_MASK_MAIN | THREAD_MASK_WORKER,
    PRIORITY_MASK_ALL = PRIORITY_MASK_HIGH | PRIORITY_MASK_LOW,

    // Useful preset of task mask.

    MAIN_THREAD_TASKS   = THREAD_MASK_MAIN | PRIORITY_MASK_ALL,
    WORKER_THREAD_TASKS = THREAD_MASK_WORKER | PRIORITY_MASK_ALL,
    HIGH_PRIORITY_TASKS = THREAD_MASK_ALL | PRIORITY_MASK_HIGH,
    LOW_PRIORITY_TASKS  = THREAD_MASK_ALL | PRIORITY_MASK_LOW,

    MAIN_THREAD_HIGH_PRIORITY_TASKS = THREAD_MASK_MAIN | PRIORITY_MASK_HIGH,
    MAIN_THREAD_LOW_PRIORITY_TASKS  = THREAD_MASK_MAIN | PRIORITY_MASK_LOW,

    ALL_TASKS = THREAD_MASK_ALL | PRIORITY_MASK_ALL,

    DEFAULT = ALL_TASKS,
  };

  /**
   * @brief Set the async tasks completed callback.
   * Inputed callback will be emitted after all tasks what user added are completed.
   *
   * Usage example:
   *
   *   void OnTasksCompleted(TasksCompletedId id);
   *   auto id0 = AsyncTaskManager::Get().SetCompletedCallback(MakeCallback(OnTasksCompleted), CompletedCallbackTraceMask::MASK_ALL);
   *   // OnTasksCompleted(id0); called at next Idler.
   *
   *   AsyncTaskManager::Get().AddTask(task1);
   *   auto id1 = AsyncTaskManager::Get().SetCompletedCallback(MakeCallback(OnTasksCompleted), CompletedCallbackTraceMask::MASK_ALL);
   *   // OnTasksCompleted(id1); called after task1 completed.
   *
   *   AsyncTaskManager::Get().AddTask(task2WhichIsLowPriority);
   *   AsyncTaskManager::Get().AddTask(task3WhichIsWorkerThread);
   *   AsyncTaskManager::Get().AddTask(task4);
   *   auto id2 = AsyncTaskManager::Get().SetCompletedCallback(MakeCallback(OnTasksCompleted), static_cast<CompletedCallbackTraceMask>(CompletedCallbackTraceMask::THREAD_MASK_MAIN | CompletedCallbackTraceMask::PRIORITY_MASK_HIGH));
   *   // OnTasksCompleted(id2); called after task1 and task4 completed.
   *
   *   AsyncTaskManager::Get().RemoveCompletedCallback(id1);
   *   // OnTasksCompleted(id1); will not be called.
   *
   * @note The ownership of callback will be hold AsyncTaskManager itself.
   * @note The callback will be emmited at Process() timming.
   *
   * @SINCE_2_2.52
   * @param[in] callback The callback base when all AsyncTasks completed.
   *                     This callback will be void return, and single input argument ; TasksCompletedId.
   * @param[in] mask Mask info s.t. what kind of async task we want to detact.
   *                 For example, if we set this value as MASK_ALL & ~PRIORITY_MASK_LOW, we will ignore low priority tasks.
   *                 Default is MASK_ALL.
   * @return The unique id for callback. It can be used when we want to remove callback.
   */
  TasksCompletedId SetCompletedCallback(CallbackBase* callback, CompletedCallbackTraceMask mask = CompletedCallbackTraceMask::DEFAULT);

  /**
   * @brief Remove the async tasks completed callback.
   * @note It will not execute setted callback.
   *
   * @SINCE_2_2.52
   * @param[in] tasksCompletedId The id for callback that want to remove.
   * @return True if we success to removed. False if it already removed, or callback already emitted.
   */
  bool RemoveCompletedCallback(TasksCompletedId tasksCompletedId);

public:
  /// @cond internal
  /**
   * @brief Allows the creation of a AsyncTaskManager handle from an internal pointer.
   *
   * @note Not intended for application developers
   * @SINCE_2_2.3
   * @param[in] impl A pointer to the object
   */
  explicit DALI_INTERNAL AsyncTaskManager(Internal::Adaptor::AsyncTaskManager* impl);
  /// @endcond
};

} // namespace Dali

#endif
