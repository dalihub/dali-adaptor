#ifndef DALI_INTERNAL_DRAG_AND_DROP_TIZEN_WAYLAND_TCORE_WL_H
#define DALI_INTERNAL_DRAG_AND_DROP_TIZEN_WAYLAND_TCORE_WL_H

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

// EXTERNAL INCLUDES
#include <limits>
#include <map>
#include <tizen_core_wl.h>
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/drag-and-drop/common/drag-and-drop-impl.h>

namespace Dali
{
class DragAndDrop;

namespace Internal
{
namespace Adaptor
{

struct DropTarget
{
  Dali::Actor                            target;
  std::string                            mimeType;
  DragAndDrop::DragCallback               callback;
  tizen_core_wl_window_h                parentWindow{nullptr};
  bool                                   inside;
};

struct DropWindowTarget
{
  Dali::Window                           target;
  std::string                            mimeType;
  DragAndDrop::DragCallback               callback;
  tizen_core_wl_window_h                window{nullptr};
  bool                                   inside;
};

/**
 * DragAndDrop Implementation (tizen_core_wl)
 */
class DragAndDropTcoreWl : public Dali::Internal::Adaptor::DragAndDrop,
                           public Dali::ConnectionTracker
{
public:
  DragAndDropTcoreWl();
  ~DragAndDropTcoreWl() override;

  bool StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& data, SourceCallback callback) override;
  bool AddListener(Dali::Actor target, const Dali::String& mimeType, DragCallback callback) override;
  bool AddListener(Dali::Window target, const Dali::String& mimeType, DragCallback callback) override;
  bool RemoveListener(Dali::Actor target) override;
  bool RemoveListener(Dali::Window target) override;

  void SendData(void* event) override;
  void ReceiveData(void* event) override;
  bool CalculateDragEvent(void* event) override;
  bool CalculateViewRegion(void* event) override;

  void TriggerDragEventForTarget(int targetIndex, void* event, char** mimes, int mimesCount, DragEventBuilder& dragEvent);
  void TriggerDragEventForWindowTarget(int targetIndex, void* event, char** mimes, int mimesCount, DragEventBuilder& dragEvent);
  void ProcessDragEventsForTargets(void* event, DragEventBuilder& dragEvent, char** mimes, int mimesCount);
  void ProcessDragEventsForWindowTargets(void* event, DragEventBuilder& dragEvent, char** mimes, int mimesCount);
  bool ProcessDropEventsForTargets(void* event, char** mimes, int mimesCount);
  bool ProcessDropEventsForWindowTargets(void* event, char** mimes, int mimesCount);

  void CallSourceEvent(Dali::DragAndDrop::SourceEventType type);
  void ResetDropTargets();

private:
  void DropTargetSceneOn(Dali::Actor target);
  Vector2 RecalculatePositionByOrientation(int x, int y, Dali::Window window);

  DragAndDropTcoreWl(const DragAndDropTcoreWl&)            = delete;
  DragAndDropTcoreWl& operator=(const DragAndDropTcoreWl&) = delete;
  DragAndDropTcoreWl(DragAndDropTcoreWl&&)                = delete;
  DragAndDropTcoreWl& operator=(DragAndDropTcoreWl&&)       = delete;

private:
  Dali::Window                       mDragWindow;
  uint32_t                           mSerial{std::numeric_limits<uint32_t>::max()};
  tizen_core_event_h                 mEvent{nullptr};
  tizen_core_wl_event_listener_h    mSendHandler{nullptr};
  tizen_core_wl_event_listener_h    mSourceEndHandler{nullptr};
  tizen_core_wl_event_listener_h    mSourceDropHandler{nullptr};
  tizen_core_wl_event_listener_h    mReceiveHandler{nullptr};
  tizen_core_wl_event_listener_h    mMotionHandler{nullptr};
  tizen_core_wl_event_listener_h    mDropHandler{nullptr};
  tizen_core_wl_event_listener_h    mEnterHandler{nullptr};
  tizen_core_wl_event_listener_h    mLeaveHandler{nullptr};
  int                                mTargetIndex{-1};
  int                                mWindowTargetIndex{-1};
  Dali::Vector2                      mPosition;
  Dali::Vector2                      mWindowPosition;
  SourceCallback                      mSourceCallback{};
  std::vector<DropTarget>            mDropTargets;
  std::vector<DropWindowTarget>      mDropWindowTargets;
  std::map<std::string, std::string> mDataMap;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_DRAG_AND_DROP_TIZEN_WAYLAND_TCORE_WL_H
