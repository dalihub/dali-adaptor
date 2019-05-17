#ifndef DALI_INTEGRATION_TRIGGER_EVENT_FACTORY_H
#define DALI_INTEGRATION_TRIGGER_EVENT_FACTORY_H

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#ifdef DALI_ADAPTOR_COMPILATION
#include <dali/integration-api/trigger-event-factory-interface.h>
#else
#include <dali/integration-api/adaptors/trigger-event-factory-interface.h>
#endif

namespace Dali
{

/**
 * @brief Trigger interface factory class
 *
 */
class DALI_ADAPTOR_API TriggerEventFactory : public TriggerEventFactoryInterface
{

public:

  /**
   * @brief Constructor
   */
  TriggerEventFactory()
  {
  }

  /**
   * @brief Destructor
   */
  virtual ~TriggerEventFactory()
  {
  }

  /**
   * @copydoc TriggerEventFactoryInterface::CreateTriggerEvent
   */
  virtual TriggerEventInterface* CreateTriggerEvent(  CallbackBase* callback, TriggerEventInterface::Options options );


  /**
   * @copydoc TriggerEventFactoryInterface::DestroyTriggerEvent
   */
  virtual void DestroyTriggerEvent( TriggerEventInterface* triggerEventInterface );

};

} // namespace Dali

#endif // DALI_INTEGRATION_TRIGGER_EVENT_FACTORY_H
