#ifndef DALI_ASYNC_TASK_MANAGER_H
#define DALI_ASYNC_TASK_MANAGER_H

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
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/callback.h>
#include <memory>

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
} // namespace DALI_INTERNAL

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
   * Constructor
   * @SINCE_2_2.3
   * @param[in] callback The callback to invoke on task completion, either on the main thread on the worker thread. The ownership of callback is taken by this class.
   * @param[in] threadType The thread type of invocation callback.
   */
  AsyncTask(CallbackBase* callback, ThreadType threadType = AsyncTask::ThreadType::MAIN_THREAD);

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
  ThreadType GetCallbackInvocationThread();

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
  virtual bool IsReady() = 0;

private:
  std::unique_ptr<CallbackBase> mCompletedCallback;
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
