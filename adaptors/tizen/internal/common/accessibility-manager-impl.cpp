//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include "accessibility-manager-impl.h"

// EXTERNAL INCLUDES
#include <vconf.h>
#include <Ecore_X.h>
#include <Elementary.h>

#include <dali/public-api/dali-core.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/gesture-requests.h>
#include "system-settings.h"

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

  if ( !handle && Adaptor::IsAvailable() )
  {
    Adaptor& adaptorImpl( Adaptor::GetImplementation( Adaptor::Get() ) );
    Dali::AccessibilityManager manager = Dali::AccessibilityManager( new AccessibilityManager() );
    adaptorImpl.RegisterSingleton( typeid( manager ), manager );
    handle = manager;
  }

  return handle;
}
TypeRegistration ACCESSIBILITY_MANAGER_TYPE( typeid(Dali::AccessibilityManager), typeid(Dali::BaseHandle), Create, true /* Create Instance At Startup */ );

} // unnamed namespace

Dali::AccessibilityManager AccessibilityManager::Get()
{
  Dali::AccessibilityManager manager;

  if ( Adaptor::IsAvailable() )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = Dali::Adaptor::Get().GetSingleton( typeid( Dali::AccessibilityManager ) );
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

bool AccessibilityManager::HandleActionNextEvent()
{
  bool ret = false;
  Dali::AccessibilityManager handle( this );

  /*
   * In order to application decide reading action first,
   * emit ActionNext signal in first, AccessibilityActionNext for handler in next
   */
  if( !mIndicatorFocused )
  {
    if( !mActionNextSignalV2.Empty() )
    {
      mActionNextSignalV2.Emit( handle );
    }
  }

  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_HIGHLIGHT_NEXT;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionNext();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionPreviousEvent()
{
  bool ret = false;

  Dali::AccessibilityManager handle( this );

   /*
   * In order to application decide reading action first,
   * emit ActionPrevious signal in first, AccessibilityActionPrevious for handler in next
   */
 if ( !mIndicatorFocused )
  {
    if( !mActionPreviousSignalV2.Empty() )
    {
      mActionPreviousSignalV2.Emit( handle );
    }
  }

  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_HIGHLIGHT_PREV;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionPrevious();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionActivateEvent()
{
  bool ret = false;

  Dali::AccessibilityManager handle( this );

  /*
   * In order to application decide reading action first,
   * emit ActionActivate signal in first, AccessibilityActionActivate for handler in next
   */
  if ( !mIndicatorFocused )
  {
    if( !mActionActivateSignalV2.Empty() )
    {
      mActionActivateSignalV2.Emit( handle );
    }
  }

  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_ACTIVATE;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionActivate();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionReadEvent(unsigned int x, unsigned int y, bool allowReadAgain)
{
  bool ret = false;

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %d , %d\n", __FUNCTION__, __LINE__, x, y);

  mReadPosition.x = x;
  mReadPosition.y = y;

  Dali::AccessibilityManager handle( this );

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
      DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] Indicator area!!!!\n", __FUNCTION__, __LINE__);
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
      Elm_Access_Action_Info actionInfo;
      actionInfo.action_type = ELM_ACCESS_ACTION_UNHIGHLIGHT;

      // Indicator should be unhighlighted
      ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
      DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] Send unhighlight message to indicator!!!!\n", __FUNCTION__, __LINE__);
    }

    mIndicatorFocused = indicatorFocused;

    // Send accessibility READ action information to Indicator
    if( mIndicatorFocused )
    {
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

      DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] Send READ message to indicator!!!!\n", __FUNCTION__, __LINE__);
    }
  }

  if(allowReadAgain)
  {
    /*
     * In order to application decide reading action first,
     * emit ActionRead signal in first, AccessibilityActionRead for handler in next
     */
    if( !mIndicatorFocused )
    {
      if ( !mActionReadSignalV2.Empty() )
      {
        mActionReadSignalV2.Emit( handle );
      }
    }
  }
  else
  {
    /*
     * In order to application decide reading action first,
     * emit ActionRead signal in first, AccessibilityActionRead for handler in next
     */
    if( !mIndicatorFocused )
    {
      if ( !mActionOverSignalV2.Empty() )
      {
        mActionOverSignalV2.Emit( handle );
      }
    }
  }

  if( mActionHandler && !mIndicatorFocused)
  {
    // If Indicator is not focused, the accessibility actions should be handled by the registered
    // accessibility action handler (e.g. focus manager)
    ret = mActionHandler->AccessibilityActionRead(allowReadAgain);
    DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");
  }

  return ret;
}

