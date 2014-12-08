#ifndef __DALI_TYPE_INFO_WRAPPER_H__
#define __DALI_TYPE_INFO_WRAPPER_H__

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
#include <vector>
#include <string>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Gets all properties from TypeInfo
 *
 * @param[in] self The TypeInfo
 *
 * @returns A list of property names
 *
 */
std::vector<std::string> GetAllProperties(Dali::TypeInfo self);

/**
 * Gets all property indices from TypeInfo
 *
 * @param[in] self The TypeInfo
 *
 * @returns A list of property indices
 *
 */
std::vector<int> GetPropertyIndices(Dali::TypeInfo& self);

/**
 * Gets all actions from TypeInfo
 *
 * @param[in] self The TypeInfo
 *
 * @returns A list of property action names
 *
 */
std::vector<std::string> GetActions(Dali::TypeInfo& self);

/**
 * Gets all signals from TypeInfo
 *
 * @param[in] self The TypeInfo
 *
 * @returns A list of signal names
 *
 */
std::vector<std::string> GetSignals(Dali::TypeInfo& self);

/**
 * Gets all bases classes from TypeInfo
 *
 * @param[in] self The TypeInfo
 *
 * @returns A list of base class names
 *
 */
std::vector<std::string> GetBases(Dali::TypeInfo& self);

/**
 * Check if a TypeInfo has a base name
 *
 * @param[in] self The TypeInfo
 * @param[in] baseName The base class name
 *
 * @returns true if baseName is a base class of self
 *
 */
bool InheritsFrom(Dali::TypeInfo& self, const std::string& baseName);

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
