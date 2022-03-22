/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/adaptor/tizen-wayland/dali-ecore-wl2.h>
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// DragAndDrop
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
static bool IsIntersection(int px, int py, int tx, int ty, int tw, int th)
{
  if(px > tx && py > ty && px < (tx + tw) && py < (ty + th))
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
  mSendHandler    = ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND, EcoreEventDataSend, this);
  mReceiveHandler = ecore_event_handler_add(ECORE_WL2_EVENT_OFFER_DATA_READY, EcoreEventOfferDataReady, this);
  mMotionHandler  = ecore_event_handler_add(ECORE_WL2_EVENT_DND_MOTION, EcoreEventDataMotion, this);
  mDropHandler    = ecore_event_handler_add(ECORE_WL2_EVENT_DND_DROP, EcoreEventDataDrop, this);
}

DragAndDropEcoreWl::~DragAndDropEcoreWl()
{
  ecore_event_handler_del(mSendHandler);
  ecore_event_handler_del(mReceiveHandler);
  ecore_event_handler_del(mMotionHandler);
  ecore_event_handler_del(mDropHandler);
}

bool DragAndDropEcoreWl::StartDragAndDrop(Dali::Actor source, Dali::Actor shadow, const Dali::DragAndDrop::DragData& data)
{
  // Get Parent Window
  auto parent = Dali::DevelWindow::Get(source);

  // Set Drag Source Data
  mMimeType = data.GetMimeType();
  mData = data.GetData();

  // Apply Shadow Property
  shadow.SetProperty(Dali::Actor::Property::SIZE, Vector2(150, 150));
  shadow.SetProperty(Dali::Actor::Property::OPACITY, 0.9f);

  // Create Drag Window
  mDragWindow = Dali::Window::New(Dali::PositionSize(0, 0, 150, 150), "DragWindow", "class", true);
  mDragWindow.SetTransparency(true);
  mDragWindow.SetSize(Dali::Window::WindowSize(150, 150));
  mDragWindow.SetBackgroundColor(Color::TRANSPARENT);
  mDragWindow.Add(shadow);

  // Start Drag and Drop
  Ecore_Wl2_Window*  parentWindow = AnyCast<Ecore_Wl2_Window*>(parent.GetNativeHandle());
  Ecore_Wl2_Window*  dragWindow   = AnyCast<Ecore_Wl2_Window*>(mDragWindow.GetNativeHandle());
  Ecore_Wl2_Display* display      = ecore_wl2_connected_display_get(NULL);
  Ecore_Wl2_Input*   input        = ecore_wl2_input_default_input_get(display);

  // Disable Default Cursor
  ecore_wl2_input_pointer_set(input, NULL, 0, 0);

  // Set mime type for drag and drop
  const char* mimeTypes[2];
  mimeTypes[0] = mMimeType.c_str();
  mimeTypes[1] = NULL;


  // Set mime type
  ecore_wl2_dnd_drag_types_set(input, (const char**)mimeTypes);

  // Start wayland drag and drop
  mSerial = ecore_wl2_dnd_drag_start(input, parentWindow, dragWindow);

  return true;
}

