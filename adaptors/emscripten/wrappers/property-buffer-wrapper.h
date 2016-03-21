#ifndef __DALI_PROPERTYBUFFER_WRAPPER_H__
#define __DALI_PROPERTYBUFFER_WRAPPER_H__

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
#include <dali/devel-api/object/property-buffer.h>
#include "emscripten/emscripten.h"
#include "emscripten/val.h"
#include "emscripten/bind.h"

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Sets a property buffer from raw data
 *
 * @param[in] self The PropertyBuffer
 * @param[in] data The raw byte data
 *
 */
void SetPropertyBufferDataRaw(Dali::PropertyBuffer& self, const std::string& data );

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
