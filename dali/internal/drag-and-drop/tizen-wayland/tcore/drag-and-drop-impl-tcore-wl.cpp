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
#include <dali/internal/drag-and-drop/tizen-wayland/tcore/drag-and-drop-impl-tcore-wl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <unistd.h>
#include <cstring>
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/drag-and-drop/common/drag-and-drop-factory.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/tizen/tcore/tizen-core-wl-display-util.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
static constexpr int32_t MAX_MIME_SIZE = 10;
} // namespace

static bool IsIntersection(int px, int py, int tx, int ty, int tw, int th)
{
  return (px >= tx && py >= ty && px <= (tx + tw) && py <= (ty + th));
}

static tizen_core_wl_window_h GetWindowFromNativeHandle(const Dali::Any& handle)
{
  if(handle.Empty())
  {
    return nullptr;
  }
  if(handle.IsType<tizen_core_wl_window_h>())
  {
    return AnyCast<tizen_core_wl_window_h>(handle);
  }
  return nullptr;
}

static void TcoreEventDataSend(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  auto* dndImpl = reinterpret_cast<DragAndDropTcoreWl*>(user_data);
  dndImpl->SendData(event_data);
}

static void TcoreEventDataSourceEnd(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  auto* dndImpl   = reinterpret_cast<DragAndDropTcoreWl*>(user_data);
  bool  cancelled = false;
  if(tizen_core_wl_event_data_source_end_get_cancelled(
       static_cast<tizen_core_wl_event_data_source_end_h>(event_data), &cancelled) == TIZEN_CORE_WL_ERROR_NONE)
  {
    if(cancelled)
    {
      dndImpl->CallSourceEvent(Dali::DragAndDrop::SourceEventType::CANCEL);
    }
    else
    {
      dndImpl->CallSourceEvent(Dali::DragAndDrop::SourceEventType::ACCEPT);
    }
  }
}

static void TcoreEventDataSourceDrop(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  auto* dndImpl = reinterpret_cast<DragAndDropTcoreWl*>(user_data);
  dndImpl->CallSourceEvent(Dali::DragAndDrop::SourceEventType::FINISH);
}

static void TcoreEventOfferDataReady(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  auto* dndImpl = reinterpret_cast<DragAndDropTcoreWl*>(user_data);
  dndImpl->ReceiveData(event_data);
}

static void TcoreEventDataMotion(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  auto* dndImpl = reinterpret_cast<DragAndDropTcoreWl*>(user_data);
  dndImpl->CalculateDragEvent(event_data);
}

static void TcoreEventDataDrop(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  auto* dndImpl = reinterpret_cast<DragAndDropTcoreWl*>(user_data);
  dndImpl->CalculateViewRegion(event_data);
}

static void TcoreEventDataEnter(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  tizen_core_wl_data_offer_h offer = nullptr;
  if(tizen_core_wl_event_data_base_get_offer(static_cast<tizen_core_wl_event_data_base_h>(event_data), &offer) == TIZEN_CORE_WL_ERROR_NONE && offer)
  {
    tizen_core_wl_data_accept(offer, nullptr);
  }
}

static void TcoreEventDataLeave(void* event_data, tizen_core_wl_event_type_e event_type, void* user_data)
{
  auto* dndImpl = reinterpret_cast<DragAndDropTcoreWl*>(user_data);
  dndImpl->ResetDropTargets();
}

Dali::DragAndDrop GetDragAndDrop()
{
  Dali::DragAndDrop      dnd;
  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::DragAndDrop));
    if(handle)
    {
      dnd = Dali::DragAndDrop(static_cast<DragAndDrop*>(handle.GetObjectPtr()));
    }
    else
    {
      auto* dndImpl = new DragAndDropTcoreWl();
      dnd           = Dali::DragAndDrop(dndImpl);
      service.Register(typeid(Dali::DragAndDrop), dnd);
    }
  }
  return dnd;
}

