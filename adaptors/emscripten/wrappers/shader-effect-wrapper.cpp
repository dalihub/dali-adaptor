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

#include "handle-wrapper.h"

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include "shader-effect-wrapper.h"
#include "emscripten-utils.h"

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

Dali::ShaderEffect CreateShaderEffect( const std::string& vertexPrefix, const std::string& vertex,
                                       const std::string& fragmentPrefix, const std::string& fragment,
                                       int geometryHints )
{
  Dali::ShaderEffect e = Dali::ShaderEffect::NewWithPrefix(vertexPrefix, vertex,
                                                           fragmentPrefix, fragment,
                                                           static_cast<Dali::ShaderEffect::GeometryHints>(geometryHints));

  return e;
}


void SetUniform( Dali::ShaderEffect& self, const std::string& name, Dali::Property::Value& propertyValue )
{
  switch(propertyValue.GetType())
  {
    case Dali::Property::FLOAT:
    {
      self.SetUniform( name, propertyValue.Get<float>() );
      break;
    };
    case Dali::Property::VECTOR2:
    {
      self.SetUniform( name, propertyValue.Get<Dali::Vector2>() );
      break;
    };
    case Dali::Property::VECTOR3:
    {
      self.SetUniform( name, propertyValue.Get<Dali::Vector3>() );
      break;
    };
    case Dali::Property::VECTOR4:
    {
      self.SetUniform( name, propertyValue.Get<Dali::Vector4>() );
      break;
    };
    case Dali::Property::MATRIX:
    {
      self.SetUniform( name, propertyValue.Get<Dali::Matrix>() );
      break;
    };
    case Dali::Property::MATRIX3:
    {
      self.SetUniform( name, propertyValue.Get<Dali::Matrix3>() );
      break;
    };
    case Dali::Property::NONE:
    case Dali::Property::BOOLEAN:
    case Dali::Property::INTEGER:
    case Dali::Property::ROTATION:
    case Dali::Property::RECTANGLE:
    case Dali::Property::STRING:
    case Dali::Property::ARRAY:
    case Dali::Property::MAP:
    {
      break;
    }
  }
}


}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
