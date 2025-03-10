/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/drag-and-drop/tizen-wayland/drag-and-drop-impl-ecore-wl2.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <unistd.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-system.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// DragAndDrop
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
// Note : The real Ecore_Window type is uintptr_t.
// But a lots of Ecore_Wl2_Event_Dnd_XXX type use window as 'unsigned int' type.
// And ecore_wl2_window_id_get() API return it as 'int' type.
// So dali just keep window id as int type now.
//
// To avoid compile warning, we should convert the type by it.
using EcoreWl2EventDragAndDropWindowIdType = unsigned int;

namespace
{
static constexpr int32_t DEFAULT_POSITION            = -1;
static constexpr int32_t INVALID_ECORE_WL2_WINDOW_ID = -1;
static constexpr int32_t MAX_MIME_SIZE = 10;
} // namespace

static const char* mimesPool [MAX_MIME_SIZE];

static bool IsIntersection(int px, int py, int tx, int ty, int tw, int th)
{
  if(px >= tx && py >= ty && px <= (tx + tw) && py <= (ty + th))
  {
    return true;
  }
  return false;
}

static Eina_Bool EcoreEventDataSend(void* data, int type, void* event)
{
  DragAndDropEcoreWl* dndImpl = reinterpret_cast<DragAndDropEcoreWl*>(data);
  dndImpl->SendData(event);

  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreEventDataSourceEnd(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Data_Source_End* event_      = reinterpret_cast<Ecore_Wl2_Event_Data_Source_End*>(event);
  DragAndDropEcoreWl*              dndImpl = reinterpret_cast<DragAndDropEcoreWl*>(data);
  if(event_->cancelled)
  {
    dndImpl->CallSourceEvent(Dali::DragAndDrop::SourceEventType::CANCEL);
  }
  else
  {
    dndImpl->CallSourceEvent(Dali::DragAndDrop::SourceEventType::ACCEPT);
  }

  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreEventDataSourceDrop(void* data, int type, void* event)
{
  DragAndDropEcoreWl* dndImpl = reinterpret_cast<DragAndDropEcoreWl*>(data);
  dndImpl->CallSourceEvent(Dali::DragAndDrop::SourceEventType::FINISH);
  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreEventOfferDataReady(void* data, int type, void* event)
{
  DragAndDropEcoreWl* dndImpl = reinterpret_cast<DragAndDropEcoreWl*>(data);
  dndImpl->ReceiveData(event);

  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreEventDataMotion(void* data, int type, void* event)
{
  DragAndDropEcoreWl* dndImpl = reinterpret_cast<DragAndDropEcoreWl*>(data);
  dndImpl->CalculateDragEvent(event);

  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreEventDataDrop(void* data, int type, void* event)
{
  DragAndDropEcoreWl* dndImpl = reinterpret_cast<DragAndDropEcoreWl*>(data);
  dndImpl->CalculateViewRegion(event);

  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreEventDataEnter(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Dnd_Enter* event_ = reinterpret_cast<Ecore_Wl2_Event_Dnd_Enter*>(event);

  // Set default offer is reject
  ecore_wl2_offer_accept(event_->offer, nullptr);
  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreEventDataLeave(void* data, int type, void* event)
{
  DragAndDropEcoreWl* dndImpl = reinterpret_cast<DragAndDropEcoreWl*>(data);
  dndImpl->ResetDropTargets();

  return ECORE_CALLBACK_PASS_ON;
}

Dali::DragAndDrop GetDragAndDrop()
{
  Dali::DragAndDrop dnd;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::DragAndDrop));
    if(handle)
    {
      // If so, downcast the handle
      dnd = Dali::DragAndDrop(dynamic_cast<DragAndDrop*>(handle.GetObjectPtr()));
    }
    else
    {
      // Create a singleon instance
      DragAndDropEcoreWl* dndImpl = new DragAndDropEcoreWl();

      dnd = Dali::DragAndDrop(dndImpl);
      service.Register(typeid(Dali::DragAndDrop), dnd);
    }
  }

  return dnd;
}

DragAndDropEcoreWl::DragAndDropEcoreWl()
{
  // Source Events
  mSendHandler       = ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND, EcoreEventDataSend, this);
  mSourceEndHandler  = ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_END, EcoreEventDataSourceEnd, this);
  mSourceDropHandler = ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_DROP, EcoreEventDataSourceDrop, this);

  // Target Events
  mReceiveHandler = ecore_event_handler_add(ECORE_WL2_EVENT_OFFER_DATA_READY, EcoreEventOfferDataReady, this);
  mMotionHandler  = ecore_event_handler_add(ECORE_WL2_EVENT_DND_MOTION, EcoreEventDataMotion, this);
  mDropHandler    = ecore_event_handler_add(ECORE_WL2_EVENT_DND_DROP, EcoreEventDataDrop, this);
  mEnterHandler   = ecore_event_handler_add(ECORE_WL2_EVENT_DND_ENTER, EcoreEventDataEnter, this);
  mLeaveHandler   = ecore_event_handler_add(ECORE_WL2_EVENT_DND_LEAVE, EcoreEventDataLeave, this);
}

DragAndDropEcoreWl::~DragAndDropEcoreWl()
{
  // Source Events
  ecore_event_handler_del(mSendHandler);
  ecore_event_handler_del(mSourceEndHandler);
  ecore_event_handler_del(mSourceDropHandler);

  // Target Events
  ecore_event_handler_del(mReceiveHandler);
  ecore_event_handler_del(mMotionHandler);
  ecore_event_handler_del(mDropHandler);
  ecore_event_handler_del(mEnterHandler);
}

bool DragAndDropEcoreWl::StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& data, Dali::DragAndDrop::SourceFunction callback)
{
  // Get Parent Window
  auto parent = Dali::DevelWindow::Get(source);

  const char** dataSet = data.GetDataSet();
  const char** mimeTypes = data.GetMimeTypes();

  mDataMap.clear();
  for(int i=0; i<data.GetDataSetSize(); ++i)
  {
    mDataMap[std::string(mimeTypes[i])] = std::string(dataSet[i]);
  }

  // Set Source Event
  mSourceCallback = callback;

  // Set Drag Window
  mDragWindow = shadowWindow;

  // Start Drag and Drop
  Ecore_Wl2_Window*  parentWindow = AnyCast<Ecore_Wl2_Window*>(parent.GetNativeHandle());
  Ecore_Wl2_Window*  dragWindow   = AnyCast<Ecore_Wl2_Window*>(mDragWindow.GetNativeHandle());
  Ecore_Wl2_Display* display      = ecore_wl2_connected_display_get(nullptr);
  Ecore_Wl2_Input*   input        = ecore_wl2_input_default_input_get(display);

  int mimeTypesCount = data.GetMimeTypesSize();
  const char* waylandMimeTypes[mimeTypesCount + 1];
  for (int i = 0; i < mimeTypesCount; ++i)
  {
    waylandMimeTypes[i] = mimeTypes[i];
  }
  waylandMimeTypes[mimeTypesCount] = nullptr;

  // Set mime types
  ecore_wl2_dnd_drag_types_set(input, (const char**)waylandMimeTypes);

  // Start wayland drag and drop
  mSerial = ecore_wl2_dnd_drag_start(input, parentWindow, dragWindow);

  // Call Start Event
  CallSourceEvent(Dali::DragAndDrop::SourceEventType::START);

  return true;
}