class DragAndDropFactoryTcoreWl : public DragAndDropFactory
{
public:
  Dali::DragAndDrop CreateDragAndDrop() override
  {
    return GetDragAndDrop();
  }
};

std::unique_ptr<DragAndDropFactory> GetDragAndDropFactory()
{
  return std::unique_ptr<DragAndDropFactory>(new DragAndDropFactoryTcoreWl());
}

DragAndDropTcoreWl::DragAndDropTcoreWl()
{
  // Acquire default display and associated event handle for registering listeners.
  tizen_core_wl_display_h display = nullptr;
  if(!TcoreWlAcquireDisplay(&display))
  {
    return;
  }

  tizen_core_event_h event = nullptr;
  if(tizen_core_wl_display_get_event(display, &event) != TIZEN_CORE_WL_ERROR_NONE || !event)
  {
    TcoreWlReleaseDisplay(display);
    return;
  }

  tizen_core_wl_event_listener_h h = nullptr;

  if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_SOURCE_SEND, TcoreEventDataSend, this, &h) == TIZEN_CORE_WL_ERROR_NONE)
  {
    mSendHandler = h;
  }
  if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_SOURCE_END, TcoreEventDataSourceEnd, this, &h) == TIZEN_CORE_WL_ERROR_NONE)
  {
    mSourceEndHandler = h;
  }
  if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_SOURCE_DROP, TcoreEventDataSourceDrop, this, &h) == TIZEN_CORE_WL_ERROR_NONE)
  {
    mSourceDropHandler = h;
  }
  if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_READY, TcoreEventOfferDataReady, this, &h) == TIZEN_CORE_WL_ERROR_NONE)
  {
    mReceiveHandler = h;
  }
  if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_MOTION, TcoreEventDataMotion, this, &h) == TIZEN_CORE_WL_ERROR_NONE)
  {
    mMotionHandler = h;
  }
  if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_DROP, TcoreEventDataDrop, this, &h) == TIZEN_CORE_WL_ERROR_NONE)
  {
    mDropHandler = h;
  }
  if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_ENTER, TcoreEventDataEnter, this, &h) == TIZEN_CORE_WL_ERROR_NONE)
  {
    mEnterHandler = h;
  }
  if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_DATA_LEAVE, TcoreEventDataLeave, this, &h) == TIZEN_CORE_WL_ERROR_NONE)
  {
    mLeaveHandler = h;
  }

  mEvent = event;
  TcoreWlReleaseDisplay(display);
}

DragAndDropTcoreWl::~DragAndDropTcoreWl()
{
  if(mEvent)
  {
    if(mSendHandler)
    {
      tizen_core_wl_event_remove_listener(mEvent, mSendHandler);
    }
    if(mSourceEndHandler)
    {
      tizen_core_wl_event_remove_listener(mEvent, mSourceEndHandler);
    }
    if(mSourceDropHandler)
    {
      tizen_core_wl_event_remove_listener(mEvent, mSourceDropHandler);
    }
    if(mReceiveHandler)
    {
      tizen_core_wl_event_remove_listener(mEvent, mReceiveHandler);
    }
    if(mMotionHandler)
    {
      tizen_core_wl_event_remove_listener(mEvent, mMotionHandler);
    }
    if(mDropHandler)
    {
      tizen_core_wl_event_remove_listener(mEvent, mDropHandler);
    }
    if(mEnterHandler)
    {
      tizen_core_wl_event_remove_listener(mEvent, mEnterHandler);
    }
    if(mLeaveHandler)
    {
      tizen_core_wl_event_remove_listener(mEvent, mLeaveHandler);
    }
  }
}

