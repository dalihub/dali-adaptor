#ifndef __DALI_PROPERTY_VALUE_WRAPPER_H__
#define __DALI_PROPERTY_VALUE_WRAPPER_H__

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
#include <string>
#include <dali/public-api/dali-core.h>
#include "emscripten/val.h"

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Sets a property value From an Emscripten value. Recursively for Maps and Arrays
 *
 * @param[in] propertyValue The property value to set
 * @param[in] fromVal The emscripten value to set from
 *
 */
void RecursiveSetProperty(Dali::Property::Value& propertyValue, const emscripten::val& fromVal);

/**
 * Set an Emscripten value from a Dali property Value
 *
 * @param[in] value The Dali Property value
 *
 * @returns The Emscripten value
 *
 */
emscripten::val JavascriptValue( const Dali::Property::Value& value );

/**
 * Gets a Dali value from a Dali Property Map
 *
 * @param[in] self The property map
 * @param[in] key The key of the value to fetch
 *
 * @returns The property value
 *
 */
Dali::Property::Value PropertyMapGet( Dali::Property::Map& self, const std::string& key );

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
