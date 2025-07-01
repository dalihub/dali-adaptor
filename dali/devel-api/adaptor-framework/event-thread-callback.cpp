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
 */

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>

namespace Dali
{
struct EventThreadCallback::Impl
{
  TriggerEventInterface* eventTrigger;
  uint32_t               id;
};

EventThreadCallback::EventThreadCallback(CallbackBase* callback)
: mImpl(new Impl())
{
  mImpl->eventTrigger = TriggerEventFactory::CreateTriggerEvent(callback, TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER);
  mImpl->id           = mImpl->eventTrigger->GetId();
}

EventThreadCallback::~EventThreadCallback()
{
  TriggerEventFactory::DestroyTriggerEvent(mImpl->eventTrigger);
  delete mImpl;
}

void EventThreadCallback::Trigger()
{
  if(mImpl->eventTrigger)
  {
    mImpl->eventTrigger->Trigger();
  }
}

uint32_t EventThreadCallback::GetId() const
{
  return mImpl->id;
}

} // namespace Dali