bool DragAndDropTcoreWl::StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& data, Dali::DragAndDrop::SourceFunction callback)
{
  auto         parent    = Dali::Window::Get(source);
  const char** dataSet   = data.GetDataSet();
  const char** mimeTypes = data.GetMimeTypes();

  mDataMap.clear();
  for(int i = 0; i < data.GetDataSetSize(); ++i)
  {
    mDataMap[std::string(mimeTypes[i])] = std::string(dataSet[i]);
  }

  mSourceCallback = callback;
  mDragWindow     = shadowWindow;

  tizen_core_wl_window_h parentWindow = GetWindowFromNativeHandle(parent.GetNativeHandle());
  tizen_core_wl_window_h dragWindow   = GetWindowFromNativeHandle(mDragWindow.GetNativeHandle());
  if(!parentWindow || !dragWindow)
  {
    return false;
  }

  tizen_core_wl_display_h display   = nullptr;
  tizen_core_wl_seat_h    seat      = nullptr;
  tizen_core_wl_data_h    tcoreData = nullptr;
  if(tizen_core_wl_window_get_display(parentWindow, &display) != TIZEN_CORE_WL_ERROR_NONE || !display)
  {
    return false;
  }
  if(tizen_core_wl_display_get_default_seat(display, &seat) != TIZEN_CORE_WL_ERROR_NONE || !seat)
  {
    return false;
  }
  if(tizen_core_wl_seat_get_data(seat, &tcoreData) != TIZEN_CORE_WL_ERROR_NONE)
  {
    return false;
  }

  int                      mimeTypesCount = data.GetMimeTypesSize();
  std::vector<const char*> waylandMimeTypes(static_cast<size_t>(mimeTypesCount) + 1u);
  for(int i = 0; i < mimeTypesCount; ++i)
  {
    waylandMimeTypes[i] = mimeTypes[i];
  }
  waylandMimeTypes[mimeTypesCount] = nullptr;

  if(tizen_core_wl_data_set_drag_types(tcoreData, waylandMimeTypes.data()) != TIZEN_CORE_WL_ERROR_NONE)
  {
    return false;
  }
  if(tizen_core_wl_data_start_drag(tcoreData, parentWindow, dragWindow, &mSerial) != TIZEN_CORE_WL_ERROR_NONE)
  {
    return false;
  }

  CallSourceEvent(Dali::DragAndDrop::SourceEventType::START);
  return true;
}

bool DragAndDropTcoreWl::AddListener(Dali::Actor target, char* mimeType, Dali::DragAndDrop::DragAndDropFunction callback)
{
  for(auto itr = mDropTargets.begin(); itr != mDropTargets.end(); ++itr)
  {
    if((*itr).target == target)
    {
      return false;
    }
  }

  auto                   window       = Dali::Window::Get(target);
  tizen_core_wl_window_h parentWindow = nullptr;

  if(!window)
  {
    target.SceneConnectedSignal().Connect(this, &DragAndDropTcoreWl::DropTargetSceneOn);
  }
  else
  {
    parentWindow = GetWindowFromNativeHandle(window.GetNativeHandle());
    if(!parentWindow)
    {
      return false;
    }
  }

  DropTarget targetData;
  targetData.target       = target;
  targetData.mimeType     = mimeType;
  targetData.callback     = callback;
  targetData.inside       = false;
  targetData.parentWindow = parentWindow;
  mDropTargets.push_back(targetData);
  return true;
}

bool DragAndDropTcoreWl::AddListener(Dali::Window target, char* mimeType, Dali::DragAndDrop::DragAndDropFunction callback)
{
  for(auto itr = mDropWindowTargets.begin(); itr != mDropWindowTargets.end(); ++itr)
  {
    if((*itr).target == target)
    {
      return false;
    }
  }

  tizen_core_wl_window_h window = GetWindowFromNativeHandle(target.GetNativeHandle());
  if(!window)
  {
    return false;
  }

  DropWindowTarget targetData;
  targetData.target   = target;
  targetData.mimeType = mimeType;
  targetData.callback = callback;
  targetData.inside   = false;
  targetData.window   = window;
  mDropWindowTargets.push_back(targetData);
  return true;
}

bool DragAndDropTcoreWl::RemoveListener(Dali::Actor target)
{
  for(auto itr = mDropTargets.begin(); itr != mDropTargets.end(); ++itr)
  {
    if((*itr).target == target)
    {
      mDropTargets.erase(itr);
      return true;
    }
  }
  return true;
}