bool DragAndDropEcoreWl::AddListener(Dali::Actor target, char* mimeType, Dali::DragAndDrop::DragAndDropFunction callback)
{
  std::vector<DropTarget>::iterator itr;
  for(itr = mDropTargets.begin(); itr < mDropTargets.end(); itr++)
  {
    if((*itr).target == target)
    {
      return false;
    }
  }

  auto window = Dali::DevelWindow::Get(target);

  int parentWindowId = INVALID_ECORE_WL2_WINDOW_ID;

  if(!window)
  {
    // Target is stil not scene-on
    // Add dummy target data, and wait until target is on scene.
    target.OnSceneSignal().Connect(this, &DragAndDropEcoreWl::DropTargetSceneOn);
  }
  else
  {
    Ecore_Wl2_Window* parentWindow = AnyCast<Ecore_Wl2_Window*>(window.GetNativeHandle());
    if(parentWindow == nullptr)
    {
      return false;
    }
    parentWindowId = ecore_wl2_window_id_get(parentWindow);
  }

  DropTarget targetData;
  targetData.target         = target;
  targetData.mimeType       = mimeType;
  targetData.callback       = callback;
  targetData.inside         = false;
  targetData.parentWindowId = parentWindowId;

  mDropTargets.push_back(targetData);

  return true;
}

