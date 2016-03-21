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

#include "property-buffer-wrapper.h"

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include "property-value-wrapper.h"

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

void SetPropertyBufferDataRaw(Dali::PropertyBuffer& self, const std::string& data, std::size_t size )
{
  self.SetData( reinterpret_cast<void*>( const_cast<char*>(data.c_str()) ), size );
}


}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