bool DragAndDropTcoreWl::RemoveListener(Dali::Window target)
{
  for(auto itr = mDropWindowTargets.begin(); itr != mDropWindowTargets.end(); ++itr)
  {
    if((*itr).target == target)
    {
      mDropWindowTargets.erase(itr);
      return true;
    }
  }
  return true;
}

void DragAndDropTcoreWl::CallSourceEvent(Dali::DragAndDrop::SourceEventType type)
{
  if(mSourceCallback)
  {
    mSourceCallback(type);
    if(type != Dali::DragAndDrop::SourceEventType::START)
    {
      mDragWindow.Reset();
    }
  }
}

void DragAndDropTcoreWl::ResetDropTargets()
{
  for(std::size_t i = 0; i < mDropTargets.size(); i++)
  {
    if(mDropTargets[i].inside)
    {
      Dali::DragAndDrop::DragEvent dragEvent;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
      dragEvent.SetPosition(Dali::Vector2(-1, -1));
      mDropTargets[i].callback(dragEvent);
    }
    mDropTargets[i].inside = false;
  }
  for(std::size_t i = 0; i < mDropWindowTargets.size(); i++)
  {
    if(mDropWindowTargets[i].inside)
    {
      Dali::DragAndDrop::DragEvent dragEvent;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
      dragEvent.SetPosition(Dali::Vector2(-1, -1));
      mDropWindowTargets[i].callback(dragEvent);
    }
    mDropWindowTargets[i].inside = false;
  }
}

void DragAndDropTcoreWl::SendData(void* event)
{
  auto*    ev      = static_cast<tizen_core_wl_event_data_source_send_h>(event);
  char*    typeStr = nullptr;
  int      fd      = -1;
  uint32_t serial  = 0;
  if(tizen_core_wl_event_data_source_send_get_type(ev, &typeStr) != TIZEN_CORE_WL_ERROR_NONE || !typeStr)
  {
    return;
  }
  if(tizen_core_wl_event_data_source_send_get_fd(ev, &fd) != TIZEN_CORE_WL_ERROR_NONE || fd < 0)
  {
    return;
  }
  tizen_core_wl_event_data_source_base_get_serial(static_cast<tizen_core_wl_event_data_source_base_h>(event), &serial);

  if(serial != mSerial)
  {
    close(fd);
    return;
  }

  auto iter = mDataMap.find(typeStr);
  if(iter != mDataMap.end())
  {
    const std::string& dataStr  = iter->second;
    size_t             dataSize = dataStr.size() + 1u;
    ssize_t            ret      = write(fd, dataStr.c_str(), dataSize);
    if(DALI_UNLIKELY(ret != static_cast<ssize_t>(dataSize)))
    {
      DALI_LOG_ERROR("SendData write failed: %zd\n", ret);
    }
  }
  close(fd);
}