bool DragAndDropEcoreWl::AddListener(Dali::Window target, char* mimeType, Dali::DragAndDrop::DragAndDropFunction callback)
{
  std::vector<DropWindowTarget>::iterator itr;
  for(itr = mDropWindowTargets.begin(); itr < mDropWindowTargets.end(); itr++)
  {
    if((*itr).target == target)
    {
      return false;
    }
  }

  int windowId = INVALID_ECORE_WL2_WINDOW_ID;

  Ecore_Wl2_Window* window = AnyCast<Ecore_Wl2_Window*>(target.GetNativeHandle());
  if(window == nullptr)
  {
    return false;
  }
  windowId = ecore_wl2_window_id_get(window);

  DropWindowTarget targetData;
  targetData.target   = target;
  targetData.mimeType = mimeType;
  targetData.callback = callback;
  targetData.inside   = false;
  targetData.windowId = windowId;

  mDropWindowTargets.push_back(targetData);

  return true;
}

bool DragAndDropEcoreWl::RemoveListener(Dali::Actor target)
{
  std::vector<DropTarget>::iterator itr;
  for(itr = mDropTargets.begin(); itr < mDropTargets.end(); itr++)
  {
    if((*itr).target == target)
    {
      mDropTargets.erase(itr);
      break;
    }
  }

  return true;
}

bool DragAndDropEcoreWl::RemoveListener(Dali::Window target)
{
  std::vector<DropWindowTarget>::iterator itr;
  for(itr = mDropWindowTargets.begin(); itr < mDropWindowTargets.end(); itr++)
  {
    if((*itr).target == target)
    {
      mDropWindowTargets.erase(itr);
      break;
    }
  }

  return true;
}

void DragAndDropEcoreWl::CallSourceEvent(Dali::DragAndDrop::SourceEventType type)
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

void DragAndDropEcoreWl::ResetDropTargets()
{
  for(std::size_t i = 0; i < mDropTargets.size(); i++)
  {
    if(mDropTargets[i].inside)
    {
      Dali::DragAndDrop::DragEvent dragEvent;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
      Dali::Vector2 position(DEFAULT_POSITION, DEFAULT_POSITION);
      dragEvent.SetPosition(position);
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
      Dali::Vector2 position(DEFAULT_POSITION, DEFAULT_POSITION);
      dragEvent.SetPosition(position);
      mDropWindowTargets[i].callback(dragEvent);
    }
    mDropWindowTargets[i].inside = false;
  }
}

static Eina_Bool WriteDelayedDataTofd(void* data, Ecore_Fd_Handler* fd_handler)
{
  int                 fd;
  size_t              len;
  DelayedWritingData* slice = (DelayedWritingData*)data;

  fd = ecore_main_fd_handler_fd_get(fd_handler);
  if(fd < 0)
  {
    ecore_main_fd_handler_del(fd_handler);
    free(slice->slice.mem);
    free(slice);
    return EINA_FALSE;
  }

  len = write(fd, (char*)slice->slice.mem + slice->writtenBytes, slice->slice.len - slice->writtenBytes);

  slice->writtenBytes += len;
  if(slice->writtenBytes != slice->slice.len)
  {
    return EINA_TRUE;
  }
  else
  {
    ecore_main_fd_handler_del(fd_handler);
    free(slice->slice.mem);
    free(slice);
    if(fd > -1) close(fd);
    return EINA_FALSE;
  }
}

