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
#include "accessibility-manager-impl.h"

// EXTERNAL INCLUDES
#include <vconf.h>

#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/hover-event-integ.h>
#include <dali/integration-api/events/gesture-requests.h>

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
#if defined(DEBUG_ENABLED)
Debug::Filter* gAccessibilityManagerLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_ACCESSIBILITY_MANAGER");
#endif

void AccessibilityOnOffNotification(keynode_t* node, void* data)
{
  AccessibilityManager* manager = static_cast<AccessibilityManager*>(data);
  int isEnabled = 0;
  vconf_get_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &isEnabled);

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, isEnabled?"ENABLED":"DISABLED");

  if(isEnabled == 1)
  {
    manager->EnableAccessibility();
  }
  else
  {
    manager->DisableAccessibility();
  }
}

BaseHandle Create()
{
  BaseHandle handle( AccessibilityManager::Get() );

  if ( !handle )
  {
    Dali::SingletonService service( SingletonService::Get() );
    if ( service )
    {
      Dali::AccessibilityManager manager = Dali::AccessibilityManager( new AccessibilityManager() );
      service.Register( typeid( manager ), manager );
      handle = manager;
    }
  }

  return handle;
}
TypeRegistration ACCESSIBILITY_MANAGER_TYPE( typeid(Dali::AccessibilityManager), typeid(Dali::BaseHandle), Create, true /* Create Instance At Startup */ );

} // unnamed namespace

Dali::AccessibilityManager AccessibilityManager::Get()
{
  Dali::AccessibilityManager manager;

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::AccessibilityManager ) );
    if(handle)
    {
      // If so, downcast the handle
      manager = Dali::AccessibilityManager( dynamic_cast< AccessibilityManager* >( handle.GetObjectPtr() ) );
    }
  }

  return manager;
}

Vector2 AccessibilityManager::GetReadPosition() const
{
  return mReadPosition;
}

void AccessibilityManager::SetActionHandler(AccessibilityActionHandler& handler)
{
  mActionHandler = &handler;
}

void AccessibilityManager::SetGestureHandler(AccessibilityGestureHandler& handler)
{
  if( mAccessibilityGestureDetector )
  {
    mAccessibilityGestureDetector->SetGestureHandler(handler);
  }
}

bool AccessibilityManager::HandleActionClearFocusEvent()
{
  bool ret = false;

  Dali::AccessibilityManager handle( this );

  /*
   * In order to application decide reading action first,
   * emit ActionClearFocus signal in first, ClearAccessibilityFocus for handler in next
   */
  if ( !mIndicatorFocused )
  {
    if( !mActionClearFocusSignal.Empty() )
    {
      mActionClearFocusSignal.Emit( handle );
    }
  }

  if( mActionHandler )
  {
    ret = mActionHandler->ClearAccessibilityFocus();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionScrollEvent(const TouchPoint& point, unsigned long timeStamp)
{
  bool ret = false;

  Dali::AccessibilityManager handle( this );

  Dali::TouchEvent event(timeStamp);
  event.points.push_back(point);

  /*
   * In order to application decide touch action first,
   * emit ActionScroll signal in first, AccessibilityActionScroll  for handler in next
   */
  if ( !mIndicatorFocused )
  {
    if( !mActionScrollSignal.Empty() )
    {
      mActionScrollSignal.Emit( handle, event );
    }
  }

  Integration::TouchEvent touchEvent;
  Integration::HoverEvent hoverEvent;
  Integration::TouchEventCombiner::EventDispatchType type = mCombiner.GetNextTouchEvent(point, timeStamp, touchEvent, hoverEvent);
  if(type == Integration::TouchEventCombiner::DispatchTouch || type == Integration::TouchEventCombiner::DispatchBoth) // hover event is ignored
  {
    // Process the touch event in accessibility gesture detector
    if( mAccessibilityGestureDetector )
    {
      mAccessibilityGestureDetector->SendEvent(touchEvent);
      ret = true;
    }
  }

  return ret;
}

bool AccessibilityManager::HandleActionTouchEvent(const TouchPoint& point, unsigned long timeStamp)
{
  bool ret = false;

  Dali::TouchEvent touchEvent(timeStamp);
  touchEvent.points.push_back(point);

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionTouch(touchEvent);
  }
  return ret;
}

bool AccessibilityManager::HandleActionBackEvent()
{
  bool ret = false;

  Dali::AccessibilityManager handle( this );

  /*
   * In order to application decide reading action first,
   * emit ActionBack signal in first, AccessibilityActionBack for handler in next
   */
  if ( !mIndicatorFocused )
  {
    if( !mActionBackSignal.Empty() )
    {
      mActionBackSignal.Emit( handle );
    }
  }

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionBack();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

void AccessibilityManager::HandleActionEnableEvent()
{
  EnableAccessibility();
}

void AccessibilityManager::HandleActionDisableEvent()
{
  DisableAccessibility();
}

void AccessibilityManager::EnableAccessibility()
{
  if(mIsEnabled == false)
  {
    mIsEnabled = true;

    if( mActionHandler )
    {
      mActionHandler->ChangeAccessibilityStatus();
    }

    //emit status changed signal
    Dali::AccessibilityManager handle( this );
    mStatusChangedSignal.Emit( handle );
  }
}

void AccessibilityManager::DisableAccessibility()
{
  if(mIsEnabled == true)
  {
    mIsEnabled = false;

    if( mActionHandler )
    {
      mActionHandler->ChangeAccessibilityStatus();
    }

    //emit status changed signal
    Dali::AccessibilityManager handle( this );
    mStatusChangedSignal.Emit( handle );

    // Destroy the TtsPlayer if exists.
    if ( Adaptor::IsAvailable() )
    {
      Dali::Adaptor& adaptor = Dali::Adaptor::Get();
      Adaptor& adaptorIpml = Adaptor::GetImplementation( adaptor );
      adaptorIpml.DestroyTtsPlayer( Dali::TtsPlayer::SCREEN_READER );
    }
  }
}

bool AccessibilityManager::IsEnabled() const
{
  return mIsEnabled;
}

void AccessibilityManager::SetIndicator(Indicator* indicator)
{
  mIndicator = indicator;
}

AccessibilityManager::AccessibilityManager()
: mIsEnabled(false),
  mActionHandler(NULL),
  mIndicator(NULL),
  mIndicatorFocused(false)
{
  int isEnabled = 0;
  vconf_get_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &isEnabled);
  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, isEnabled?"ENABLED":"DISABLED");

  if(isEnabled == 1)
  {
    mIsEnabled = true;
  }
  else
  {
    mIsEnabled = false;
  }

  vconf_notify_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, AccessibilityOnOffNotification, this );

  mAccessibilityGestureDetector = new AccessibilityGestureDetector();
}

AccessibilityManager::~AccessibilityManager()
{
  vconf_ignore_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, AccessibilityOnOffNotification );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
