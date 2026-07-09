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
#include <dali/internal/drag-and-drop/ubuntu-x11/drag-and-drop-impl-x11.h>

// EXTERNAL INCLUDES
#include <X11/Xlib.h>
#include <cstring>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/drag-and-drop/common/drag-and-drop-factory.h>
#include <dali/internal/window-system/common/window-impl.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// DragAndDropX11
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace
{
static constexpr int32_t DEFAULT_POSITION    = -1;
static constexpr int32_t INVALID_X_WINDOW_ID = 0;

// File-level pool for delivering DragEvent MIME array to callbacks (mirrors ecore-wl2 pattern).
// Capacity = DragAndDropX11::MAX_MIME_SIZE(10) + 1 null-terminator slot.
static constexpr int MIME_POOL_CAPACITY = 11;
static const char*   mimesPool[MIME_POOL_CAPACITY];

static bool IsIntersection(int px, int py, int tx, int ty, int tw, int th)
{
  return (px >= tx && py >= ty && px <= (tx + tw) && py <= (ty + th));
}

// Actor::Property::SCREEN_POSITION is window-client-area-relative.
// DragPosUpdateCb gives root (screen-absolute) cursor coordinates.
// This helper gets the window client area's position in root coordinates via XTranslateCoordinates.
static void GetWindowRootOffset(Ecore_X_Window win, int& outX, int& outY)
{
  outX = outY = 0;
  if(!win)
  {
    return;
  }
  Display* dpy   = static_cast<Display*>(ecore_x_display_get());
  ::Window root  = static_cast<::Window>(ecore_x_window_root_get(win));
  ::Window child = 0;
  XTranslateCoordinates(dpy, static_cast<::Window>(win), root, 0, 0, &outX, &outY, &child);
}
} // namespace

// ---------------------------------------------------------------------------
// Static ecore event callbacks – forward to instance methods
// ---------------------------------------------------------------------------

static Eina_Bool EcoreXEventXdndEnter(void* data, int /*type*/, void* event)
{
  static_cast<DragAndDropX11*>(data)->HandleXdndEnter(event);
  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreXEventXdndPosition(void* data, int /*type*/, void* event)
{
  static_cast<DragAndDropX11*>(data)->CalculateDragEvent(event);
  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreXEventXdndDrop(void* data, int /*type*/, void* event)
{
  static_cast<DragAndDropX11*>(data)->CalculateViewRegion(event);
  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreXEventXdndLeave(void* data, int /*type*/, void* event)
{
  static_cast<DragAndDropX11*>(data)->HandleXdndLeave(event);
  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreXEventXdndFinished(void* data, int /*type*/, void* event)
{
  static_cast<DragAndDropX11*>(data)->HandleXdndFinished(event);
  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreXEventSelectionRequest(void* data, int /*type*/, void* event)
{
  static_cast<DragAndDropX11*>(data)->SendData(event);
  return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool EcoreXEventSelectionNotify(void* data, int /*type*/, void* event)
{
  static_cast<DragAndDropX11*>(data)->ReceiveData(event);
  return ECORE_CALLBACK_PASS_ON;
}

// Called by ecore_x when drag cursor position changes (used to move shadow window)
static void DragPosUpdateCb(void* data, Ecore_X_Xdnd_Position* pos)
{
  static_cast<DragAndDropX11*>(data)->UpdateDragWindowPosition(pos->position.x, pos->position.y);
}

// ecore_x fires this when mouse button released (works during DnD pointer grab)
static Eina_Bool EcoreEventMouseButtonUp(void* data, int /*type*/, void* event)
{
  static_cast<DragAndDropX11*>(data)->HandleMouseButtonUp(event);
  return ECORE_CALLBACK_PASS_ON;
}

// ---------------------------------------------------------------------------
// Singleton / Factory
// ---------------------------------------------------------------------------

Dali::DragAndDrop GetDragAndDrop()
{
  Dali::DragAndDrop dnd;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::DragAndDrop));
    if(handle)
    {
      dnd = Dali::DragAndDrop(dynamic_cast<DragAndDrop*>(handle.GetObjectPtr()));
    }
    else
    {
      DragAndDropX11* impl = new DragAndDropX11();
      dnd                  = Dali::DragAndDrop(impl);
      service.Register(typeid(Dali::DragAndDrop), dnd);
    }
  }
  return dnd;
}

class DragAndDropFactoryX11 : public DragAndDropFactory
{
public:
  Dali::DragAndDrop CreateDragAndDrop() override
  {
    return GetDragAndDrop();
  }
};

std::unique_ptr<DragAndDropFactory> GetDragAndDropFactory()
{
  return std::unique_ptr<DragAndDropFactory>(new DragAndDropFactoryX11());
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

DragAndDropX11::DragAndDropX11()
{
  // Target-side events
  mEnterHandler  = ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER, EcoreXEventXdndEnter, this);
  mMotionHandler = ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION, EcoreXEventXdndPosition, this);
  mDropHandler   = ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP, EcoreXEventXdndDrop, this);
  mLeaveHandler  = ecore_event_handler_add(ECORE_X_EVENT_XDND_LEAVE, EcoreXEventXdndLeave, this);

  // Source-side events
  mFinishedHandler = ecore_event_handler_add(ECORE_X_EVENT_XDND_FINISHED, EcoreXEventXdndFinished, this);
  mSendHandler     = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_REQUEST, EcoreXEventSelectionRequest, this);

  // Data delivery to target
  mReceiveHandler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, EcoreXEventSelectionNotify, this);

  // Mouse button release: used for same-process drop (ecore_x doesn't deliver XDND
  // target events when source and target are in the same process, pos->win stays 0)
  mMouseUpHandler = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, EcoreEventMouseButtonUp, this);
}