void DragAndDropEcoreWl::SendData(void* event)
{
  Ecore_Wl2_Event_Data_Source_Send* event_ = reinterpret_cast<Ecore_Wl2_Event_Data_Source_Send*>(event);
  if(event_->serial != mSerial)
  {
    return;
  }

  DelayedWritingData* data = (DelayedWritingData*)calloc(1, sizeof(DelayedWritingData));
  if(DALI_UNLIKELY(!data))
  {
    DALI_LOG_ERROR("Failed to allocate DelayedWritingData. calloc size : %zu\n", sizeof(DelayedWritingData));
    return;
  }

  int dataLength = strlen(mDataMap[std::string(event_->type)].c_str());
  int bufferSize = dataLength + 1;

  data->slice.mem = new char[bufferSize];
  data->slice.len = bufferSize;
  memcpy(data->slice.mem, mDataMap[std::string(event_->type)].c_str(), dataLength);
  ((char*)data->slice.mem)[dataLength] = '\0';

  ecore_main_fd_handler_add(event_->fd, ECORE_FD_WRITE, WriteDelayedDataTofd, data, nullptr, nullptr);
}

void DragAndDropEcoreWl::ReceiveData(void* event)
{
  Ecore_Wl2_Event_Offer_Data_Ready* event_ = reinterpret_cast<Ecore_Wl2_Event_Offer_Data_Ready*>(event);

  if(mTargetIndex != -1)
  {
    mimesPool[0] = event_->mimetype;
    mimesPool[1] = nullptr;
    Dali::DragAndDrop::DragEvent dragEvent(Dali::DragAndDrop::DragType::DROP, mPosition, mimesPool, 1, event_->data);
    mDropTargets[mTargetIndex].callback(dragEvent);
    mDropTargets[mTargetIndex].inside = false;
  }
  mTargetIndex = -1;

#ifdef OVER_TIZEN_VERSION_8
  Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(nullptr);
  Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);
#endif

  if(mWindowTargetIndex != -1)
  {
    mimesPool[0] = event_->mimetype;
    mimesPool[1] = nullptr;
    Dali::DragAndDrop::DragEvent dragEvent(Dali::DragAndDrop::DragType::DROP, mWindowPosition, mimesPool, 1, event_->data);
    mDropWindowTargets[mWindowTargetIndex].callback(dragEvent);
    mDropWindowTargets[mWindowTargetIndex].inside = false;
  }
#ifdef OVER_TIZEN_VERSION_8
  else if(event_->offer != ecore_wl2_dnd_selection_get(input))
  {
    for(std::size_t i = 0; i < mDropWindowTargets.size(); i++)
    {
      if(event_->win == static_cast<EcoreWl2EventDragAndDropWindowIdType>(mDropWindowTargets[i].windowId))
      {
        mimesPool[0] = event_->mimetype;
        mimesPool[1] = nullptr;
        Dali::DragAndDrop::DragEvent dragEvent(Dali::DragAndDrop::DragType::DROP, mWindowPosition, mimesPool, 1, event_->data);
        mDropWindowTargets[i].callback(dragEvent);
        break;
      }
    }
  }
#endif

  mWindowTargetIndex = -1;
}

Vector2 DragAndDropEcoreWl::RecalculatePositionByOrientation(int x, int y, Dali::Window window)
{
  int screenWidth, screenHeight;
  Internal::Adaptor::WindowSystem::GetScreenSize(screenWidth, screenHeight);
  int angle = static_cast<int>(DevelWindow::GetCurrentOrientation(window));
  Dali::Window::WindowSize size = window.GetSize();

  int           newX, newY;
  Dali::Vector2 newPosition;

  if(angle == 90)
  {
    newX = (size.GetWidth() - 1) - y;
    newY = x;
  }
  else if(angle == 180)
  {
    newX = (size.GetHeight() - 1) - x;
    newY = (size.GetWidth() - 1) - y;
  }
  else if(angle == 270)
  {
    newX = y;
    newY = (size.GetHeight() - 1) - x;
  }
  else
  {
    newX = x;
    newY = y;
  }

  newPosition.x = newX;
  newPosition.y = newY;

  return newPosition;
}