bool AccessibilityManager::HandleActionReadNextEvent()
{
  bool ret = false;

  Dali::AccessibilityManager handle( this );

  /*
   * In order to application decide reading action first,
   * emit ActionReadNext signal in first, AccessibilityActionReadNext for handler in next
   */
  if ( !mIndicatorFocused )
  {
    if( !mActionReadNextSignalV2.Empty() )
    {
      mActionReadNextSignalV2.Emit( handle );
    }
  }

  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_HIGHLIGHT_NEXT;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionReadNext();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionReadPreviousEvent()
{
  bool ret = false;

  Dali::AccessibilityManager handle( this );

  /*
   * In order to application decide reading action first,
   * emit ActionReadPrevious signal in first, AccessibilityActionReadPrevious for handler in next
   */
  if ( !mIndicatorFocused )
  {
    if( !mActionReadPreviousSignalV2.Empty() )
    {
      mActionReadPreviousSignalV2.Emit( handle );
    }
  }

  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_HIGHLIGHT_PREV;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionReadPrevious();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionUpEvent()
{
  bool ret = false;

  Dali::AccessibilityManager handle( this );

  /*
   * In order to application decide reading action first,
   * emit ActionUp signal in first, AccessibilityActionUp for handler in next
   */
  if ( !mIndicatorFocused )
  {
    if( !mActionUpSignalV2.Empty() )
    {
      mActionUpSignalV2.Emit( handle );
    }
  }

  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_UP;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionUp();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionDownEvent()
{
  bool ret = false;

  Dali::AccessibilityManager handle( this );

  /*
   * In order to application decide reading action first,
   * emit ActionDown signal in first, AccessibilityActionDown for handler in next
   */
  if ( !mIndicatorFocused )
  {
    if( !mActionDownSignalV2.Empty() )
    {
      mActionDownSignalV2.Emit( handle );
    }
  }

  if( mIndicator && mIndicatorFocused )
  {
    Elm_Access_Action_Info actionInfo;
    actionInfo.action_type = ELM_ACCESS_ACTION_DOWN;

    ret = mIndicator->SendMessage(MSG_DOMAIN_CONTROL_ACCESS, actionInfo.action_type, &actionInfo, sizeof(actionInfo));
  }
  else if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionDown();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
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
    if( !mActionClearFocusSignalV2.Empty() )
    {
      mActionClearFocusSignalV2.Emit( handle );
    }
  }

  if( mActionHandler )
  {
    ret = mActionHandler->ClearAccessibilityFocus();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionScrollEvent(TouchPoint& point, unsigned long timeStamp)
{
  bool ret = false;

  Integration::TouchEvent event;
  if (mCombiner.GetNextTouchEvent(point, timeStamp, event))
  {
    // Process the touch event in accessibility gesture detector
    if( mAccessibilityGestureDetector )
    {
      mAccessibilityGestureDetector->SendEvent(event);
      ret = true;
    }
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
    if( !mActionBackSignalV2.Empty() )
    {
      mActionBackSignalV2.Emit( handle );
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
    mStatusChangedSignalV2.Emit( handle );
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
    mStatusChangedSignalV2.Emit( handle );

    // Destroy the TtsPlayer if exists.
    Dali::Adaptor& adaptor = Dali::Adaptor::Get();
    Adaptor::GetImplementation(adaptor).DestroyTtsPlayer(Dali::TtsPlayer::SCREEN_READER);
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
