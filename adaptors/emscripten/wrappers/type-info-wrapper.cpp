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

#include "type-info-wrapper.h"

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include "property-value-wrapper.h"

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

std::vector<std::string> GetAllProperties(Dali::TypeInfo self)
{
  std::vector<std::string> names;

  // get the other properties
  if(Dali::Handle handle = Dali::Handle::DownCast( self.CreateInstance() ) )
  {
    typedef Dali::Property::IndexContainer IndexContainer;

    Dali::Property::IndexContainer indices;
    handle.GetPropertyIndices( indices );

    for(IndexContainer::Iterator iter(indices.Begin()); iter != indices.End(); ++iter)
    {
      std::string name = handle.GetPropertyName( *iter );

      names.push_back(name);
    }
  }
  else
  {
    // all we can do is get the event side properties
    // get the event side properties
    Property::IndexContainer indices;
    self.GetPropertyIndices( indices );
    for(Property::IndexContainer::Iterator iter(indices.Begin()); iter != indices.End(); ++iter)
    {
      std::string name = self.GetPropertyName( *iter );
      names.push_back(name);
    }
  }

  return names;
}

std::vector<int> GetPropertyIndices(Dali::TypeInfo& self)
{
  Dali::Property::IndexContainer indices;
  self.GetPropertyIndices( indices );

  std::vector<int> ret( indices.Begin(), indices.End() );
  return ret;
}

std::vector<std::string> GetActions(Dali::TypeInfo& self)
{
  std::vector<std::string> names;
  std::size_t size = self.GetActionCount();
  for(std::size_t i = 0; i < size; i++)
  {
    names.push_back(self.GetActionName(i));
  }
  return names;
}

std::vector<std::string> GetSignals(Dali::TypeInfo& self)
{
  std::vector<std::string> names;
  std::size_t size = self.GetSignalCount();
  for(std::size_t i = 0; i < size; i++)
  {
    names.push_back(self.GetSignalName(i));
  }
  return names;
}

std::vector<std::string> GetBases(Dali::TypeInfo& self)
{
  std::vector<std::string> names;

  Dali::TypeRegistry registry = Dali::TypeRegistry::Get();

  Dali::TypeInfo base = registry.GetTypeInfo( self.GetBaseName() );

  while(base)
  {
    base = registry.GetTypeInfo( base.GetBaseName() );
    names.push_back(base.GetName());
  }

  return names;
}

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
