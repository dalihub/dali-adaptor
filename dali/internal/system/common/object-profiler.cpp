/*
 * Copyright (c) 20227 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/object-profiler.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/profiling.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/object/type-registry.h>

#include <stdlib.h>

using std::string;
using namespace Dali::Integration::Profiling;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
ObjectProfiler::ObjectProfiler(Dali::ObjectRegistry objectRegistry, uint32_t timeInterval)
: mObjectRegistry(objectRegistry)
{
  // This class must be created after the Stage; this means it doesn't count the initial objects
  // that are created by the stage (base layer, default camera actor)

  mTimer = Dali::Timer::New(timeInterval * 1000);
  mTimer.TickSignal().Connect(this, &ObjectProfiler::OnTimeout);
  mTimer.Start();

  mObjectRegistry.ObjectCreatedSignal().Connect(this, &ObjectProfiler::OnObjectCreated);
  mObjectRegistry.ObjectDestroyedSignal().Connect(this, &ObjectProfiler::OnObjectDestroyed);
}

ObjectProfiler::~ObjectProfiler()
{
}

void ObjectProfiler::DisplayInstanceCounts()
{
  for(auto&& element : mInstanceCountContainer)
  {
    std::size_t memorySize = GetMemorySize(element.first, element.second);
    if(memorySize > 0)
    {
      LogMessage(Debug::INFO, "%-30s: % 4d  Memory MemorySize: ~% 6.1f kB\n", element.first.c_str(), element.second, memorySize / 1024.0f);
    }
    else
    {
      LogMessage(Debug::INFO, "%-30s: % 4d\n", element.first.c_str(), element.second);
    }
  }
  LogMessage(Debug::INFO, "\n");
}

bool ObjectProfiler::OnTimeout()
{
  uint32_t pixelDataSize   = Dali::PixelData::GetTotalAllocatedSize();
  uint32_t pixelBufferSize = Dali::Internal::Adaptor::PixelBuffer::GetTotalAllocatedSize();
  LogMessage(Debug::INFO, "Total PixelData: %9.1fkb\n", ((float)pixelDataSize) / 1024.0f);
  LogMessage(Debug::INFO, "Total PixelBuffer: %9.1fkb\n", ((float)pixelBufferSize) / 1024.0f);

  DisplayInstanceCounts();
  return true;
}

void ObjectProfiler::OnObjectCreated(BaseHandle handle)
{
  string theType = handle.GetTypeName();
  if(theType.empty())
  {
    DALI_LOG_ERROR("Object created from an unregistered type\n");
    theType = "<Unregistered>";
  }

  mInstanceTypes.push_back(InstanceTypePair(&handle.GetBaseObject(), theType));

  bool found = false;
  for(auto&& element : mInstanceCountContainer)
  {
    if(element.first == theType)
    {
      element.second++;
      found = true;
    }
  }
  if(!found)
  {
    InstanceCountPair instanceCount(theType, 1);
    mInstanceCountContainer.emplace_back(instanceCount);
  }
}

void ObjectProfiler::OnObjectDestroyed(const Dali::RefObject* object)
{
  const BaseObject* baseObject = static_cast<const BaseObject*>(object);

  const auto end = mInstanceTypes.end();
  for(auto iter = mInstanceTypes.begin(); iter != end; ++iter)
  {
    if(iter->first == baseObject)
    {
      const auto& theType = iter->second;
      if(!theType.empty())
      {
        auto&& countIter = std::find_if(mInstanceCountContainer.begin(),
                                        mInstanceCountContainer.end(),
                                        [theType](const InstanceCountPair& instance) { return instance.first == theType; });
        if(countIter != mInstanceCountContainer.end())
        {
          (*countIter).second--;
        }
      }
      mInstanceTypes.erase(iter);
      return;
    }
  }
}

std::size_t ObjectProfiler::GetMemorySize(const std::string& name, uint32_t count)
{
  struct MemoryMemorySize
  {
    std::string name;
    std::size_t memorySize;
  };
  MemoryMemorySize memoryMemorySizes[] =
    {
      {"Animation", ANIMATION_MEMORY_SIZE},
      {"Constraint", CONSTRAINT_MEMORY_SIZE},
      {"Actor", ACTOR_MEMORY_SIZE},
      {"Layer", LAYER_MEMORY_SIZE},
      {"CameraActor", CAMERA_ACTOR_MEMORY_SIZE},
      {"Renderer", RENDERER_MEMORY_SIZE},
      {"Geometry", GEOMETRY_MEMORY_SIZE},
      {"PropertyBuffer", PROPERTY_BUFFER_MEMORY_SIZE},
      {"TextureSet", TEXTURE_SET_MEMORY_SIZE},
      {"Sampler", SAMPLER_MEMORY_SIZE},
      {"Shader", SHADER_MEMORY_SIZE},
    };

  for(size_t i = 0; i < sizeof(memoryMemorySizes) / sizeof(MemoryMemorySize); i++)
  {
    if(memoryMemorySizes[i].name.compare(name) == 0)
    {
      return count * memoryMemorySizes[i].memorySize;
    }
  }
  return 0;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
