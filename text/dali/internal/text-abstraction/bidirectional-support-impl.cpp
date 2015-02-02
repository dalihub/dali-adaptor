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
#include "bidirectional-support-impl.h"

// INTERNAL INCLUDES
#include <singleton-service-impl.h>

namespace Dali
{

namespace Internal
{

namespace TextAbstraction
{

BidirectionalSupport::BidirectionalSupport()
: mPlugin( NULL )
{

}

BidirectionalSupport::~BidirectionalSupport()
{

}

Dali::TextAbstraction::BidirectionalSupport BidirectionalSupport::Get()
{
  Dali::TextAbstraction::BidirectionalSupport bidirectionalSupportHandle;

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
     // Check whether the singleton is already created
     Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::TextAbstraction::BidirectionalSupport ) );
     if(handle)
     {
       // If so, downcast the handle
       BidirectionalSupport* impl = dynamic_cast< Dali::Internal::TextAbstraction::BidirectionalSupport* >( handle.GetObjectPtr() );
       bidirectionalSupportHandle = Dali::TextAbstraction::BidirectionalSupport( impl );
     }
     else // create and register the object
     {
       bidirectionalSupportHandle = Dali::TextAbstraction::BidirectionalSupport( new BidirectionalSupport);
       service.Register( typeid( bidirectionalSupportHandle ), bidirectionalSupportHandle );
     }
   }

   return bidirectionalSupportHandle;
}

Dali::TextAbstraction::BidiInfoIndex BidirectionalSupport::CreateInfo( const Dali::TextAbstraction::Character* const paragraph,
                                                                       Dali::TextAbstraction::Length numberOfCharacters )
{
  return 0u;
}

void BidirectionalSupport::DestroyInfo( Dali::TextAbstraction::BidiInfoIndex bidiInfoIndex )
{
}

void BidirectionalSupport::Reorder( Dali::TextAbstraction::BidiInfoIndex bidiInfoIndex,
                                    Dali::TextAbstraction::CharacterIndex firstCharacterIndex,
                                    Dali::TextAbstraction::Length numberOfCharacters,
                                    Dali::TextAbstraction::CharacterIndex* visualToLogicalMap )
{
}

} // namespace TextAbstraction

} // namespace Internal

} // namespace Dali
