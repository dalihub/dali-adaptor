/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/internal/accessibility/common/accessibility-adaptor-impl.h>

// EXTERNAL INCLUDES
#ifndef DALI_PROFILE_UBUNTU
#include <system_settings.h>
#endif
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/touch-integ.h>
#include <dali/integration-api/events/hover-event-integ.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/system-settings.h>

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

} // unnamed namespace

AccessibilityAdaptor::AccessibilityAdaptor()
: mReadPosition(),
  mFocusedActorPosition(-1.0f, -1.0f),
  mActionHandler( NULL ),
  mIsEnabled( false ),
  mIsForced( false )
{
  mAccessibilityGestureDetector = new AccessibilityGestureDetector();
}

void AccessibilityAdaptor::EnableAccessibility()
{
  bool accessibilityState = false;
#ifndef DALI_PROFILE_UBUNTU
  system_settings_get_value_bool( SYSTEM_SETTINGS_KEY_ACCESSIBILITY_TTS, &accessibilityState );
#endif
  if (accessibilityState == false)
  {
    DALI_LOG_ERROR("The Current Accessibility system cannot run. \n");
    return;
  }

  if (mIsEnabled == false)
  {
    mIsEnabled = true;

    SetFocusedActorPosition(Dali::Vector2(-1.0f, -1.0f));

    if (mActionHandler)
    {
      DALI_LOG_ERROR("[FYI] AccessibilityAdaptor::EnableAccessibility() \n");
      mActionHandler->ChangeAccessibilityStatus();
    }
  }
}

void AccessibilityAdaptor::DisableAccessibility()
{
  if (mIsEnabled == true)
  {
    mIsEnabled = false;

    if (mActionHandler)
    {
      DALI_LOG_ERROR("[FYI] AccessibilityAdaptor::DisableAccessibility() \n");
      mActionHandler->ChangeAccessibilityStatus();
    }

    // Destroy the TtsPlayer if exists.
    if (Adaptor::IsAvailable())
    {
      Dali::Adaptor& adaptor = Dali::Adaptor::Get();
      Adaptor& adaptorImpl = Adaptor::GetImplementation( adaptor );
      adaptorImpl.DestroyTtsPlayer( Dali::TtsPlayer::SCREEN_READER );
    }
  }
}

bool AccessibilityAdaptor::IsEnabled() const
{
  return mIsEnabled;
}

void AccessibilityAdaptor::SetForcedEnable( bool forced )
{
  mIsForced = forced;
}

bool AccessibilityAdaptor::IsForcedEnable() const
{
  return mIsForced;
}

Vector2 AccessibilityAdaptor::GetReadPosition() const
{
  return mReadPosition;
}

void AccessibilityAdaptor::SetActionHandler(AccessibilityActionHandler& handler)
{
  mActionHandler = &handler;
}

void AccessibilityAdaptor::SetGestureHandler(AccessibilityGestureHandler& handler)
{
  if( mAccessibilityGestureDetector )
  {
    mAccessibilityGestureDetector->SetGestureHandler(handler);
  }
}

void AccessibilityAdaptor::SetFocusedActorPosition(Vector2 currentPosition)
{
  mFocusedActorPosition = currentPosition;
}

Vector2 AccessibilityAdaptor::GetFocusedActorPosition() const
{
  return mFocusedActorPosition;
}

bool AccessibilityAdaptor::HandleActionNextEvent(bool allowEndFeedback)
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionNext(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionPreviousEvent(bool allowEndFeedback)
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionPrevious(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionActivateEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionActivate();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionReadEvent(unsigned int x, unsigned int y, bool allowReadAgain)
{
  bool ret = false;

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %d , %d\n", __FUNCTION__, __LINE__, x, y);

  mReadPosition.x = static_cast< float > (x);
  mReadPosition.y = static_cast< float > (y);

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionRead( allowReadAgain );
    DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");
  }

  return ret;
}

bool AccessibilityAdaptor::HandleActionReadNextEvent(bool allowEndFeedback)
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionReadNext(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionReadPreviousEvent(bool allowEndFeedback)
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionReadPrevious(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionUpEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionUp();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionDownEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionDown();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionClearFocusEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->ClearAccessibilityFocus();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionScrollEvent(const TouchPoint& point, uint32_t timeStamp)
{
  bool ret = false;

  // We always need to emit a scroll signal, whether it's only a hover or not,
  // so always send the action to the action handler.
  if( mActionHandler )
  {
    Dali::TouchEvent touch = Integration::NewTouchEvent( timeStamp, point );
    ret = mActionHandler->AccessibilityActionScroll( touch );
  }

  Integration::TouchEvent touchEvent;
  Integration::HoverEvent hoverEvent;
  Integration::TouchEventCombiner::EventDispatchType type = mCombiner.GetNextTouchEvent( Integration::Point( point ), timeStamp, touchEvent, hoverEvent );
  if( type == Integration::TouchEventCombiner::DISPATCH_TOUCH || type == Integration::TouchEventCombiner::DISPATCH_BOTH ) // hover event is ignored
  {
    // Process the touch event in accessibility gesture detector
    if( mAccessibilityGestureDetector )
    {
      mAccessibilityGestureDetector->SendEvent( touchEvent );
      ret = true;
    }
  }

  return ret;
}

bool AccessibilityAdaptor::HandleActionBackEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionBack();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

void AccessibilityAdaptor::HandleActionEnableEvent()
{
  EnableAccessibility();
}

void AccessibilityAdaptor::HandleActionDisableEvent()
{
  DisableAccessibility();
}

bool AccessibilityAdaptor::HandleActionScrollUpEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionScrollUp();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}


bool AccessibilityAdaptor::HandleActionScrollDownEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionScrollDown();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionPageLeftEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionPageLeft();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionPageRightEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionPageRight();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionPageUpEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionPageUp();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionPageDownEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionPageDown();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionMoveToFirstEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionMoveToFirst();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionMoveToLastEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionMoveToLast();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionReadFromTopEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionReadFromTop();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionReadFromNextEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionReadFromNext();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionZoomEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionZoom();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionReadPauseResumeEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionReadPauseResume();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionStartStopEvent()
{
  bool ret = false;

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionStartStop();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityAdaptor::HandleActionForwardToAppEvent()
{
  bool ret = false;
  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionForwardToApp();
  }

  DALI_LOG_INFO(gAccessibilityAdaptorLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

AccessibilityAdaptor::~AccessibilityAdaptor()
{
  // Do any platform specific clean-up in OnDestroy()
  OnDestroy();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
