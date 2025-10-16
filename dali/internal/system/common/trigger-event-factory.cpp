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
 *
 */

// CLASS HEADER
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/trigger-event.h>
#include <dali/internal/system/common/unified-trigger-event-manager-impl.h>
#include <dali/internal/system/common/unified-trigger-event-manager.h>

namespace Dali
{
TriggerEventFactory::TriggerEventPtr TriggerEventFactory::CreateTriggerEvent(Dali::CallbackBase* callback, Dali::TriggerEventInterface::Options options)
{
  auto unifiedTriggerEventManager = Internal::Adaptor::UnifiedTriggerEventManager::Get();
  if(DALI_LIKELY(unifiedTriggerEventManager))
  {
    return TriggerEventFactory::TriggerEventPtr(GetImplementation(unifiedTriggerEventManager).GenerateTriggerEvent(callback, options));
  }
  else
  {
    auto* triggerEvent = new Internal::Adaptor::TriggerEvent(nullptr, callback, options);
    DALI_LOG_DEBUG_INFO("Generated Trigger[%p] Id(%u) options:%d without unified trigger event manager!\n", triggerEvent, triggerEvent->GetId(), static_cast<int>(options));
    return TriggerEventFactory::TriggerEventPtr(triggerEvent);
  }
}

void TriggerEventFactory::DestroyTriggerEvent(TriggerEventInterface* triggerEventInterface)
{
  Internal::Adaptor::TriggerEvent* triggerEvent(static_cast<Internal::Adaptor::TriggerEvent*>(triggerEventInterface));

  if(DALI_LIKELY(triggerEvent))
  {
    auto unifiedTriggerEventManager = triggerEvent->GetUnifiedTriggerEventManager();
    if(DALI_LIKELY(unifiedTriggerEventManager))
    {
      GetImplementation(unifiedTriggerEventManager).DiscardTriggerEvent(triggerEvent);
    }
    else
    {
      DALI_LOG_DEBUG_INFO("Delete Trigger[%p] Id(%u) synchronous\n", triggerEvent, triggerEvent ? triggerEvent->GetId() : 0);
      delete triggerEvent;
    }
  }
}

} // namespace Dali
