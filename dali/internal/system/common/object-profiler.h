#ifndef DALI_ADAPTOR_OBJECT_PROFILER_H
#define DALI_ADAPTOR_OBJECT_PROFILER_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <cstdint> // uint32_t
#include <cstddef> // size_t
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/object/object-registry.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/timer.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * Class to profile the number of instances of Objects in the system
 */
class ObjectProfiler : public ConnectionTracker
{
public:

  /**
   * Constructor
   * @param timeInterval to specify the frequency of reporting
   */
  ObjectProfiler( uint32_t timeInterval );

  /**
   * Destructor
   */
  ~ObjectProfiler();

  /**
   * Display a list of types with the current number of instances in the system
   */
  void DisplayInstanceCounts();

private:
  /**
   * If timer is running, display the instance counts
   */
  bool OnTimeout();

  /**
   * Callback used when objects are created. Increases instance count for that object type
   * @param[in] handle of the created object
   */
  void OnObjectCreated( BaseHandle handle );

  /**
   * Callback used when objects are created. Decreases instance count for that object type
   * @param[in] object The object being destroyed
   */
  void OnObjectDestroyed( const Dali::RefObject* object );

  /**
   * Get the memory size of the given object
   */
  std::size_t GetMemorySize( const std::string& name, uint32_t count );

private:

  using InstanceCountPair = std::pair< const std::string, uint32_t >;
  using InstanceTypePair = std::pair< BaseObject*, std::string >;

  Dali::ObjectRegistry    mObjectRegistry;
  Dali::Timer             mTimer;
  std::vector< InstanceCountPair > mInstanceCountContainer;
  std::vector< InstanceTypePair >  mInstanceTypes;
};

} // Adaptor
} // Internal
} // Dali

#endif // DALI_ADAPTOR_OBJECT_PROFILER_H
