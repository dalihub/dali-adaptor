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
#include <dali/internal/system/windows/trigger-event-win.h>

namespace Dali
{
TriggerEventInterface* TriggerEventFactory::CreateTriggerEvent(CallbackBase* callback, TriggerEventInterface::Options options)
{
  return new Internal::Adaptor::TriggerEvent(callback, options);
}

void TriggerEventFactory::DestroyTriggerEvent(TriggerEventInterface* triggerEventInterface)
{
  Internal::Adaptor::TriggerEvent* triggerEvent(static_cast<Internal::Adaptor::TriggerEvent*>(triggerEventInterface));
  delete triggerEvent;
}

} // namespace Dali