bool DragAndDropEcoreWl::AddListener(Dali::Actor target, Dali::DragAndDrop::DragAndDropFunction callback)
{
  std::vector<DropTarget>::iterator itr;
  for(itr = mDropTargets.begin(); itr < mDropTargets.end(); itr++)
  {
    if((*itr).target == target)
    {
      return false;
    }
  }

  DropTarget targetData;
  targetData.target   = target;
  targetData.callback = callback;
  targetData.inside   = false;

  mDropTargets.push_back(targetData);

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

void DragAndDropEcoreWl::SendData(void* event)
{
  Ecore_Wl2_Event_Data_Source_Send* ev = reinterpret_cast<Ecore_Wl2_Event_Data_Source_Send*>(event);
  if(ev->serial != mSerial)
  {
    return;
  }

  int dataLength = strlen(mData.c_str());
  int bufferSize = dataLength;
  if((mMimeType.find("text") != std::string::npos) ||
     (mMimeType.find("markup") != std::string::npos) ||
     (mMimeType.find("image") != std::string::npos))
  {
     bufferSize += 1;
  }

  char* buffer = new char[bufferSize];
  if(!buffer)
  {
    return;
  }

  memcpy(buffer, mData.c_str(), dataLength);
  buffer[dataLength] = '\0';

  write(ev->fd, buffer, bufferSize);
  close(ev->fd);

  if(mDragWindow)
  {
    mDragWindow.Hide();
  }

  delete [] buffer;
}

void DragAndDropEcoreWl::ReceiveData(void* event)
{
  Ecore_Wl2_Event_Offer_Data_Ready* ev = reinterpret_cast<Ecore_Wl2_Event_Offer_Data_Ready*>(event);

  if(mTargetIndex != -1)
  {
    Dali::DragAndDrop::DragEvent dragEvent(Dali::DragAndDrop::DragType::DROP, mPosition, ev->mimetype, ev->data);
    mDropTargets[mTargetIndex].callback(dragEvent);
    mDropTargets[mTargetIndex].inside = false;
  }
  mTargetIndex = -1;
}

bool DragAndDropEcoreWl::CalculateDragEvent(void* event)
{
  Ecore_Wl2_Event_Dnd_Motion* ev = reinterpret_cast<Ecore_Wl2_Event_Dnd_Motion*>(event);

  Dali::DragAndDrop::DragEvent dragEvent;
  Dali::Vector2                curPosition(ev->x, ev->y);

  for(int i = 0; i < mDropTargets.size(); i++)
  {
    Vector2 position      = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::POSITION);
    Vector2 size          = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);
    bool    currentInside = IsIntersection(ev->x, ev->y, position.x, position.y, size.width, size.height);

    // Calculate Drag Enter, Leave, Move Event
    if(currentInside && !mDropTargets[i].inside)
    {
      mDropTargets[i].inside = true;
      // Call Enter Event
      dragEvent.SetAction(Dali::DragAndDrop::DragType::ENTER);
      dragEvent.SetPosition(curPosition);
      mDropTargets[i].callback(dragEvent);
    }
    else if(!currentInside && mDropTargets[i].inside)
    {
      mDropTargets[i].inside = false;
      // Call Leave Event
      dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
      dragEvent.SetPosition(curPosition);
      mDropTargets[i].callback(dragEvent);
    }
    else if(currentInside && mDropTargets[i].inside)
    {
      // Call Move Event
      dragEvent.SetAction(Dali::DragAndDrop::DragType::MOVE);
      dragEvent.SetPosition(curPosition);
      mDropTargets[i].callback(dragEvent);
    }
  }

  return true;
}

bool DragAndDropEcoreWl::CalculateViewRegion(void* event)
{
  Ecore_Wl2_Event_Dnd_Drop* ev = reinterpret_cast<Ecore_Wl2_Event_Dnd_Drop*>(event);

  // Check the target object region
  mTargetIndex = -1;

  for(int i = 0; i < mDropTargets.size(); i++)
  {
    Vector2 position = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::POSITION);
    Vector2 size     = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);
    // If the drop position is in the target object region, request drop data to the source object
    if(IsIntersection(ev->x, ev->y, position.x, position.y, size.width, size.height))
    {
      mTargetIndex        = i;
      mPosition           = position;
      Dali::Window window = Dali::DevelWindow::Get(mDropTargets[i].target);

      char* mimetype = (char*)eina_array_data_get(ecore_wl2_offer_mimes_get(ev->offer), 0);
      if(mimetype)
      {
        ecore_wl2_offer_accept(ev->offer, mimetype);
        ecore_wl2_offer_receive(ev->offer, mimetype);
        Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
        Ecore_Wl2_Input*   input   = ecore_wl2_input_default_input_get(display);
        ecore_wl2_display_flush(ecore_wl2_input_display_get(input));
      }
      return true;
    }
  }

  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
