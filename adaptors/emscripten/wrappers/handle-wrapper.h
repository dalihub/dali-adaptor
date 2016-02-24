#ifndef __DALI_HANDLE_WRAPPER_H__
#define __DALI_HANDLE_WRAPPER_H__

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
#include "emscripten/bind.h"

// INTERNAL INCLUDES
#include "type-info-wrapper.h"


namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Checks if a handle is pointing somewhere
 *
 * @param[in] self The basehandle
 * @returns true if the handle is not empty
 *
 */
bool BaseHandleOk(Dali::BaseHandle& self);

/**
 * Sets a handle to point to another object
 *
 * @param[in] self The handle to change
 * @param[in] self The handle to point to
 *
 */
void SetSelf(Dali::Handle& self, Dali::Handle& other);

/**
 * Sets a property by name
 *
 * @param[in] self The handle
 * @param[in] javascriptName The property by name
 * @param[in] value The property value
 *
 */
void SetProperty(Dali::Handle& self, const std::string& javascriptName, const Dali::Property::Value& value);

/**
 * Gets a property by name
 *
 * @param[in] self The handle
 * @param[in] javascriptName The property by name
 * @returns The property value
 *
 */
Dali::Property::Value GetProperty(Dali::Handle& self, const std::string& javascriptName);

/**
 * Gets a property index
 *
 * @param[in] self The handle
 * @param[in] javascriptName The property by name
 * @returns The property index
 *
 */
int GetPropertyIndex(Dali::Handle& self, const std::string& javascriptName);

/**
 * Gets a list of property names
 *
 * @param[in] self The handle
 * @returns The list of property names
 *
 */
std::vector<std::string> GetProperties(Dali::Handle& self);

/**
 * Gets a property type by name
 *
 * @param[in] self The handle
 * @param[in] name The property type name
 * @returns The property type
 *
 */
Dali::Property::Type GetPropertyTypeFromName(Dali::Handle& self, const std::string& name);

/**
 * Gets a property type name
 *
 * @param[in] self The handle
 * @param[in] name The property by name
 * @returns The property type name
 *
 */
std::string GetPropertyTypeName(Dali::Handle& self, const std::string& name);

/**
 * Registers a property by name
 *
 * @param[in] self The handle
 * @param[in] name The property by name
 * @returns The property index of the newly registered property
 *
 */
Dali::Property::Index RegisterProperty(Dali::Handle& self, const std::string& name, const Dali::Property::Value& propertyValue);

/**
 * Registers an animated property
 *
 * @param[in] self The handle
 * @param[in] name The property by name
 * @returns The property index of the newly registered property
 *
 */
Dali::Property::Index RegisterAnimatedProperty(Dali::Handle& self, const std::string& name, const Dali::Property::Value& propertyValue);

/**
 * Gets Dali type info
 *
 * @param[in] self The handle
 * @returns The type info
 *
 */
Dali::TypeInfo GetTypeInfo(Dali::Handle& self);

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
