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

// CLASS HEADER
#include <dali/internal/vector-animation/common/vector-animation-renderer-event-manager.h>

// EXTERNAL INCLUDES
#include <thorvg.h>

// INTERNAL INCLUDES
#include <dali/internal/vector-animation/common/vector-animation-renderer-native.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <map>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gVectorAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VECTOR_ANIMATION");
#endif

VectorAnimationRendererEventManager& VectorAnimationRendererEventManager::Get()
{
  static VectorAnimationRendererEventManager instance;
  return instance;
}

VectorAnimationRendererEventManager::VectorAnimationRendererEventManager()
: mEventHandlers(),
  mTriggeredHandlers(),
  mMutex(),
  mEventTrigger(),
  mTriggerOrderId(0u),
  mEventTriggered(false),
  mProcessorRegistered(false),
  mEventHandlerRemovedDuringEventProcessing(false)
{
  auto result = tvg::Initializer::init(1);
  if(result != tvg::Result::Success)
  {
    DALI_LOG_ERROR("VectorAnimationRendererEventManager: Failed to initialize ThorVG\n");
  }
}

VectorAnimationRendererEventManager::~VectorAnimationRendererEventManager()
{
  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "this = %p\n", this);
  if(Dali::Adaptor::IsAvailable() && mProcessorRegistered)
  {
    Dali::Adaptor::Get().UnregisterProcessorOnce(*this);
    mProcessorRegistered = false;
  }

  tvg::Initializer::term();
}

// This function is called in the main thread.
void VectorAnimationRendererEventManager::AddEventHandler(VectorAnimationRendererNative& renderer)
{
  if(mEventHandlers.end() == mEventHandlers.find(&renderer))
  {
    if(!mProcessorRegistered)
    {
      mProcessorRegistered = true;
      Dali::Adaptor::Get().RegisterProcessorOnce(*this);
    }

    mEventHandlers.insert(&renderer);

    {
      Dali::Mutex::ScopedLock lock(mMutex);

      if(!mEventTrigger)
      {
        mEventTrigger = TriggerEventFactory::CreateTriggerEvent(
          MakeCallback(this, &VectorAnimationRendererEventManager::OnEventTriggered));
      }
    }
  }
}

// This function is called in the main thread.
void VectorAnimationRendererEventManager::RemoveEventHandler(VectorAnimationRendererNative& renderer)
{
  auto iter = mEventHandlers.find(&renderer);
  if(iter != mEventHandlers.end())
  {
    bool releaseEventTrigger = false;

    mEventHandlers.erase(iter);

    // Mark removed flag now.
    // Note that it will be checked during event processing.
    mEventHandlerRemovedDuringEventProcessing = true;

    if(mEventHandlers.empty())
    {
      releaseEventTrigger = true;
    }

    {
      Dali::Mutex::ScopedLock lock(mMutex);

      if(releaseEventTrigger)
      {
        // There is no valid event handler now. We can remove whole triggered event handlers.
        mTriggeredHandlers.clear();
        mEventHandlerRemovedDuringEventProcessing = false;

        mEventTrigger.reset();
        mEventTriggered = false;
      }
      else
      {
        auto triggeredHandler = mTriggeredHandlers.find(&renderer);
        if(triggeredHandler != mTriggeredHandlers.end())
        {
          mTriggeredHandlers.erase(triggeredHandler);
        }
      }
    }
  }
}

// Called by VectorAnimationRendererNative (from rendering thread)
void VectorAnimationRendererEventManager::TriggerEvent(VectorAnimationRendererNative& renderer)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  // Add triggered handler only if event trigger exist.
  // If event trigger is null, it means there is no valid handler now. So we can ignore this trigger.
  if(DALI_LIKELY(mEventTrigger))
  {
    if(mTriggeredHandlers.end() == mTriggeredHandlers.find(&renderer))
    {
      mTriggeredHandlers.insert({&renderer, mTriggerOrderId++});

      if(!mEventTriggered)
      {
        mEventTrigger->Trigger();
        mEventTriggered = true;
      }
    }
  }
}

void VectorAnimationRendererEventManager::Process(bool postProcessor)
{
  mProcessorRegistered = false;
  OnEventTriggered();
}

// This function is called in the main thread.
void VectorAnimationRendererEventManager::OnEventTriggered()
{
  std::map<uint32_t, VectorAnimationRendererNative*> handlers;

  decltype(mTriggeredHandlers) movedTriggeredHandlers;

  {
    Dali::Mutex::ScopedLock lock(mMutex);

    // Copy the list to the local variable and clear
    movedTriggeredHandlers.swap(mTriggeredHandlers);

    mTriggerOrderId  = 0u;
    mEventTriggered  = false;
  }

  // Reorder event handler ordered by trigger request.
  // And also, check validation of event handler.
  for(auto&& iter : movedTriggeredHandlers)
  {
    if(mEventHandlers.end() != mEventHandlers.find(iter.first))
    {
      handlers[iter.second] = iter.first;
    }
  }

  // We check validation before processing handlers. Reset removed flag now.
  mEventHandlerRemovedDuringEventProcessing = false;

  for(auto&& iter : handlers)
  {
    auto* handler = iter.second;

    // Check if it is valid.
    // (If the event handler is removed during event processing, it is not valid. So we should not notify event)
    if(!mEventHandlerRemovedDuringEventProcessing || mEventHandlers.end() != mEventHandlers.find(handler))
    {
      handler->NotifyEvent();
    }
  }
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali