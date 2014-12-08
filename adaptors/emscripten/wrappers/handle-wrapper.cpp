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
#include <sstream>
#include <dali/devel-api/scripting/scripting.h>

// INTERNAL INCLUDES
#include "type-info-wrapper.h"
#include "property-value-wrapper.h"

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

bool BaseHandleOk(Dali::BaseHandle& self)
{
  return self;
}

void SetSelf(Dali::Handle& self, Dali::Handle& other)
{
  self = other;
}

void SetProperty(Dali::Handle& self, const std::string& name, const Dali::Property::Value& value)
{
  DALI_ASSERT_ALWAYS(self);
  if( self )
  {
    Dali::Property::Index index = self.GetPropertyIndex(name);

    if( Dali::Property::INVALID_INDEX != index )
    {
      self.SetProperty(index, value);
    }
    else
    {
      printf("ERR Invalid property name:%s", name.c_str());
      EM_ASM( throw "Invalid property name (HandleWrapper::SetProperty)" );
    }
  }
  else
  {
    EM_ASM( throw "ActorWrapper has no actor" );
  }

}

Dali::Property::Value GetProperty(Dali::Handle& self, const std::string& name)
{
  DALI_ASSERT_ALWAYS(self);
  Dali::Property::Value ret;
  if( self )
  {
    Dali::Property::Index index = self.GetPropertyIndex(name);

    if( Dali::Property::INVALID_INDEX != index )
    {
      ret = self.GetProperty(index);
    }
    else
    {
      printf("ERR Invalid property name:%s", name.c_str());
      EM_ASM( throw new Error("Invalid property name (HandleWrapper::GetProperty)") );
    }
  }
  else
  {
    EM_ASM( throw new Error("ActorWrapper has no actor") );
  }

  return ret;
}

int GetPropertyIndex(Dali::Handle& self, const std::string& name)
{
  if( self )
  {
    Dali::Property::Index index = self.GetPropertyIndex(name);

    return (int)index; // self.GetPropertyIndex(name);
  }

  return -1;
}

std::vector<std::string> GetProperties(Dali::Handle& self)
{
  Dali::Property::IndexContainer indices;
  self.GetPropertyIndices( indices );
  std::vector<std::string> names;
  for(Dali::Property::IndexContainer::Iterator iter(indices.Begin()); iter != indices.End(); ++iter)
  {
    std::string name = self.GetPropertyName( *iter );

    names.push_back(name);
  }
  return names;
}

std::string GetPropertyTypeName(Dali::Handle& self, const std::string& name)
{
  if(self)
  {
    Dali::Property::Index index = self.GetPropertyIndex(name);
    if(Dali::Property::INVALID_INDEX != index)
    {
      return Dali::PropertyTypes::GetName(self.GetPropertyType(index));
    }
  }

  // if we got here
  return Dali::PropertyTypes::GetName(Dali::Property::NONE);
}

Dali::Property::Type GetPropertyTypeFromName(Dali::Handle& self, const std::string& name)
{
  Dali::Property::Type type = Dali::Property::NONE;

  if(self)
  {
    Dali::Property::Index index = self.GetPropertyIndex(name);
    if(Dali::Property::INVALID_INDEX != index)
    {
      type = self.GetPropertyType(index);
    }
  }

  return type;
}

Dali::Property::Index RegisterProperty(Dali::Handle& self, const std::string& name, const Dali::Property::Value& propertyValue)
{
  Dali::Property::Index ret = Dali::Property::INVALID_INDEX;

  Dali::Property::Type type = propertyValue.GetType();
  if(Dali::Property::ARRAY == type || Dali::Property::MAP == type)
  {
    // these types would need support in the javascript side of the wrapper
    EM_ASM( throw "Property type not supported" );
  }

  if(self)
  {
    ret = self.RegisterProperty(name, propertyValue, Dali::Property::AccessMode::READ_WRITE);
  }
  return ret;
}

Dali::TypeInfo GetTypeInfo(Dali::Handle& self)
{
  Dali::TypeInfo ret;
  if( self )
  {
    self.GetTypeInfo(ret);
  }
  return ret;
}

Dali::Property::Index RegisterAnimatedProperty(Dali::Handle& self, const std::string& name, const Dali::Property::Value& propertyValue)
{
  Dali::Property::Index ret = Dali::Property::INVALID_INDEX;

  Dali::Property::Type type = propertyValue.GetType();
  if(Dali::Property::ARRAY == type || Dali::Property::MAP == type)
  {
    // these types would need support in the javascript side of the wrapper
    EM_ASM( throw "Property type not supported" );
  }

  if(self)
  {
    ret = self.RegisterProperty(name, propertyValue,  Dali::Property::AccessMode::ANIMATABLE);
  }
  return ret;
}


}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