DragAndDropX11::~DragAndDropX11()
{
  ecore_event_handler_del(mEnterHandler);
  ecore_event_handler_del(mMotionHandler);
  ecore_event_handler_del(mDropHandler);
  ecore_event_handler_del(mLeaveHandler);
  ecore_event_handler_del(mFinishedHandler);
  ecore_event_handler_del(mSendHandler);
  ecore_event_handler_del(mReceiveHandler);
  ecore_event_handler_del(mMouseUpHandler);
}

// ---------------------------------------------------------------------------
// StartDragAndDrop
// ---------------------------------------------------------------------------

bool DragAndDropX11::StartDragAndDrop(Dali::Actor                        source,
                                      Dali::Window                       shadowWindow,
                                      const Dali::DragAndDrop::DragData& data,
                                      Dali::DragAndDrop::SourceFunction  callback)
{
  auto parent = Dali::Window::Get(source);
  if(!parent)
  {
    return false;
  }

  mSourceWindowId     = AnyCast<Ecore_X_Window>(parent.GetNativeHandle());
  mPrevSourceWindowId = INVALID_X_WINDOW_ID; // clear so cross-process DnD isn't filtered
  if(mSourceWindowId == INVALID_X_WINDOW_ID)
  {
    return false;
  }

  // Store mime→data mapping for SELECTION_REQUEST responses
  const char** mimeTypes = data.GetMimeTypes();
  const char** dataSet   = data.GetDataSet();
  mDataMap.clear();
  for(int i = 0; i < data.GetDataSetSize(); ++i)
  {
    mDataMap[std::string(mimeTypes[i])] = std::string(dataSet[i]);
  }

  mSourceCallback = callback;
  mDragWindow     = shadowWindow;

  // Register MIME types with ecore_x DnD (sets XdndTypeList property on source window)
  int                      mimeCount = data.GetMimeTypesSize();
  std::vector<const char*> typesArray(static_cast<std::size_t>(mimeCount) + 1);
  for(int i = 0; i < mimeCount; ++i)
  {
    typesArray[i] = mimeTypes[i];
  }
  typesArray[mimeCount] = nullptr;
  ecore_x_dnd_types_set(mSourceWindowId, typesArray.data(), static_cast<unsigned int>(mimeCount));

  // Pre-populate MIME types so DoPositionUpdate can use them immediately.
  // For same-process drags ecore_x never fires XDND_ENTER (pos->win stays 0),
  // so we cannot rely on HandleXdndEnter to set these.
  mCurrentMimeCount = mimeCount < MAX_MIME_SIZE ? mimeCount : MAX_MIME_SIZE;
  for(int i = 0; i < mCurrentMimeCount; ++i)
  {
    if(mimeTypes[i])
    {
      strncpy(mCurrentMimeTypesBuf[i], mimeTypes[i], MAX_MIME_LEN - 1);
      mCurrentMimeTypesBuf[i][MAX_MIME_LEN - 1] = '\0';
    }
    else
    {
      mCurrentMimeTypesBuf[i][0] = '\0';
    }
    mCurrentMimeTypes[i] = mCurrentMimeTypesBuf[i];
  }
  mCurrentMimeTypes[mCurrentMimeCount] = nullptr;

  // Register position callback to move shadow window during drag
  ecore_x_dnd_callback_pos_update_set(DragPosUpdateCb, this);

  // Begin drag: grabs pointer and makes source window own XdndSelection
  ecore_x_dnd_begin(mSourceWindowId, nullptr, 0);

  CallSourceEvent(Dali::DragAndDrop::SourceEventType::START);

  return true;
}

// ---------------------------------------------------------------------------
// AddListener / RemoveListener
// ---------------------------------------------------------------------------

