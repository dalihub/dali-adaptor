#ifndef __DALI_SHADER_EFFECT_WRAPPER_H__
#define __DALI_SHADER_EFFECT_WRAPPER_H__

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

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Creates a Dali shder effect
 *
 * @param[in] vertexPrefix The ShaderEffect vertexPrefix
 * @param[in] vertex The ShaderEffect vertex
 * @param[in] fragmentPrefix The ShaderEffect fragmentPrefix
 * @param[in] fragment The ShaderEffect fragment
 * @param[in] geometryHints The Geometry hints; an integer for Emscripten embind (additive enum)
 *
 * @returns The ShaderEffect
 *
 */
Dali::ShaderEffect CreateShaderEffect( const std::string& vertexPrefix, const std::string& vertex,
                                       const std::string& fragmentPrefix, const std::string& fragment,
                                       int geometryHints );

/**
 * Gets a property by name
 *
 * @param[in] self The ShaderEffect
 * @param[in] name The uniform name
 * @param[in] propertyValue The PropertyValue
 *
 */
void SetUniform( Dali::ShaderEffect& self, const std::string& name, Dali::Property::Value& propertyValue );

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