void DragAndDropTcoreWl::ReceiveData(void* event)
{
  auto*       ev       = static_cast<tizen_core_wl_event_data_ready_h>(event);
  void*       dataPtr  = nullptr;
  int         len      = 0;
  const char* mimetype = nullptr;
  tizen_core_wl_event_data_ready_get_data(ev, &dataPtr);
  tizen_core_wl_event_data_ready_get_len(ev, &len);
  tizen_core_wl_event_data_ready_get_mimetype(ev, &mimetype);

  if(mTargetIndex != -1)
  {
    const char*                  mimes[] = {mimetype ? mimetype : "", nullptr};
    Dali::DragAndDrop::DragEvent dragEvent(Dali::DragAndDrop::DragType::DROP, mPosition, mimes, 1, static_cast<char*>(dataPtr));
    mDropTargets[mTargetIndex].callback(dragEvent);
    mDropTargets[mTargetIndex].inside = false;
  }
  mTargetIndex = -1;

  tizen_core_wl_window_h eventWindow = nullptr;
  tizen_core_wl_event_data_ready_get_window(ev, &eventWindow);

  if(mWindowTargetIndex != -1)
  {
    const char*                  mimes[] = {mimetype ? mimetype : "", nullptr};
    Dali::DragAndDrop::DragEvent dragEvent(Dali::DragAndDrop::DragType::DROP, mWindowPosition, mimes, 1, static_cast<char*>(dataPtr));
    mDropWindowTargets[mWindowTargetIndex].callback(dragEvent);
    mDropWindowTargets[mWindowTargetIndex].inside = false;
  }
  else if(eventWindow)
  {
    for(std::size_t i = 0; i < mDropWindowTargets.size(); i++)
    {
      if(mDropWindowTargets[i].window == eventWindow)
      {
        const char*                  mimes[] = {mimetype ? mimetype : "", nullptr};
        Dali::DragAndDrop::DragEvent dragEvent(Dali::DragAndDrop::DragType::DROP, mWindowPosition, mimes, 1, static_cast<char*>(dataPtr));
        mDropWindowTargets[i].callback(dragEvent);
        break;
      }
    }
  }
  mWindowTargetIndex = -1;
}

Vector2 DragAndDropTcoreWl::RecalculatePositionByOrientation(int x, int y, Dali::Window window)
{
  int screenWidth, screenHeight;
  Internal::Adaptor::WindowSystem::GetScreenSize(screenWidth, screenHeight);
  Internal::Adaptor::Window& windowImpl   = Dali::GetImplementation(window);
  int                        angle        = windowImpl.GetCurrentWindowRotationAngle();
  auto                       positionSize = window.GetPositionSize();

  int newX, newY;
  if(angle == 90)
  {
    newX = (positionSize.width - 1) - y;
    newY = x;
  }
  else if(angle == 180)
  {
    newX = (positionSize.height - 1) - x;
    newY = (positionSize.width - 1) - y;
  }
  else if(angle == 270)
  {
    newX = y;
    newY = (positionSize.height - 1) - x;
  }
  else
  {
    newX = x;
    newY = y;
  }
  return Dali::Vector2(static_cast<float>(newX), static_cast<float>(newY));
}

void DragAndDropTcoreWl::TriggerDragEventForTarget(int targetIndex, void* event, char** mimes, int mimesCount, Dali::DragAndDrop::DragEvent& dragEvent)
{
  tizen_core_wl_event_data_motion_h ev = static_cast<tizen_core_wl_event_data_motion_h>(event);
  int                               x = 0, y = 0;
  tizen_core_wl_event_data_motion_get_position(ev, &x, &y);

  Vector2       position      = mDropTargets[targetIndex].target.GetProperty<Vector2>(Dali::Actor::Property::SCREEN_POSITION);
  Vector2       size          = mDropTargets[targetIndex].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);
  Dali::Window  window        = Dali::Window::Get(mDropTargets[targetIndex].target);
  Dali::Vector2 cursor        = RecalculatePositionByOrientation(x, y, window);
  bool          currentInside = IsIntersection(static_cast<int>(cursor.x), static_cast<int>(cursor.y), static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(size.width), static_cast<int>(size.height));

  tizen_core_wl_data_offer_h offer = nullptr;
  tizen_core_wl_event_data_base_get_offer(static_cast<tizen_core_wl_event_data_base_h>(event), &offer);

  if(currentInside && !mDropTargets[targetIndex].inside)
  {
    mDropTargets[targetIndex].inside = true;
    dragEvent.SetAction(Dali::DragAndDrop::DragType::ENTER);
    dragEvent.SetPosition(cursor);
    mDropTargets[targetIndex].callback(dragEvent);
    if(offer && mimes)
    {
      tizen_core_wl_data_set_mimes(offer, mimes);
    }
  }
  else if(!currentInside && mDropTargets[targetIndex].inside)
  {
    mDropTargets[targetIndex].inside = false;
    dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
    dragEvent.SetPosition(cursor);
    mDropTargets[targetIndex].callback(dragEvent);
    if(offer)
    {
      tizen_core_wl_data_accept(offer, nullptr);
    }
  }
  else if(currentInside && mDropTargets[targetIndex].inside)
  {
    dragEvent.SetAction(Dali::DragAndDrop::DragType::MOVE);
    dragEvent.SetPosition(cursor);
    mDropTargets[targetIndex].callback(dragEvent);
  }
}