void DragAndDropEcoreWl::TriggerDragEventForTarget(int targetIndex, Ecore_Wl2_Event_Dnd_Motion* event, Eina_Array* mimes,  Dali::DragAndDrop::DragEvent& dragEvent)
{
  Vector2 position = mDropTargets[targetIndex].target.GetProperty<Vector2>(Dali::Actor::Property::SCREEN_POSITION);
  Vector2 size     = mDropTargets[targetIndex].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);

  // Recalculate Cursor by Orientation
  Dali::Window  window = Dali::DevelWindow::Get(mDropTargets[targetIndex].target);
  Dali::Vector2 cursor = RecalculatePositionByOrientation(event->x, event->y, window);
  bool currentInside = IsIntersection(cursor.x, cursor.y, position.x, position.y, size.width, size.height);

  // Calculate Drag Enter, Leave, Move Event
  if(currentInside && !mDropTargets[targetIndex].inside)
  {
    mDropTargets[targetIndex].inside = true;
    // Call Enter Event
    dragEvent.SetAction(Dali::DragAndDrop::DragType::ENTER);
    dragEvent.SetPosition(cursor);
    mDropTargets[targetIndex].callback(dragEvent);
    // Accept Offer
    ecore_wl2_offer_mimes_set(event->offer, mimes);
  }
  else if(!currentInside && mDropTargets[targetIndex].inside)
  {
    mDropTargets[targetIndex].inside = false;
    // Call Leave Event
    dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
    dragEvent.SetPosition(cursor);
    mDropTargets[targetIndex].callback(dragEvent);
    // Reject Offer
    ecore_wl2_offer_accept(event->offer, nullptr);
  }
  else if(currentInside && mDropTargets[targetIndex].inside)
  {
    // Call Move Event
    dragEvent.SetAction(Dali::DragAndDrop::DragType::MOVE);
    dragEvent.SetPosition(cursor);
    mDropTargets[targetIndex].callback(dragEvent);
  }
}

void DragAndDropEcoreWl::ProcessDragEventsForTargets(Ecore_Wl2_Event_Dnd_Motion* event, Dali::DragAndDrop::DragEvent& dragEvent, Eina_Array* mimes)
{
  for(std::size_t targetIndex = 0; targetIndex < mDropTargets.size(); targetIndex++)
  {
    if(event->win != static_cast<EcoreWl2EventDragAndDropWindowIdType>(mDropTargets[targetIndex].parentWindowId))
    {
      continue;
    }

    TriggerDragEventForTarget(targetIndex, event, mimes, dragEvent);
  }
}