bool DragAndDropX11::AddListener(Dali::Actor                            target,
                                 char*                                  mimeType,
                                 Dali::DragAndDrop::DragAndDropFunction callback)
{
  for(const auto& dt : mDropTargets)
  {
    if(dt.target == target)
    {
      return false;
    }
  }

  Ecore_X_Window parentWindowId = INVALID_X_WINDOW_ID;
  auto           window         = Dali::Window::Get(target);

  if(!window)
  {
    // Actor not yet on scene: defer window ID resolution
    target.SceneConnectedSignal().Connect(this, &DragAndDropX11::DropTargetSceneOn);
  }
  else
  {
    parentWindowId = AnyCast<Ecore_X_Window>(window.GetNativeHandle());
    // Note: ecore_x_dnd_aware_set was already called for all windows
    // in WindowBaseEcoreX::Initialize() (window-base-ecore-x.cpp)
  }

  DropTarget td;
  td.target         = target;
  td.mimeType       = mimeType;
  td.callback       = callback;
  td.inside         = false;
  td.parentWindowId = parentWindowId;
  mDropTargets.push_back(td);

  return true;
}

bool DragAndDropX11::AddListener(Dali::Window                           target,
                                 char*                                  mimeType,
                                 Dali::DragAndDrop::DragAndDropFunction callback)
{
  for(const auto& wt : mDropWindowTargets)
  {
    if(wt.target == target)
    {
      return false;
    }
  }

  Ecore_X_Window windowId = AnyCast<Ecore_X_Window>(target.GetNativeHandle());
  if(windowId == INVALID_X_WINDOW_ID)
  {
    return false;
  }

  DropWindowTarget wt;
  wt.target   = target;
  wt.mimeType = mimeType;
  wt.callback = callback;
  wt.inside   = false;
  wt.windowId = windowId;
  mDropWindowTargets.push_back(wt);

  return true;
}

bool DragAndDropX11::RemoveListener(Dali::Actor target)
{
  for(auto itr = mDropTargets.begin(); itr != mDropTargets.end(); ++itr)
  {
    if(itr->target == target)
    {
      mDropTargets.erase(itr);
      break;
    }
  }
  return true;
}

bool DragAndDropX11::RemoveListener(Dali::Window target)
{
  for(auto itr = mDropWindowTargets.begin(); itr != mDropWindowTargets.end(); ++itr)
  {
    if(itr->target == target)
    {
      mDropWindowTargets.erase(itr);
      break;
    }
  }
  return true;
}

// ---------------------------------------------------------------------------
// HandleXdndEnter  (ECORE_X_EVENT_XDND_ENTER)
// Store offered MIME types for the duration of this drag session.
// XDND_POSITION events do not carry MIME info, so we cache them here.
// ---------------------------------------------------------------------------

void DragAndDropX11::HandleXdndEnter(void* event)
{
  auto* ev = static_cast<Ecore_X_Event_Xdnd_Enter*>(event);

  // After a same-process drag ends, ecore_x continues the XDND protocol with the shadow
  // window (or wherever the cursor is) until it receives XdndFinished. Ignore these spurious
  // ENTER events by checking whether they come from our own former source window.
  if(mSourceWindowId == INVALID_X_WINDOW_ID && ev->source == mPrevSourceWindowId)
  {
    return;
  }

  mCurrentSourceWindow = ev->source;
  mCurrentMimeCount    = (ev->num_types < MAX_MIME_SIZE) ? ev->num_types : MAX_MIME_SIZE;

  for(int i = 0; i < mCurrentMimeCount; ++i)
  {
    if(ev->types[i])
    {
      strncpy(mCurrentMimeTypesBuf[i], ev->types[i], MAX_MIME_LEN - 1);
      mCurrentMimeTypesBuf[i][MAX_MIME_LEN - 1] = '\0';
    }
    else
    {
      mCurrentMimeTypesBuf[i][0] = '\0';
    }
    mCurrentMimeTypes[i] = mCurrentMimeTypesBuf[i];
  }
  mCurrentMimeTypes[mCurrentMimeCount] = nullptr;
}

// ---------------------------------------------------------------------------
// HandleXdndLeave  (ECORE_X_EVENT_XDND_LEAVE)
// ---------------------------------------------------------------------------

void DragAndDropX11::HandleXdndLeave(void* /*event*/)
{
  if(mSourceWindowId != INVALID_X_WINDOW_ID)
  {
    return; // same-process drag: DoPositionUpdate owns state
  }
  ResetDropTargets();
  mCurrentMimeCount    = 0;
  mCurrentSourceWindow = 0;
}

void DragAndDropX11::ResetDropTargets()
{
  Dali::DragAndDrop::DragEvent leaveEvent;
  leaveEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
  leaveEvent.SetPosition(Dali::Vector2(DEFAULT_POSITION, DEFAULT_POSITION));

  for(auto& dt : mDropTargets)
  {
    if(dt.inside)
    {
      dt.callback(leaveEvent);
      dt.inside = false;
    }
  }
  for(auto& wt : mDropWindowTargets)
  {
    if(wt.inside)
    {
      wt.callback(leaveEvent);
      wt.inside = false;
    }
  }
}

// ---------------------------------------------------------------------------
// CalculateDragEvent  (ECORE_X_EVENT_XDND_POSITION)
// Determine ENTER / LEAVE / MOVE for registered targets and reply with
// XdndStatus so the source knows whether the drop would be accepted.
// ---------------------------------------------------------------------------

