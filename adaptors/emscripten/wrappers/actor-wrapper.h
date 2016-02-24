#ifndef __DALI_ACTOR_WRAPPER_H__
#define __DALI_ACTOR_WRAPPER_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/dali-core.h>
#include "emscripten/emscripten.h"
#include "emscripten/val.h"

// INTERNAL INCLUDES
#include "signal-holder.h"

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Exposes a representation of the actor base object pointer for javascript debugging
 *
 * @param[in] self The dali actor
 *
 * @return the address as an int
 */
unsigned int AddressOf(Dali::Actor self);

/**
 * Provides screen to local values conveniently for javascript
 *
 * @param[in] self The dali actor
 * @param[in] screenX The screen X coordinate
 * @param[in] screenY The screen Y coordinate
 *
 * @return the local coordinates
 */
std::vector<float> ScreenToLocal(Dali::Actor self, float screenX, float screenY);

/**
 * Connect a javascript function to a dali signal.
 *
 * Handles touched and hovered specially to provide more context than Dali provides.
 *
 * @param[in] actor The dali actor
 * @param[in] signalHolder The Dali signal holder
 * @param[in] signalName The name of the signal
 * @param[in] javascriptFunction The function to call back when the signal is triggered
 *
 * @return the local coordinates
 *
 */
bool ConnectSignal( Dali::Actor actor,
                    SignalHolder& signalHolder,
                    const std::string& signalName,
                    const emscripten::val& javascriptFunction );

/**
 * Sets a javascript function to an actor property notification
 *
 * @param[in] self The dali actor
 * @param[in] signalHolder The Dali signal holder
 * @param[in] index The property Index
 * @param[in] propertyConditionType The condition type name
 * @param[in] arg0 The property notification arg0
 * @param[in] arg1 The property notification arg1
 * @param[in] propertyConditionType The condition type name
 * @param[in] javascriptFunction The function to call back when the signal is triggered
 *
 * @return the local coordinates

 */
void SetPropertyNotification( Dali::Actor self,
                              SignalHolder& signalHolder,
                              Dali::Property::Index index, const std::string& propertyConditionType, float arg0, float arg1,
                              const emscripten::val& javascriptFunction);


}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
