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
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/system/windows/trigger-event-win.h>
#include <dali/internal/system/windows/unified-trigger-event-manager-impl-win.h>

namespace Dali
{
TriggerEventFactory::TriggerEventPtr TriggerEventFactory::CreateTriggerEvent(CallbackBase* callback)
{
  auto unifiedTriggerEventManager = Internal::Adaptor::UnifiedTriggerEventManager::Get();
  if(DALI_LIKELY(unifiedTriggerEventManager))
  {
    return TriggerEventFactory::TriggerEventPtr(GetImplementation(unifiedTriggerEventManager).GenerateTriggerEvent(callback));
  }

  auto* triggerEvent = new Internal::Adaptor::TriggerEvent(nullptr, callback);
  DALI_LOG_DEBUG_INFO("Generated Trigger[%p] Id(%u) without unified trigger event manager!\n", triggerEvent, triggerEvent->GetId());
  return TriggerEventFactory::TriggerEventPtr(triggerEvent);
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
      delete triggerEvent;
    }
  }
}

} // namespace Dali
