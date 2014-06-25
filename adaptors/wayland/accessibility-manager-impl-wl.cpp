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
#include <Ecore.h>
#include <Ecore_Wayland.h>
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

bool AccessibilityManager::HandleActionNextEvent(bool allowEndFeedback)
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

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionNext(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionPreviousEvent(bool allowEndFeedback)
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

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionPrevious(allowEndFeedback);
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

  if( mActionHandler )
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
    }

    mIndicatorFocused = indicatorFocused;

    // Send accessibility READ action information to Indicator
    if( mIndicatorFocused )
    {
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

bool AccessibilityManager::HandleActionReadNextEvent(bool allowEndFeedback)
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

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionReadNext(allowEndFeedback);
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

bool AccessibilityManager::HandleActionReadPreviousEvent(bool allowEndFeedback)
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

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionReadPrevious(allowEndFeedback);
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

  if( mActionHandler )
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

  if( mActionHandler )
  {
    ret = mActionHandler->AccessibilityActionDown();
  }

  DALI_LOG_INFO(gAccessibilityManagerLogFilter, Debug::General, "[%s:%d] %s\n", __FUNCTION__, __LINE__, ret?"TRUE":"FALSE");

  return ret;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
