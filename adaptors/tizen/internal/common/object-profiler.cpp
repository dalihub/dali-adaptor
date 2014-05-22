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

#include "object-profiler.h"
#include <stdlib.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/stage.h>
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/type-registry.h>

using std::string;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

ObjectProfiler::ObjectProfiler()
: mIsActive(false)
{
  // This class must be created after the Stage; this means it doesn't count the initial objects
  // that are created by the stage (base layer, default camera actor)
  mObjectRegistry = Dali::Stage::GetCurrent().GetObjectRegistry();

  char* profile = getenv("PROFILE_DALI_OBJECTS");
  if( profile != NULL )
  {
    mIsActive = true;
    int timeInterval = atoi(profile);
    if( timeInterval > 0 )
    {
      mTimer = Dali::Timer::New(timeInterval*1000);
      mTimer.TickSignal().Connect( this, &ObjectProfiler::OnTimeout );
      mTimer.Start();
    }

    mObjectRegistry.ObjectCreatedSignal().Connect( this, &ObjectProfiler::OnObjectCreated );
    mObjectRegistry.ObjectDestroyedSignal().Connect( this, &ObjectProfiler::OnObjectDestroyed );
  }
}

ObjectProfiler::~ObjectProfiler()
{
}

void ObjectProfiler::DisplayInstanceCounts()
{
  InstanceCountMapIterator iter = mInstanceCountMap.begin();
  InstanceCountMapIterator end = mInstanceCountMap.end();

  for( ; iter != end; iter++ )
  {
    LogMessage(Debug::DebugInfo, "%-30s: %d\n", iter->first.c_str(), iter->second);
  }
  LogMessage(Debug::DebugInfo, "\n");
}

bool ObjectProfiler::OnTimeout()
{
  DisplayInstanceCounts();
  return true;
}

void ObjectProfiler::OnObjectCreated(BaseHandle handle)
{
  string theType = handle.GetTypeName();
  if( theType.empty() )
  {
    DALI_LOG_ERROR("Object created from an unregistered type\n");
    theType = "<Unregistered>";
  }

  mInstanceTypes.push_back(InstanceTypePair(&handle.GetBaseObject(), theType));

  InstanceCountMapIterator iter = mInstanceCountMap.find(theType);
  if( iter == mInstanceCountMap.end() )
  {
    InstanceCountPair instanceCount(theType, 1);
    mInstanceCountMap.insert(instanceCount);
  }
  else
  {
    iter->second++;
  }
}

void ObjectProfiler::OnObjectDestroyed(const Dali::RefObject* object)
{
  const BaseObject* baseObject = static_cast<const BaseObject*>(object);

  InstanceTypes::iterator end = mInstanceTypes.end();
  for( InstanceTypes::iterator iter = mInstanceTypes.begin(); iter != end; iter++)
  {
    if( iter->first == baseObject )
    {
      const std::string& theType = iter->second;
      if( !theType.empty() )
      {
        InstanceCountMapIterator countIter = mInstanceCountMap.find(theType);
        if( countIter != mInstanceCountMap.end() )
        {
          countIter->second--;
        }
      }
      mInstanceTypes.erase( iter );
      break;
    }
  }
}


} // Adaptor
} // Internal
} // Dali