bool DragAndDropX11::CalculateDragEvent(void* event)
{
  // Note: during same-process drag, XDND_POSITION is received by sub-windows (Window A/B)
  // because owner_events=True causes pointer events over same-client windows to go there.
  // DragPosUpdateCb is NOT called for those positions, so we rely on CalculateDragEvent
  // to deliver ENTER/MOVE/LEAVE for targets in those windows.
  // HandleXdndLeave is guarded to prevent ResetDropTargets from corrupting same-window state.

  // XdndActionCopy atom used in all STATUS replies from this function
  static Ecore_X_Atom xdndActionCopy = ecore_x_atom_get("XdndActionCopy");
  Ecore_X_Rectangle   rect           = {0, 0, 0, 0};

  if(mCurrentMimeCount == 0)
  {
    // No active drag session: send rejection STATUS so source does not hang waiting
    ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, xdndActionCopy);
    ecore_x_flush();
    return false;
  }

  auto* ev = static_cast<Ecore_X_Event_Xdnd_Position*>(event);

  Dali::DragAndDrop::DragEvent dragEvent;
  dragEvent.SetMimeTypes(mCurrentMimeTypes, mCurrentMimeCount);

  bool anyAccepted = false;
  int  cx          = ev->position.x; // screen-absolute cursor X
  int  cy          = ev->position.y; // screen-absolute cursor Y

  // --- Actor targets ---
  for(std::size_t i = 0; i < mDropTargets.size(); ++i)
  {
    // SCREEN_POSITION is window-client-area relative; cx/cy are screen-absolute.
    // Use GetWindowRootOffset to convert SCREEN_POSITION to screen-absolute.
    int winX, winY;
    GetWindowRootOffset(mDropTargets[i].parentWindowId, winX, winY);
    Vector2 pos  = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SCREEN_POSITION);
    Vector2 size = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);
    int     absX = winX + static_cast<int>(pos.x);
    int     absY = winY + static_cast<int>(pos.y);

    bool currentInside = IsIntersection(cx, cy, absX, absY, static_cast<int>(size.width), static_cast<int>(size.height));

    if(currentInside && !mDropTargets[i].inside)
    {
      mDropTargets[i].inside = true;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::ENTER);
      dragEvent.SetPosition(Dali::Vector2(cx, cy));
      mDropTargets[i].callback(dragEvent);
      anyAccepted = true;
    }
    else if(!currentInside && mDropTargets[i].inside)
    {
      mDropTargets[i].inside = false;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
      dragEvent.SetPosition(Dali::Vector2(cx, cy));
      mDropTargets[i].callback(dragEvent);
    }
    else if(currentInside && mDropTargets[i].inside)
    {
      dragEvent.SetAction(Dali::DragAndDrop::DragType::MOVE);
      dragEvent.SetPosition(Dali::Vector2(cx, cy));
      mDropTargets[i].callback(dragEvent);
      anyAccepted = true;
    }
  }

  // --- Window targets ---
  for(std::size_t i = 0; i < mDropWindowTargets.size(); ++i)
  {
    // No window ID filter: same reason as actor targets above.
    auto winPositionSize = mDropWindowTargets[i].target.GetPositionSize();

    bool currentInside = IsIntersection(cx, cy, winPositionSize.x, winPositionSize.y, winPositionSize.width, winPositionSize.height);

    // Position delivered to callback is window-relative
    Dali::Vector2 relPos(cx - winPositionSize.x, cy - winPositionSize.y);

    if(currentInside && !mDropWindowTargets[i].inside)
    {
      mDropWindowTargets[i].inside = true;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::ENTER);
      dragEvent.SetPosition(relPos);
      mDropWindowTargets[i].callback(dragEvent);
      anyAccepted = true;
    }
    else if(!currentInside && mDropWindowTargets[i].inside)
    {
      mDropWindowTargets[i].inside = false;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
      dragEvent.SetPosition(relPos);
      mDropWindowTargets[i].callback(dragEvent);
    }
    else if(currentInside && mDropWindowTargets[i].inside)
    {
      dragEvent.SetAction(Dali::DragAndDrop::DragType::MOVE);
      dragEvent.SetPosition(relPos);
      mDropWindowTargets[i].callback(dragEvent);
      anyAccepted = true;
    }
  }

  // Reply to source with XdndStatus (accept/reject + action)
  ecore_x_dnd_send_status(anyAccepted ? EINA_TRUE : EINA_FALSE, EINA_FALSE, rect, xdndActionCopy);
  ecore_x_flush();

  return true;
}

// ---------------------------------------------------------------------------
// CalculateViewRegion  (ECORE_X_EVENT_XDND_DROP)
// Find the target under the cursor, then request the data via XdndSelection.
// The actual data arrives asynchronously in ReceiveData (SELECTION_NOTIFY).
// ---------------------------------------------------------------------------

