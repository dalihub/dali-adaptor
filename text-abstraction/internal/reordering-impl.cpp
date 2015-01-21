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

// CLASS  HEADER
#include "reordering-impl.h"

// INTERNAL INCLUDES
#include <singleton-service-impl.h>

namespace Dali
{

namespace Internal
{

namespace TextAbstraction
{


Reordering::Reordering()
:mPlugin(NULL)
{

}

Reordering::~Reordering()
{

}

Dali::TextAbstraction::Reordering Reordering::Get()
{
  Dali::TextAbstraction::Reordering reorderingHandle;

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
     // Check whether the singleton is already created
     Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::TextAbstraction::Reordering ) );
     if(handle)
     {
       // If so, downcast the handle
       Reordering* impl = dynamic_cast< Dali::Internal::TextAbstraction::Reordering* >( handle.GetObjectPtr() );
       reorderingHandle = Dali::TextAbstraction::Reordering( impl );
     }
     else // create and register the object
     {
       reorderingHandle = Dali::TextAbstraction::Reordering( new Reordering);
       service.Register( typeid( reorderingHandle ), reorderingHandle );
     }
   }

   return reorderingHandle;
}

} // namespace Reordering
} // namespace Internal
} // namespace Dali