void DragAndDropTcoreWl::ProcessDragEventsForTargets(void* event, Dali::DragAndDrop::DragEvent& dragEvent, char** mimes, int mimesCount)
{
  tizen_core_wl_window_h eventWindow = nullptr;
  tizen_core_wl_event_data_base_get_window(static_cast<tizen_core_wl_event_data_base_h>(event), &eventWindow);

  for(std::size_t targetIndex = 0; targetIndex < mDropTargets.size(); targetIndex++)
  {
    if(mDropTargets[targetIndex].parentWindow != eventWindow)
    {
      continue;
    }
    TriggerDragEventForTarget(static_cast<int>(targetIndex), event, mimes, mimesCount, dragEvent);
  }
}

void DragAndDropTcoreWl::TriggerDragEventForWindowTarget(int targetIndex, void* event, char** mimes, int mimesCount, Dali::DragAndDrop::DragEvent& dragEvent)
{
  tizen_core_wl_event_data_motion_h ev = static_cast<tizen_core_wl_event_data_motion_h>(event);
  int                               x = 0, y = 0;
  tizen_core_wl_event_data_motion_get_position(ev, &x, &y);

  Dali::Window  window        = mDropWindowTargets[targetIndex].target;
  auto          positionSize  = window.GetPositionSize();
  Dali::Vector2 cursor        = RecalculatePositionByOrientation(x, y, window);
  bool          currentInside = IsIntersection(
    static_cast<int>(cursor.x) + positionSize.x, static_cast<int>(cursor.y) + positionSize.y,
    positionSize.x, positionSize.y, positionSize.width, positionSize.height);

  tizen_core_wl_data_offer_h offer = nullptr;
  tizen_core_wl_event_data_base_get_offer(static_cast<tizen_core_wl_event_data_base_h>(event), &offer);

  if(currentInside && !mDropWindowTargets[targetIndex].inside)
  {
    mDropWindowTargets[targetIndex].inside = true;
    dragEvent.SetAction(Dali::DragAndDrop::DragType::ENTER);
    dragEvent.SetPosition(cursor);
    mDropWindowTargets[targetIndex].callback(dragEvent);
    if(offer && mimes)
    {
      tizen_core_wl_data_set_mimes(offer, mimes);
    }
  }
  else if(!currentInside && mDropWindowTargets[targetIndex].inside)
  {
    mDropWindowTargets[targetIndex].inside = false;
    dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
    dragEvent.SetPosition(cursor);
    mDropWindowTargets[targetIndex].callback(dragEvent);
    if(offer)
    {
      tizen_core_wl_data_accept(offer, nullptr);
    }
  }
  else if(currentInside && mDropWindowTargets[targetIndex].inside)
  {
    dragEvent.SetAction(Dali::DragAndDrop::DragType::MOVE);
    dragEvent.SetPosition(cursor);
    mDropWindowTargets[targetIndex].callback(dragEvent);
  }
}

void DragAndDropTcoreWl::ProcessDragEventsForWindowTargets(void* event, Dali::DragAndDrop::DragEvent& dragEvent, char** mimes, int mimesCount)
{
  tizen_core_wl_window_h eventWindow = nullptr;
  tizen_core_wl_event_data_base_get_window(static_cast<tizen_core_wl_event_data_base_h>(event), &eventWindow);

  for(std::size_t targetIndex = 0; targetIndex < mDropWindowTargets.size(); targetIndex++)
  {
    if(mDropWindowTargets[targetIndex].window != eventWindow)
    {
      continue;
    }
    TriggerDragEventForWindowTarget(static_cast<int>(targetIndex), event, mimes, mimesCount, dragEvent);
  }
}