bool DragAndDropX11::CalculateViewRegion(void* event)
{
  if(mSourceWindowId != INVALID_X_WINDOW_ID)
  {
    return false; // same-process drag: DoPositionUpdate owns events
  }

  auto* ev = static_cast<Ecore_X_Event_Xdnd_Drop*>(event);

  mTargetIndex       = -1;
  mWindowTargetIndex = -1;

  if(mCurrentMimeCount == 0)
  {
    ecore_x_dnd_send_finished();
    return false;
  }

  int cx = ev->position.x;
  int cy = ev->position.y;

  // --- Actor targets ---
  for(std::size_t i = 0; i < mDropTargets.size(); ++i)
  {
    // SCREEN_POSITION is window-client-area relative; cx/cy are screen-absolute.
    // Convert SCREEN_POSITION to screen-absolute via the window root offset, exactly as
    // CalculateDragEvent does. Without this the hit-test is wrong for any window not at
    // the screen origin, so cross-process drops would be rejected even though the drag
    // hover (POSITION/STATUS) worked.
    int winX, winY;
    GetWindowRootOffset(mDropTargets[i].parentWindowId, winX, winY);
    Vector2 pos  = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SCREEN_POSITION);
    Vector2 size = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);
    int     absX = winX + static_cast<int>(pos.x);
    int     absY = winY + static_cast<int>(pos.y);

    if(!IsIntersection(cx, cy, absX, absY, static_cast<int>(size.width), static_cast<int>(size.height)))
    {
      continue;
    }

    for(int j = 0; j < mCurrentMimeCount; ++j)
    {
      const char* availMime = mCurrentMimeTypes[j];
      if(!availMime || !availMime[0])
      {
        continue;
      }

      if(!strcmp(mDropTargets[i].mimeType.c_str(), "*/*") ||
         !strcmp(availMime, mDropTargets[i].mimeType.c_str()))
      {
        mTargetIndex = static_cast<int>(i);
        mPosition    = pos;
        ecore_x_selection_xdnd_request(ev->win, availMime);
        ecore_x_flush();
        return true;
      }
    }
  }

  // --- Window targets ---
  for(std::size_t i = 0; i < mDropWindowTargets.size(); ++i)
  {
    auto winPositionSize = mDropWindowTargets[i].target.GetPositionSize();

    if(!IsIntersection(cx, cy, winPositionSize.x, winPositionSize.y, winPositionSize.width, winPositionSize.height))
    {
      continue;
    }

    for(int j = 0; j < mCurrentMimeCount; ++j)
    {
      const char* availMime = mCurrentMimeTypes[j];
      if(!availMime || !availMime[0])
      {
        continue;
      }

      if(!strcmp(mDropWindowTargets[i].mimeType.c_str(), "*/*") ||
         !strcmp(availMime, mDropWindowTargets[i].mimeType.c_str()))
      {
        mWindowTargetIndex = static_cast<int>(i);
        mWindowPosition    = Dali::Vector2(winPositionSize.x, winPositionSize.y);
        ecore_x_selection_xdnd_request(ev->win, availMime);
        ecore_x_flush();
        return true;
      }
    }
  }

  // No matching target found: tell source drop is done (rejected)
  ecore_x_dnd_send_finished();
  return false;
}

// ---------------------------------------------------------------------------
// ReceiveData  (ECORE_X_EVENT_SELECTION_NOTIFY)
// Deliver DROP event with the received data to the registered callback.
// ---------------------------------------------------------------------------

void DragAndDropX11::ReceiveData(void* event)
{
  auto* ev = static_cast<Ecore_X_Event_Selection_Notify*>(event);

  if(ev->selection != ECORE_X_SELECTION_XDND)
  {
    return;
  }
  if(!ev->data)
  {
    return;
  }

  auto* selData = static_cast<Ecore_X_Selection_Data*>(ev->data);
  if(!selData)
  {
    return;
  }

  // ecore parses well-known XDND types into specialised structs whose base
  // Ecore_X_Selection_Data.data may be null. Extract the payload per content type:
  //   FILES (e.g. text/uri-list) -> Ecore_X_Selection_Data_Files.files[]
  //   TEXT                       -> Ecore_X_Selection_Data_Text.text
  //   otherwise (CUSTOM)         -> raw base data buffer
  // Ecore_X_Selection_Content enum values (stable ecore ABI):
  //   NONE=0, TEXT=1, FILES=2, X_MOZ_URL=3, TARGETS=4, CUSTOM=5
  constexpr int SEL_CONTENT_TEXT  = 1;
  constexpr int SEL_CONTENT_FILES = 2;

  std::string extracted;
  bool        haveData = false;

  switch(static_cast<int>(selData->content))
  {
    case SEL_CONTENT_FILES:
    {
      auto* filesData = static_cast<Ecore_X_Selection_Data_Files*>(ev->data);
      for(int i = 0; i < filesData->num_files; ++i)
      {
        if(i != 0) extracted += "\n";
        if(filesData->files[i]) extracted += filesData->files[i];
      }
      haveData = (filesData->num_files > 0);
      break;
    }
    case SEL_CONTENT_TEXT:
    {
      auto* textData = static_cast<Ecore_X_Selection_Data_Text*>(ev->data);
      if(textData->text)
      {
        extracted = textData->text;
        haveData  = true;
      }
      break;
    }
    default:
    {
      if(selData->data && selData->length > 0)
      {
        extracted.assign(reinterpret_cast<char*>(selData->data), static_cast<std::size_t>(selData->length));
        haveData = true;
      }
      break;
    }
  }

  if(!haveData)
  {
    return;
  }

  char* receivedData = const_cast<char*>(extracted.c_str());

  mimesPool[0] = ev->target; // resolved MIME type string
  mimesPool[1] = nullptr;

  if(mTargetIndex != -1)
  {
    Dali::DragAndDrop::DragEvent dragEvent(
      Dali::DragAndDrop::DragType::DROP, mPosition, mimesPool, 1, receivedData);
    mDropTargets[mTargetIndex].callback(dragEvent);
    mDropTargets[mTargetIndex].inside = false;
    auto win                          = Dali::Window::Get(mDropTargets[mTargetIndex].target);
    if(win) win.Activate();
    mTargetIndex = -1;
  }

  if(mWindowTargetIndex != -1)
  {
    Dali::DragAndDrop::DragEvent dragEvent(
      Dali::DragAndDrop::DragType::DROP, mWindowPosition, mimesPool, 1, receivedData);
    mDropWindowTargets[mWindowTargetIndex].callback(dragEvent);
    mDropWindowTargets[mWindowTargetIndex].inside = false;
    mDropWindowTargets[mWindowTargetIndex].target.Activate();
    mWindowTargetIndex = -1;
  }

  // Inform source that the drop is fully processed
  ecore_x_dnd_send_finished();

  // Clear session MIME state
  mCurrentMimeCount    = 0;
  mCurrentSourceWindow = 0;
}

