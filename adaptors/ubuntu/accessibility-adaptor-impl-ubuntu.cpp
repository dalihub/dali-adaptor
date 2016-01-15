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

// CLASS HEADER
#include "accessibility-adaptor-impl.h"

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <singleton-service-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace // unnamed namespace
{

BaseHandle Create()
{
  BaseHandle handle( AccessibilityAdaptor::Get() );

  if ( !handle )
  {
    Dali::SingletonService service( SingletonService::Get() );
    if ( service )
    {
      Dali::AccessibilityAdaptor adaptor = Dali::AccessibilityAdaptor( new AccessibilityAdaptor() );
      service.Register( typeid( adaptor ), adaptor );
      handle = adaptor;
    }
  }

  return handle;
}

TypeRegistration ACCESSIBILITY_ADAPTOR_TYPE( typeid(Dali::AccessibilityAdaptor), typeid(Dali::BaseHandle), Create, true /* Create Instance At Startup */ );

} // unnamed namespace

Dali::AccessibilityAdaptor AccessibilityAdaptor::Get()
{
  Dali::AccessibilityAdaptor adaptor;

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::AccessibilityAdaptor ) );
    if(handle)
    {
      // If so, downcast the handle
      adaptor = Dali::AccessibilityAdaptor( dynamic_cast< AccessibilityAdaptor* >( handle.GetObjectPtr() ) );
    }
  }

  return adaptor;
}

void AccessibilityAdaptor::OnDestroy()
{
  // Nothing to do here
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
