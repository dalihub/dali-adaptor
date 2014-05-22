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

// CLASS HEADER
#include "trigger-event-factory.h"

// INTERNAL INCLUDES
#include <internal/common/trigger-event.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

TriggerEventInterface* TriggerEventFactory::CreateTriggerEvent(  boost::function<void()> functor,  TriggerEventInterface::Options options )
{
  return new TriggerEvent( functor, options );
}

void TriggerEventFactory::DestroyTriggerEvent( TriggerEventInterface* triggerEventInterface )
{
  TriggerEvent* triggerEvent( static_cast< TriggerEvent* >( triggerEventInterface) );
  delete triggerEvent;
}


} // namespace Internal

} // namespace Adaptor

} // namespace Dali