// ---------------------------------------------------------------------------
// SendData  (ECORE_X_EVENT_SELECTION_REQUEST)
// Source-side: a target has requested our drag data.
// Write data to the requestor's X property and send SelectionNotify.
// ---------------------------------------------------------------------------

void DragAndDropX11::SendData(void* event)
{
  auto* ev = static_cast<Ecore_X_Event_Selection_Request*>(event);

  // Only handle XdndSelection requests (ignore clipboard etc.)
  static Ecore_X_Atom xdndSelAtom = ecore_x_atom_get("XdndSelection");
  if(ev->selection != xdndSelAtom)
  {
    return;
  }

  // Resolve atom → MIME type string
  char* mimeName = ecore_x_atom_name_get(ev->target);
  if(!mimeName)
  {
    return;
  }

  std::string key(mimeName);
  free(mimeName);

  auto it = mDataMap.find(key);
  if(it == mDataMap.end())
  {
    return;
  }

  const std::string& dataStr  = it->second;
  Ecore_X_Atom       propAtom = ev->property ? ev->property : ev->target;

  // Write data into the requestor window's property.
  // ecore API takes void* (non-const); data is read-only inside ecore.
  ecore_x_window_prop_property_set(
    ev->requestor,
    propAtom,
    ev->target,
    8,
    const_cast<char*>(dataStr.c_str()),
    static_cast<int>(dataStr.size() + 1)); // include null terminator

  // Notify requestor that the property is ready
  ecore_x_selection_notify_send(ev->requestor, ev->selection, ev->target, propAtom, ev->time);
  ecore_x_flush();
}

// ---------------------------------------------------------------------------
// HandleXdndFinished  (ECORE_X_EVENT_XDND_FINISHED)
// Source-side: target confirms it received the data.
// ---------------------------------------------------------------------------

void DragAndDropX11::HandleXdndFinished(void* event)
{
  auto* ev = static_cast<Ecore_X_Event_Xdnd_Finished*>(event);

  if(ev->completed)
  {
    // Target accepted and processed the drop
    CallSourceEvent(Dali::DragAndDrop::SourceEventType::ACCEPT);
    CallSourceEvent(Dali::DragAndDrop::SourceEventType::FINISH);
  }
  else
  {
    CallSourceEvent(Dali::DragAndDrop::SourceEventType::CANCEL);
  }

  // Clean up drag session
  mSourceWindowId = 0;
  ecore_x_dnd_callback_pos_update_set(nullptr, nullptr);
}

void DragAndDropX11::CallSourceEvent(Dali::DragAndDrop::SourceEventType type)
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

// ---------------------------------------------------------------------------
// UpdateDragWindowPosition
// Called from DragPosUpdateCb to move shadow window with cursor during drag.
// ---------------------------------------------------------------------------

void DragAndDropX11::UpdateDragWindowPosition(int x, int y)
{
  if(mDragWindow)
  {
    auto positionSize = mDragWindow.GetPositionSize();
    mDragWindow.SetPositionSize(Dali::PositionSize(x, y, positionSize.width, positionSize.height));
  }
  mLastDragX = x;
  mLastDragY = y;

  // Same-process drags: ecore_x never delivers XDND target events (pos->win == 0).
  // Drive ENTER/MOVE/LEAVE directly from cursor position.
  DoPositionUpdate(x, y);
}

