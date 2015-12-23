/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <vconf.h>

#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <singleton-service-impl.h>
#include <system-settings.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

const char * DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_DBUS_TTS = "db/setting/accessibility/atspi";

bool GetEnabledVConf()
{
  int isEnabled = 0;
  vconf_get_bool( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_DBUS_TTS, &isEnabled );

  if( isEnabled == 0 )
  {
    vconf_get_bool( VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &isEnabled );
  }

  return (bool)isEnabled;
}

#if defined(DEBUG_ENABLED)
Debug::Filter* gAccessibilityAdaptorLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_ACCESSIBILITY_ADAPTOR" );
#endif

void AccessibilityOnOffNotification(keynode_t* node, void* data)
{
  AccessibilityAdaptor* adaptor = static_cast<AccessibilityAdaptor*>( data );

  bool isEnabled = GetEnabledVConf();

  DALI_LOG_INFO( gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, isEnabled ? "ENABLED" : "DISABLED" );

  if( isEnabled )
  {
    adaptor->EnableAccessibility();
  }
  else
  {
    adaptor->DisableAccessibility();
  }
}

BaseHandle Create()
{
  BaseHandle handle( AccessibilityAdaptor::Get() );

  if ( !handle )
  {
    Dali::SingletonService service( SingletonService::Get() );
    if ( service )
    {
      Dali::AccessibilityAdaptor adaptor = Dali::AccessibilityAdaptor( new AccessibilityAdaptor() );
      AccessibilityAdaptor& adaptorImpl = AccessibilityAdaptor::GetImplementation( adaptor );

      bool isEnabled = GetEnabledVConf();

      if( isEnabled )
      {
        adaptorImpl.EnableAccessibility();
      }
      DALI_LOG_INFO( gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, isEnabled ? "ENABLED" : "DISABLED" );

      vconf_notify_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_DBUS_TTS, AccessibilityOnOffNotification, &adaptorImpl );
      vconf_notify_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, AccessibilityOnOffNotification, &adaptorImpl );

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
  vconf_ignore_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, AccessibilityOnOffNotification );
  vconf_ignore_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_DBUS_TTS, AccessibilityOnOffNotification );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