void DragAndDropEcoreWl::TriggerDragEventForWindowTarget(int targetIndex, Ecore_Wl2_Event_Dnd_Motion* event, Eina_Array* mimes, Dali::DragAndDrop::DragEvent& dragEvent)
{
  // Recalculate Cursor by Orientation
  Dali::Window                 window   = mDropWindowTargets[targetIndex].target;
  Dali::Window::WindowPosition position = window.GetPosition();
  Dali::Window::WindowSize     size     = window.GetSize();

  Dali::Vector2 cursor = RecalculatePositionByOrientation(event->x, event->y, window);
  bool currentInside = IsIntersection(cursor.x + position.GetX(), cursor.y + position.GetY(), position.GetX(), position.GetY(), size.GetWidth(), size.GetHeight());

  // Calculate Drag Enter, Leave, Move Event
  if(currentInside && !mDropWindowTargets[targetIndex].inside)
  {
    mDropWindowTargets[targetIndex].inside = true;
    // Call Enter Event
    dragEvent.SetAction(Dali::DragAndDrop::DragType::ENTER);
    dragEvent.SetPosition(cursor);
    mDropWindowTargets[targetIndex].callback(dragEvent);
    // Accept Offer
    ecore_wl2_offer_mimes_set(event->offer, mimes);
  }
  else if(!currentInside && mDropWindowTargets[targetIndex].inside)
  {
    mDropWindowTargets[targetIndex].inside = false;
    // Call Leave Event
    dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
    dragEvent.SetPosition(cursor);
    mDropWindowTargets[targetIndex].callback(dragEvent);
    // Reject Offer
    ecore_wl2_offer_accept(event->offer, nullptr);
  }
  else if(currentInside && mDropWindowTargets[targetIndex].inside)
  {
    // Call Move Event
    dragEvent.SetAction(Dali::DragAndDrop::DragType::MOVE);
    dragEvent.SetPosition(cursor);
    mDropWindowTargets[targetIndex].callback(dragEvent);
  }
}

void DragAndDropEcoreWl::ProcessDragEventsForWindowTargets(Ecore_Wl2_Event_Dnd_Motion* event, Dali::DragAndDrop::DragEvent& dragEvent, Eina_Array* mimes)
{
  for(std::size_t targetIndex = 0; targetIndex < mDropWindowTargets.size(); targetIndex++)
  {
    if(event->win != static_cast<EcoreWl2EventDragAndDropWindowIdType>(mDropWindowTargets[targetIndex].windowId))
    {
      continue;
    }

    TriggerDragEventForWindowTarget(targetIndex, event, mimes, dragEvent);
  }
}

bool DragAndDropEcoreWl::CalculateDragEvent(void* event)
{
  Ecore_Wl2_Event_Dnd_Motion* event_ = reinterpret_cast<Ecore_Wl2_Event_Dnd_Motion*>(event);

  Dali::DragAndDrop::DragEvent dragEvent;

  Eina_Array* mimes = ecore_wl2_offer_mimes_get(event_->offer);
  if(mimes == nullptr)
  {
    mimesPool[0] = "";
    dragEvent.SetMimeTypes(mimesPool, 0);
  }
  else
  {
    unsigned int mimeCount = (unsigned int)eina_array_count((Eina_Array*)mimes);
    if (mimeCount > MAX_MIME_SIZE)
    {
      mimeCount = MAX_MIME_SIZE;
    }
    for (unsigned int i = 0; i < mimeCount; ++i)
    {
      mimesPool[i] = (char*)eina_array_data_get((Eina_Array*)mimes, i);
    }
    dragEvent.SetMimeTypes(mimesPool, mimeCount);
  }

  ProcessDragEventsForTargets(event_, dragEvent, mimes);
  ProcessDragEventsForWindowTargets(event_, dragEvent, mimes);

  return true;
}

bool DragAndDropEcoreWl::ProcessDropEventsForTargets(Ecore_Wl2_Event_Dnd_Drop* event, Eina_Array* mimes)
{
  for(std::size_t i = 0; i < mDropTargets.size(); i++)
  {
    if(event->win != static_cast<EcoreWl2EventDragAndDropWindowIdType>(mDropTargets[i].parentWindowId))
    {
      continue;
    }

    Vector2 position = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SCREEN_POSITION);
    Vector2 size     = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);

    // Recalculate Cursor by Orientation
    Dali::Window  window = Dali::DevelWindow::Get(mDropTargets[i].target);
    Dali::Vector2 cursor = RecalculatePositionByOrientation(event->x, event->y, window);

    // If the drop position is in the target object region, request drop data to the source object
    if(IsIntersection(cursor.x, cursor.y, position.x, position.y, size.width, size.height))
    {
      mTargetIndex        = i;
      mPosition           = position;
      Dali::Window window = Dali::DevelWindow::Get(mDropTargets[i].target);

      unsigned int mimeCount = (unsigned int)eina_array_count((Eina_Array*)mimes);
      for (unsigned int j = 0; j < mimeCount; ++j)
      {
        char* availableType = (char*)eina_array_data_get((Eina_Array*)mimes, j);
        if (!availableType)
        {
          return false;
        }

        if (!strcmp(mDropTargets[i].mimeType.c_str(),"*/*")
            || !strcmp(availableType, mDropTargets[i].mimeType.c_str()))
        {
          ecore_wl2_offer_receive(event->offer, availableType);
          Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(nullptr);
          Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);
          ecore_wl2_display_flush(ecore_wl2_input_display_get(input));
          return true;
        }
      }
    }
  }
  return false;
}