// ---------------------------------------------------------------------------
// DoPositionUpdate
// Screen-coordinate intersection check used when ecore_x does not deliver
// XDND target events (same-process drag: pos->win always 0).
// ---------------------------------------------------------------------------

void DragAndDropX11::DoPositionUpdate(int cx, int cy)
{
  if(mSourceWindowId == INVALID_X_WINDOW_ID || mCurrentMimeCount == 0)
  {
    return;
  }

  Dali::DragAndDrop::DragEvent dragEvent;
  dragEvent.SetMimeTypes(mCurrentMimeTypes, mCurrentMimeCount);

  // --- Actor targets ---
  for(std::size_t i = 0; i < mDropTargets.size(); ++i)
  {
    // SCREEN_POSITION is window-client-area relative; cx/cy are screen-absolute.
    // Add the window's root offset to convert SCREEN_POSITION to screen-absolute.
    int winX, winY;
    GetWindowRootOffset(mDropTargets[i].parentWindowId, winX, winY);
    Vector2 pos  = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SCREEN_POSITION);
    Vector2 size = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);
    int     absX = winX + static_cast<int>(pos.x);
    int     absY = winY + static_cast<int>(pos.y);
    bool    cur  = IsIntersection(cx, cy, absX, absY, static_cast<int>(size.width), static_cast<int>(size.height));

    if(cur && !mDropTargets[i].inside)
    {
      mDropTargets[i].inside = true;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::ENTER);
      dragEvent.SetPosition(Dali::Vector2(cx, cy));
      mDropTargets[i].callback(dragEvent);
    }
    else if(!cur && mDropTargets[i].inside)
    {
      mDropTargets[i].inside = false;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
      dragEvent.SetPosition(Dali::Vector2(cx, cy));
      mDropTargets[i].callback(dragEvent);
    }
    else if(cur)
    {
      dragEvent.SetAction(Dali::DragAndDrop::DragType::MOVE);
      dragEvent.SetPosition(Dali::Vector2(cx, cy));
      mDropTargets[i].callback(dragEvent);
    }
  }

  // --- Window targets ---
  for(std::size_t i = 0; i < mDropWindowTargets.size(); ++i)
  {
    auto          winPositionSize = mDropWindowTargets[i].target.GetPositionSize();
    bool          cur             = IsIntersection(cx, cy, winPositionSize.x, winPositionSize.y, winPositionSize.width, winPositionSize.height);
    Dali::Vector2 relPos(cx - winPositionSize.x, cy - winPositionSize.y);

    if(cur && !mDropWindowTargets[i].inside)
    {
      mDropWindowTargets[i].inside = true;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::ENTER);
      dragEvent.SetPosition(relPos);
      mDropWindowTargets[i].callback(dragEvent);
    }
    else if(!cur && mDropWindowTargets[i].inside)
    {
      mDropWindowTargets[i].inside = false;
      dragEvent.SetAction(Dali::DragAndDrop::DragType::LEAVE);
      dragEvent.SetPosition(relPos);
      mDropWindowTargets[i].callback(dragEvent);
    }
    else if(cur)
    {
      dragEvent.SetAction(Dali::DragAndDrop::DragType::MOVE);
      dragEvent.SetPosition(relPos);
      mDropWindowTargets[i].callback(dragEvent);
    }
  }
}

// ---------------------------------------------------------------------------
// HandleMouseButtonUp
// Same-process drop: ecore_x does not send XdndDrop when pos->win == 0.
// We handle button release directly and deliver DROP to the target under cursor.
// ---------------------------------------------------------------------------

