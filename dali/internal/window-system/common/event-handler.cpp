/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/common/event-handler.h>

// EXTERNAL INCLUDES
#include <sys/time.h>
#include <algorithm>
#include <cstring>

#include <dali/devel-api/events/touch-point.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/hover-event-integ.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/wheel-event-integ.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/wheel-event.h>

// INTERNAL INCLUDES
#include <dali/internal/clipboard/common/clipboard-impl.h>
#include <dali/internal/styling/common/style-monitor-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#if defined(DEBUG_ENABLED)
namespace
{
Integration::Log::Filter* gSelectionEventLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_SELECTION");
} // unnamed namespace
#endif

EventHandler::EventHandler(WindowBase* windowBase, DamageObserver& damageObserver)
: mStyleMonitor(StyleMonitor::Get()),
  mDamageObserver(damageObserver),
  mPaused(false)
{
  // Connect signals
  if(windowBase)
  {
    windowBase->WindowDamagedSignal().Connect(this, &EventHandler::OnWindowDamaged);
    windowBase->FocusChangedSignal().Connect(this, &EventHandler::OnFocusChanged);
    windowBase->RotationSignal().Connect(this, &EventHandler::OnRotation);
    windowBase->TouchEventSignal().Connect(this, &EventHandler::OnTouchEvent);
    windowBase->MouseFrameEventSignal().Connect(this, &EventHandler::OnMouseFrameEvent);
    windowBase->WheelEventSignal().Connect(this, &EventHandler::OnWheelEvent);
    windowBase->KeyEventSignal().Connect(this, &EventHandler::OnKeyEvent);
    windowBase->SelectionDataSendSignal().Connect(this, &EventHandler::OnSelectionDataSend);
    windowBase->SelectionDataReceivedSignal().Connect(this, &EventHandler::OnSelectionDataReceived);
    windowBase->StyleChangedSignal().Connect(this, &EventHandler::OnStyleChanged);
  }
  else
  {
    DALI_LOG_ERROR("WindowBase is invalid!!!\n");
  }
}

EventHandler::~EventHandler()
{
}

void EventHandler::SendEvent(StyleChange::Type styleChange)
{
  DALI_ASSERT_DEBUG(mStyleMonitor && "StyleMonitor Not Available");
  GetImplementation(mStyleMonitor).StyleChanged(styleChange);
}

void EventHandler::SendEvent(const DamageArea& area)
{
  mDamageObserver.OnDamaged(area);
}

void EventHandler::Pause()
{
  mPaused = true;
}

void EventHandler::Resume()
{
  mPaused = false;
}

void EventHandler::OnTouchEvent(Integration::Point& point, uint32_t timeStamp)
{
  for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
  {
    (*iter)->OnTouchPoint(point, timeStamp);
  }
}

void EventHandler::OnMouseFrameEvent()
{
  for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
  {
    (*iter)->OnMouseFrameEvent();
  }
}

void EventHandler::OnWheelEvent(Integration::WheelEvent& wheelEvent)
{
  for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
  {
    (*iter)->OnWheelEvent(wheelEvent);
  }
}

void EventHandler::OnKeyEvent(Integration::KeyEvent& keyEvent)
{
  for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
  {
    (*iter)->OnKeyEvent(keyEvent);
  }
}

void EventHandler::OnFocusChanged(bool focusIn)
{
  // If the window gains focus and we hid the keyboard then show it again.
  if(Clipboard::IsAvailable())
  {
    if(focusIn)
    {
      Dali::Clipboard clipboard = Clipboard::Get();
      if(clipboard)
      {
        clipboard.HideClipboard();
      }
    }
    else
    {
      // Hiding clipboard event will be ignored once because window focus out event is always received on showing clipboard
      Dali::Clipboard clipboard = Clipboard::Get();
      if(clipboard)
      {
        Clipboard& clipBoardImpl(GetImplementation(clipboard));
        clipBoardImpl.HideClipboard(true);
      }
    }
  }
}

void EventHandler::OnRotation(const RotationEvent& event)
{
  for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
  {
    (*iter)->OnRotation(event);
  }
}

void EventHandler::OnWindowDamaged(const DamageArea& area)
{
  SendEvent(area);
}

void EventHandler::OnSelectionDataSend(void* event)
{
  // Note that the clipboard-related operstions previously available have been moved to Clipboard class.
  // It is advised not to handle any clipboard-specific works within this context.
  // There are currently no immediate works required in this callback.
  // But this function is retained for the purpose of handling the event at the window level, if needed.
}

void EventHandler::OnSelectionDataReceived(void* event)
{
  // Note that the clipboard-related operstions previously available have been moved to Clipboard class.
  // It is advised not to handle any clipboard-specific works within this context.
  // There are currently no immediate works required in this callback.
  // But this function is retained for the purpose of handling the event at the window level, if needed.
}

void EventHandler::OnStyleChanged(StyleChange::Type styleChange)
{
  SendEvent(styleChange);
}

void EventHandler::AddObserver(Observer& observer)
{
  ObserverContainer::iterator match(std::find(mObservers.begin(), mObservers.end(), &observer));

  if(match == mObservers.end())
  {
    mObservers.push_back(&observer);
  }
}

void EventHandler::RemoveObserver(Observer& observer)
{
  ObserverContainer::iterator match(std::find(mObservers.begin(), mObservers.end(), &observer));

  if(match != mObservers.end())
  {
    mObservers.erase(match);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