bool DragAndDropTcoreWl::CalculateDragEvent(void* event)
{
  tizen_core_wl_data_offer_h offer = nullptr;
  if(tizen_core_wl_event_data_base_get_offer(static_cast<tizen_core_wl_event_data_base_h>(event), &offer) != TIZEN_CORE_WL_ERROR_NONE || !offer)
  {
    return true;
  }

  char** mimetypes = nullptr;
  int    types_num = 0;
  if(tizen_core_wl_data_get_mimes(offer, &mimetypes, &types_num) != TIZEN_CORE_WL_ERROR_NONE || !mimetypes)
  {
    const char*                  mimes[] = {""};
    Dali::DragAndDrop::DragEvent dragEvent;
    dragEvent.SetMimeTypes(mimes, 0);
    ProcessDragEventsForTargets(event, dragEvent, nullptr, 0);
    ProcessDragEventsForWindowTargets(event, dragEvent, nullptr, 0);
    return true;
  }

  unsigned int mimeCount = static_cast<unsigned int>(types_num);
  if(mimeCount > MAX_MIME_SIZE)
  {
    mimeCount = MAX_MIME_SIZE;
  }
  const char* mimesPool[MAX_MIME_SIZE];
  for(unsigned int i = 0; i < mimeCount; ++i)
  {
    mimesPool[i] = mimetypes[i];
  }
  Dali::DragAndDrop::DragEvent dragEvent;
  dragEvent.SetMimeTypes(mimesPool, mimeCount);

  std::vector<char*> mimePtrs(static_cast<size_t>(types_num) + 1u);
  for(int i = 0; i < types_num; i++)
  {
    mimePtrs[i] = mimetypes[i];
  }
  mimePtrs[types_num] = nullptr;

  ProcessDragEventsForTargets(event, dragEvent, mimePtrs.data(), types_num);
  ProcessDragEventsForWindowTargets(event, dragEvent, mimePtrs.data(), types_num);
  return true;
}

bool DragAndDropTcoreWl::ProcessDropEventsForTargets(void* event, char** mimes, int mimesCount)
{
  auto*                      ev          = static_cast<tizen_core_wl_event_data_drop_h>(event);
  tizen_core_wl_window_h     eventWindow = nullptr;
  int                        x = 0, y = 0;
  tizen_core_wl_data_offer_h offer = nullptr;
  tizen_core_wl_event_data_base_get_window(static_cast<tizen_core_wl_event_data_base_h>(event), &eventWindow);
  tizen_core_wl_event_data_drop_get_position(ev, &x, &y);
  tizen_core_wl_event_data_base_get_offer(static_cast<tizen_core_wl_event_data_base_h>(event), &offer);

  tizen_core_wl_display_h display = nullptr;
  if(!eventWindow || tizen_core_wl_window_get_display(eventWindow, &display) != TIZEN_CORE_WL_ERROR_NONE)
  {
    return false;
  }

  for(std::size_t i = 0; i < mDropTargets.size(); i++)
  {
    if(mDropTargets[i].parentWindow != eventWindow)
    {
      continue;
    }

    Vector2       position = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SCREEN_POSITION);
    Vector2       size     = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);
    Dali::Window  window   = Dali::Window::Get(mDropTargets[i].target);
    Dali::Vector2 cursor   = RecalculatePositionByOrientation(x, y, window);

    if(!IsIntersection(static_cast<int>(cursor.x), static_cast<int>(cursor.y), static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(size.width), static_cast<int>(size.height)))
    {
      continue;
    }

    mTargetIndex = static_cast<int>(i);
    mPosition    = Dali::Vector2(position.x, position.y);

    for(int j = 0; j < mimesCount && mimes && mimes[j]; j++)
    {
      const char* availableType = mimes[j];
      if(!strcmp(mDropTargets[i].mimeType.c_str(), "*/*") || !strcmp(availableType, mDropTargets[i].mimeType.c_str()))
      {
        tizen_core_wl_data_receive(offer, const_cast<char*>(availableType));
        tizen_core_wl_display_flush(display);
        return true;
      }
    }
  }
  return false;
}