void DragAndDropX11::HandleMouseButtonUp(void* event)
{
  if(mSourceWindowId == INVALID_X_WINDOW_ID)
  {
    return; // not dragging
  }

  // Use exact root coordinates from the button-up event (more accurate than mLastDragX/Y).
  auto* ev = static_cast<Ecore_Event_Mouse_Button*>(event);
  int   cx = ev ? ev->root.x : mLastDragX;
  int   cy = ev ? ev->root.y : mLastDragY;

  bool dropped = false;

  // --- Actor targets ---
  for(std::size_t i = 0; i < mDropTargets.size(); ++i)
  {
    // Do not rely on `inside` flag — it may have been corrupted by XDND events.
    // Use the accurate intersection check directly.
    int winX, winY;
    GetWindowRootOffset(mDropTargets[i].parentWindowId, winX, winY);
    Vector2 pos  = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SCREEN_POSITION);
    Vector2 size = mDropTargets[i].target.GetProperty<Vector2>(Dali::Actor::Property::SIZE);
    int     absX = winX + static_cast<int>(pos.x);
    int     absY = winY + static_cast<int>(pos.y);
    if(!IsIntersection(cx, cy, absX, absY, static_cast<int>(size.width), static_cast<int>(size.height)))
    {
      continue;
    }

    // Find matching MIME type
    for(int j = 0; j < mCurrentMimeCount; ++j)
    {
      const char* mime = mCurrentMimeTypes[j];
      if(!mime || !mime[0])
      {
        continue;
      }

      if(!strcmp(mDropTargets[i].mimeType.c_str(), "*/*") ||
         !strcmp(mime, mDropTargets[i].mimeType.c_str()))
      {
        auto        it       = mDataMap.find(std::string(mime));
        const char* dropData = (it != mDataMap.end()) ? it->second.c_str() : "";

        mimesPool[0] = mime;
        mimesPool[1] = nullptr;
        // DragEvent API takes char*; dropData is read-only (from std::string::c_str())
        Dali::DragAndDrop::DragEvent dropEvent(Dali::DragAndDrop::DragType::DROP, Dali::Vector2(cx, cy), mimesPool, 1, const_cast<char*>(dropData));
        mDropTargets[i].callback(dropEvent);
        mDropTargets[i].inside = false;
        auto win               = Dali::Window::Get(mDropTargets[i].target);
        if(win) win.Activate();
        dropped = true;
        break;
      }
    }
  }

  // --- Window targets ---
  for(std::size_t i = 0; i < mDropWindowTargets.size(); ++i)
  {
    auto winPositionSize = mDropWindowTargets[i].target.GetPositionSize();
    if(!IsIntersection(cx, cy, winPositionSize.x, winPositionSize.y, winPositionSize.width, winPositionSize.height))
    {
      continue;
    }

    for(int j = 0; j < mCurrentMimeCount; ++j)
    {
      const char* mime = mCurrentMimeTypes[j];
      if(!mime || !mime[0])
      {
        continue;
      }

      if(!strcmp(mDropWindowTargets[i].mimeType.c_str(), "*/*") ||
         !strcmp(mime, mDropWindowTargets[i].mimeType.c_str()))
      {
        auto        it       = mDataMap.find(std::string(mime));
        const char* dropData = (it != mDataMap.end()) ? it->second.c_str() : "";

        mimesPool[0] = mime;
        mimesPool[1] = nullptr;
        Dali::Vector2 relPos(cx - winPositionSize.x, cy - winPositionSize.y);
        // DragEvent API takes char*; dropData is read-only (from std::string::c_str())
        Dali::DragAndDrop::DragEvent dropEvent(Dali::DragAndDrop::DragType::DROP, relPos, mimesPool, 1, const_cast<char*>(dropData));
        mDropWindowTargets[i].callback(dropEvent);
        mDropWindowTargets[i].inside = false;
        mDropWindowTargets[i].target.Activate();
        dropped = true;
        break;
      }
    }
  }

  // Send LEAVE to any remaining inside targets
  ResetDropTargets();

  // Notify source
  if(dropped)
  {
    // Local (same-process) drop succeeded.
    CallSourceEvent(Dali::DragAndDrop::SourceEventType::ACCEPT);
    CallSourceEvent(Dali::DragAndDrop::SourceEventType::FINISH);
  }
  else
  {
    // No local target matched. This may be a cross-process drag: ask ecore_x to send
    // XdndDrop to whichever external window currently accepts the drop (will_accept).
    // If one does, ecore_x_dnd_drop() returns true and the drop completes asynchronously
    // via XdndFinished (HandleXdndFinished delivers ACCEPT/FINISH and cleans up the
    // session). Without this call ecore never sends XdndDrop, so cross-process drops
    // silently fail even though the drag hover (POSITION/STATUS) worked.
    bool dropSent = ecore_x_dnd_drop();
    if(dropSent)
    {
      // Cross-process drop in progress; keep the session until XdndFinished arrives.
      return;
    }
    // Empty drop (no accepting target anywhere): cancel.
    CallSourceEvent(Dali::DragAndDrop::SourceEventType::CANCEL);
  }

  // Clean up session state (same-process drop or empty drop)
  mPrevSourceWindowId  = mSourceWindowId; // saved to filter post-drop spurious XDND_ENTER
  mSourceWindowId      = INVALID_X_WINDOW_ID;
  mCurrentMimeCount    = 0;
  mCurrentSourceWindow = 0;
  mLastDragX           = DEFAULT_POSITION;
  mLastDragY           = DEFAULT_POSITION;
  ecore_x_dnd_callback_pos_update_set(nullptr, nullptr);
}

// ---------------------------------------------------------------------------
// DropTargetSceneOn
// Resolves the parent window ID for an actor that was registered before it
// was added to the scene.
// ---------------------------------------------------------------------------

void DragAndDropX11::DropTargetSceneOn(Dali::Actor target)
{
  target.SceneConnectedSignal().Disconnect(this, &DragAndDropX11::DropTargetSceneOn);

  auto window = Dali::Window::Get(target);
  if(!window)
  {
    return;
  }

  Ecore_X_Window xwin = AnyCast<Ecore_X_Window>(window.GetNativeHandle());

  for(auto& dt : mDropTargets)
  {
    if(dt.target == target)
    {
      dt.parentWindowId = xwin;
      break;
    }
  }
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
