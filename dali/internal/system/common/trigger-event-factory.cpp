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

// INTERNAL INCLUDES
#include <dali/internal/system/common/trigger-event.h>
#include <dali/internal/system/common/unified-trigger-event-manager.h>

namespace Dali
{
TriggerEventFactory::TriggerEventPtr TriggerEventFactory::CreateTriggerEvent(Dali::CallbackBase* callback, Dali::TriggerEventInterface::Options options)
{
  return TriggerEventFactory::TriggerEventPtr(Internal::Adaptor::UnifiedTriggerEventManager::Get().GenerateTriggerEvent(callback, options));
}

void TriggerEventFactory::DestroyTriggerEvent(TriggerEventInterface* triggerEventInterface)
{
  Internal::Adaptor::TriggerEvent* triggerEvent(static_cast<Internal::Adaptor::TriggerEvent*>(triggerEventInterface));
  Internal::Adaptor::UnifiedTriggerEventManager::Get().DiscardTriggerEvent(triggerEvent);
}

} // namespace Dali
