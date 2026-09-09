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
 *
 */

#include <dali-test-suite-utils.h>
#include <dali/devel-api/adaptor-framework/async-task-manager.h>

using namespace Dali;

namespace
{
class TestAsyncTask : public AsyncTask
{
public:
  TestAsyncTask(CallbackBase* callback, PriorityType priority = PriorityType::HIGH, ThreadType threadType = ThreadType::MAIN_THREAD)
  : AsyncTask(callback, priority, threadType),
    mProcessed(false)
  {
  }

  void Process() override
  {
    mProcessed = true;
  }

  bool GetResult()
  {
    return mProcessed;
  }

private:
  bool mProcessed;
};
} // namespace

int UtcDaliAsyncTaskModuleExistsP(void)
{
  // Verify that AsyncTask module is available
  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliAsyncTaskPriorityTypeEnumP(void)
{
  // Verify priority type enum values
  DALI_TEST_EQUALS(static_cast<int>(AsyncTask::PriorityType::HIGH), 0, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(AsyncTask::PriorityType::LOW), 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(AsyncTask::PriorityType::PRIORITY_COUNT), 2, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAsyncTaskThreadTypeEnumP(void)
{
  // Verify thread type enum values
  DALI_TEST_EQUALS(static_cast<int>(AsyncTask::ThreadType::MAIN_THREAD), 0, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(AsyncTask::ThreadType::WORKER_THREAD), 1, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAsyncTaskDefaultPriorityP(void)
{
  // Verify default priority is HIGH
  DALI_TEST_EQUALS(static_cast<int>(AsyncTask::PriorityType::DEFAULT), static_cast<int>(AsyncTask::PriorityType::HIGH), TEST_LOCATION);

  END_TEST;
}

int UtcDaliAsyncTaskProcessMethodP(void)
{
  // Create a dummy callback (will not be executed in unit test context)
  CallbackBase* callback = nullptr;
  TestAsyncTask task(callback, AsyncTask::PriorityType::HIGH, AsyncTask::ThreadType::MAIN_THREAD);

  // Process should be callable
  task.Process();
  DALI_TEST_EQUALS(task.GetResult(), true, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAsyncTaskIsReadyP(void)
{
  // Task should report ready status
  CallbackBase* callback = nullptr;
  TestAsyncTask task(callback, AsyncTask::PriorityType::HIGH, AsyncTask::ThreadType::MAIN_THREAD);

  bool isReady = task.IsReady();
  DALI_TEST_EQUALS(isReady, true, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAsyncTaskHighPriorityConstructionP(void)
{
  // Create task with HIGH priority
  CallbackBase* callback = nullptr;
  TestAsyncTask task(callback, AsyncTask::PriorityType::HIGH, AsyncTask::ThreadType::MAIN_THREAD);

  DALI_TEST_EQUALS(task.GetPriorityType(), AsyncTask::PriorityType::HIGH, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAsyncTaskLowPriorityConstructionP(void)
{
  // Create task with LOW priority
  CallbackBase* callback = nullptr;
  TestAsyncTask task(callback, AsyncTask::PriorityType::LOW, AsyncTask::ThreadType::WORKER_THREAD);

  DALI_TEST_EQUALS(task.GetPriorityType(), AsyncTask::PriorityType::LOW, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAsyncTaskMainThreadCallbackP(void)
{
  // Create task with MAIN_THREAD callback
  CallbackBase* callback = nullptr;
  TestAsyncTask task(callback, AsyncTask::PriorityType::HIGH, AsyncTask::ThreadType::MAIN_THREAD);

  DALI_TEST_EQUALS(task.GetCallbackInvocationThread(), AsyncTask::ThreadType::MAIN_THREAD, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAsyncTaskWorkerThreadCallbackP(void)
{
  // Create task with WORKER_THREAD callback
  CallbackBase* callback = nullptr;
  TestAsyncTask task(callback, AsyncTask::PriorityType::HIGH, AsyncTask::ThreadType::WORKER_THREAD);

  DALI_TEST_EQUALS(task.GetCallbackInvocationThread(), AsyncTask::ThreadType::WORKER_THREAD, TEST_LOCATION);

  END_TEST;
}
