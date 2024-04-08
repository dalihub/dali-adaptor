/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include "test-graphics-sync-impl.h"

namespace Dali
{
#if 0
TestGraphicsSyncImplementation::TestGraphicsSyncImplementation()
{
}

/**
 * Destructor
 */
TestGraphicsSyncImplementation::~TestGraphicsSyncImplementation()
{
}

/**
 * Initialize the sync objects
 */
void TestGraphicsSyncImplementation::Initialize()
{
}


/**
 * Destroy a sync object
 * @param[in] syncObject The object to destroy
 */
void TestGraphicsSyncImplementation::DestroySyncObject(void* syncObject)
{
  std::stringstream out;
  out << syncObject;
  mTrace.PushCall("DestroySyncObject", out.str()); // Trace the method

  for(SyncIter iter = mSyncObjects.begin(), end = mSyncObjects.end(); iter != end; ++iter)
  {
    if(*iter == syncObject)
    {
      delete *iter;
      mSyncObjects.erase(iter);
      break;
    }
  }
}

Integration::GraphicsSyncAbstraction::SyncObject* TestGraphicsSyncImplementation::GetLastSyncObject()
{
  if(!mSyncObjects.empty())
  {
    return mSyncObjects.back();
  }
  return NULL;
}

/**
 * Test method to trigger the object sync behaviour.
 * @param[in]
 * @param[in] sync The sync value to set
 */
void TestGraphicsSyncImplementation::SetObjectSynced(Integration::GraphicsSyncAbstraction::SyncObject* syncObject, bool sync)
{
  TestSyncObject* testSyncObject = static_cast<TestSyncObject*>(syncObject);
  testSyncObject->synced         = sync;
}

/**
 * Turn trace on
 */
void TestGraphicsSyncImplementation::EnableTrace(bool enable)
{
  mTrace.Enable(enable);
}

/**
 * Reset the trace callstack
 */
void TestGraphicsSyncImplementation::ResetTrace()
{
  mTrace.Reset();
}

/**
 * Get the trace object (allows test case to find methods on it)
 */
TraceCallStack& TestGraphicsSyncImplementation::GetTrace()
{
  return mTrace;
}

int32_t TestGraphicsSyncImplementation::GetNumberOfSyncObjects()
{
  return static_cast<int32_t>(mSyncObjects.size());
}
#endif
} // namespace Dali
