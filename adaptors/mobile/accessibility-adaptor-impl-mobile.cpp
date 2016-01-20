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
#include "accessibility-adaptor-impl-mobile.h"

// EXTERNAL INCLUDES
#include <vconf.h>

#ifndef WAYLAND
#include <Ecore_X.h>
#include <Elementary.h>
#endif

#include <vconf.h>

#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/gesture-requests.h>

// INTERNAL INCLUDES
#include <singleton-service-impl.h>
#include "system-settings.h"

#ifndef WAYLAND
#define MSG_DOMAIN_CONTROL_ACCESS (int)ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL
#endif

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace // unnamed namespace
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gAccessibilityAdaptorLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_ACCESSIBILITY_ADAPTOR");
#endif

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
    else
    {
      adaptor = Dali::AccessibilityAdaptor( new AccessibilityAdaptorMobile() );
      AccessibilityAdaptorMobile& adaptorImpl = AccessibilityAdaptorMobile::GetImplementation( adaptor );

      bool isEnabled = GetEnabledVConf();

      if( isEnabled )
      {
        adaptorImpl.EnableAccessibility();
      }
      DALI_LOG_INFO( gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, isEnabled ? "ENABLED" : "DISABLED" );

      vconf_notify_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_DBUS_TTS, AccessibilityOnOffNotification, &adaptorImpl );
      vconf_notify_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, AccessibilityOnOffNotification, &adaptorImpl );

      service.Register( typeid( adaptor ), adaptor );
    }
  }

  return adaptor;
}

void AccessibilityAdaptor::OnDestroy()
{
  vconf_ignore_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, AccessibilityOnOffNotification );
  vconf_ignore_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_DBUS_TTS, AccessibilityOnOffNotification );
}

AccessibilityAdaptorMobile::AccessibilityAdaptorMobile()
{
}

bool AccessibilityAdaptorMobile::HandleActionNextEvent(bool allowEndFeedback)
{
  bool ret = false;

#ifndef WAYLAND
  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_HIGHLIGHT_NEXT;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
#else
  if( mActionHandler )
#endif
  {
    ret = mActionHandler->AccessibilityActionNext(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptorMobile::HandleActionPreviousEvent(bool allowEndFeedback)
{
  bool ret = false;

#ifndef WAYLAND
  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_HIGHLIGHT_PREV;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
#else
  if( mActionHandler )
#endif
  {
    ret = mActionHandler->AccessibilityActionPrevious(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptorMobile::HandleActionActivateEvent()
{
  bool ret = false;

#ifndef WAYLAND
  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_ACTIVATE;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
#else
  if( mActionHandler )
#endif
  {
    ret = mActionHandler->AccessibilityActionActivate();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptorMobile::HandleActionReadEvent(unsigned int x, unsigned int y, bool allowReadAgain)
{
  bool ret = false;

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %d , %d\n", __FUNCTION__, __LINE__, x, y);

  mReadPosition.x = x;
  mReadPosition.y = y;

  Dali::AccessibilityAdaptor handle( this );

  bool indicatorFocused = false;

  // Check whether the Indicator is focused
  if( mIndicator && mIndicator->IsConnected() )
  {
    // Check the position and size of Indicator actor
    Dali::Actor indicatorActor = mIndicator->GetActor();
    Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 size = indicatorActor.GetCurrentSize();

    if(mReadPosition.x >= position.x &&
       mReadPosition.x <= position.x + size.width &&
       mReadPosition.y >= position.y &&
       mReadPosition.y <= position.y + size.height)
    {
      indicatorFocused = true;
      DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] Indicator area!!!!\n", __FUNCTION__, __LINE__);
    }
  }

  if( mIndicator )
  {
    if( !mIndicatorFocused && indicatorFocused )
    {
      // If Indicator is focused, the focus should be cleared in Dali focus chain.
      if( mActionHandler )
      {
        mActionHandler->ClearAccessibilityFocus();
      }
    }
    else if( mIndicatorFocused && !indicatorFocused )
    {
#ifndef WAYLAND
      Elm_Access_Action_Info actionInfo;
      actionInfo.action_type = ELM_ACCESS_ACTION_UNHIGHLIGHT;

      // Indicator should be unhighlighted
      ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
      DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] Send unhighlight message to indicator!!!!\n", __FUNCTION__, __LINE__);
#endif
    }

    mIndicatorFocused = indicatorFocused;

    // Send accessibility READ action information to Indicator
    if( mIndicatorFocused )
    {
#ifndef WAYLAND
      Elm_Access_Action_Info actionInfo;
      actionInfo.x = mReadPosition.x;
      actionInfo.y = mReadPosition.y;

      if(allowReadAgain)
      {
        actionInfo.action_type = ELM_ACCESS_ACTION_READ;
      }
      else
      {
        actionInfo.action_type = static_cast<Elm_Access_Action_Type>( GetElmAccessActionOver() );
      }

      ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));

      DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] Send READ message to indicator!!!!\n", __FUNCTION__, __LINE__);
#endif
    }
  }

  if( mActionHandler && !mIndicatorFocused)
  {
    // If Indicator is not focused, the accessibility actions should be handled by the registered
    // accessibility action handler (e.g. focus manager)
    ret = mActionHandler->AccessibilityActionRead(allowReadAgain);
    DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");
  }

  return ret;
}

bool AccessibilityAdaptorMobile::HandleActionReadNextEvent(bool allowEndFeedback)
{
  bool ret = false;

#ifndef WAYLAND
  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_HIGHLIGHT_NEXT;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
#else
  if( mActionHandler )
#endif
  {
    ret = mActionHandler->AccessibilityActionReadNext(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptorMobile::HandleActionReadPreviousEvent(bool allowEndFeedback)
{
  bool ret = false;

#ifndef WAYLAND
  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_HIGHLIGHT_PREV;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
#else
  if( mActionHandler )
#endif
  {
    ret = mActionHandler->AccessibilityActionReadPrevious(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptorMobile::HandleActionUpEvent()
{
  bool ret = false;

#ifndef WAYLAND
  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_UP;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
#else
  if( mActionHandler )
#endif
  {
    ret = mActionHandler->AccessibilityActionUp();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptorMobile::HandleActionDownEvent()
{
  bool ret = false;

#ifndef WAYLAND
  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_DOWN;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
#else
  if( mActionHandler )
#endif
  {
    ret = mActionHandler->AccessibilityActionDown();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

AccessibilityAdaptorMobile::~AccessibilityAdaptorMobile()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