bool DragAndDropTcoreWl::ProcessDropEventsForWindowTargets(void* event, char** mimes, int mimesCount)
{
  auto*                      ev          = static_cast<tizen_core_wl_event_data_drop_h>(event);
  tizen_core_wl_window_h     eventWindow = nullptr;
  int                        x = 0, y = 0;
  tizen_core_wl_data_offer_h offer = nullptr;
  tizen_core_wl_event_data_base_get_window(static_cast<tizen_core_wl_event_data_base_h>(event), &eventWindow);
  tizen_core_wl_event_data_drop_get_position(ev, &x, &y);
  tizen_core_wl_event_data_base_get_offer(static_cast<tizen_core_wl_event_data_base_h>(event), &offer);

  tizen_core_wl_display_h display = nullptr;
  if(!eventWindow || tizen_core_wl_window_get_display(eventWindow, &display) != TIZEN_CORE_WL_ERROR_NONE)
  {
    return false;
  }

  for(std::size_t i = 0; i < mDropWindowTargets.size(); i++)
  {
    if(mDropWindowTargets[i].window != eventWindow)
    {
      continue;
    }

    Dali::Window  window       = mDropWindowTargets[i].target;
    auto          positionSize = window.GetPositionSize();
    Dali::Vector2 cursor       = RecalculatePositionByOrientation(x, y, window);
    if(!IsIntersection(
         static_cast<int>(cursor.x) + positionSize.x, static_cast<int>(cursor.y) + positionSize.y,
         positionSize.x, positionSize.y, positionSize.width, positionSize.height))
    {
      continue;
    }

    mWindowTargetIndex = static_cast<int>(i);
    mWindowPosition    = Dali::Vector2(static_cast<float>(position.GetX()), static_cast<float>(position.GetY()));

    for(int j = 0; j < mimesCount && mimes && mimes[j]; j++)
    {
      const char* availableType = mimes[j];
      if(!strcmp(mDropWindowTargets[i].mimeType.c_str(), "*/*") || !strcmp(availableType, mDropWindowTargets[i].mimeType.c_str()))
      {
        tizen_core_wl_data_receive(offer, const_cast<char*>(availableType));
        tizen_core_wl_display_flush(display);
        return true;
      }
    }
  }
  return false;
}

bool DragAndDropTcoreWl::CalculateViewRegion(void* event)
{
  mTargetIndex       = -1;
  mWindowTargetIndex = -1;

  tizen_core_wl_data_offer_h offer = nullptr;
  tizen_core_wl_event_data_base_get_offer(static_cast<tizen_core_wl_event_data_base_h>(event), &offer);
  if(!offer)
  {
    return false;
  }

  char** mimetypes = nullptr;
  int    types_num = 0;
  if(tizen_core_wl_data_get_mimes(offer, &mimetypes, &types_num) != TIZEN_CORE_WL_ERROR_NONE || !mimetypes)
  {
    return false;
  }

  bool ok = ProcessDropEventsForTargets(event, mimetypes, types_num) || ProcessDropEventsForWindowTargets(event, mimetypes, types_num);
  return ok;
}

void DragAndDropTcoreWl::DropTargetSceneOn(Dali::Actor target)
{
  target.SceneConnectedSignal().Disconnect(this, &DragAndDropTcoreWl::DropTargetSceneOn);
  for(auto iter = mDropTargets.begin(), iterEnd = mDropTargets.end(); iter != iterEnd; iter++)
  {
    if((*iter).target == target)
    {
      auto                   window       = Dali::Window::Get(target);
      tizen_core_wl_window_h parentWindow = GetWindowFromNativeHandle(window.GetNativeHandle());
      if(parentWindow)
      {
        (*iter).parentWindow = parentWindow;
      }
      break;
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