bool DragAndDropEcoreWl::ProcessDropEventsForWindowTargets(Ecore_Wl2_Event_Dnd_Drop* event, Eina_Array* mimes)
{
  for(std::size_t i = 0; i < mDropWindowTargets.size(); i++)
  {
    if(event->win != static_cast<EcoreWl2EventDragAndDropWindowIdType>(mDropWindowTargets[i].windowId))
    {
      continue;
    }

    // Recalculate Cursor by Orientation
    Dali::Window                 window   = mDropWindowTargets[i].target;
    Dali::Window::WindowPosition position = window.GetPosition();
    Dali::Window::WindowSize     size     = window.GetSize();

    Dali::Vector2 cursor = RecalculatePositionByOrientation(event->x, event->y, window);
    // If the drop position is in the target object region, request drop data to the source object
    if(IsIntersection(cursor.x + position.GetX(), cursor.y + position.GetY(), position.GetX(), position.GetY(), size.GetWidth(), size.GetHeight()))
    {
      mWindowTargetIndex = i;
      mWindowPosition    = Dali::Vector2(position.GetX(), position.GetY());

      unsigned int mimeCount = (unsigned int)eina_array_count((Eina_Array*)mimes);
      for (unsigned int j = 0; j < mimeCount; ++j)
      {
        char* availableType = (char*)eina_array_data_get((Eina_Array*)mimes, j);
        if (!availableType)
        {
          return false;
        }

        if (!strcmp(mDropTargets[i].mimeType.c_str(),"*/*")
            || !strcmp(availableType, mDropTargets[i].mimeType.c_str()))
        {
          ecore_wl2_offer_receive(event->offer, availableType);
          Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(nullptr);
          Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);
          ecore_wl2_display_flush(ecore_wl2_input_display_get(input));
          return true;
        }
      }
    }
  }
  return false;
}

bool DragAndDropEcoreWl::CalculateViewRegion(void* event)
{
  Ecore_Wl2_Event_Dnd_Drop* event_ = reinterpret_cast<Ecore_Wl2_Event_Dnd_Drop*>(event);

  // Check the target object region
  mTargetIndex       = -1;
  mWindowTargetIndex = -1;

  Eina_Array* mimes = ecore_wl2_offer_mimes_get(event_->offer);
  if(mimes == nullptr)
  {
    return false;
  }

  if(ProcessDropEventsForTargets(event_, mimes) || ProcessDropEventsForWindowTargets(event_, mimes))
  {
    return true;
  }

  return false;
}

void DragAndDropEcoreWl::DropTargetSceneOn(Dali::Actor target)
{
  // Disconnect scene on signal
  target.OnSceneSignal().Disconnect(this, &DragAndDropEcoreWl::DropTargetSceneOn);

  for(auto iter = mDropTargets.begin(), iterEnd = mDropTargets.end(); iter != iterEnd; iter++)
  {
    if((*iter).target == target)
    {
      auto window = Dali::DevelWindow::Get(target);

      Ecore_Wl2_Window* parentWindow = AnyCast<Ecore_Wl2_Window*>(window.GetNativeHandle());
      if(parentWindow == nullptr)
      {
        return;
      }

      (*iter).parentWindowId = ecore_wl2_window_id_get(parentWindow);
      break;
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
