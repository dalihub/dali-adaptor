#ifndef DALI_INTERNAL_DRAG_AND_DROP_X11_H
#define DALI_INTERNAL_DRAG_AND_DROP_X11_H

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
#include <Ecore.h>
#include <Ecore_Input.h>
#include <Ecore_X.h>
#include <map>
#include <string>
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
  bool                                   inside;
  Ecore_X_Window                         parentWindowId;
};

struct DropWindowTarget
{
  Dali::Window                           target;
  std::string                            mimeType;
  DragAndDrop::DragCallback               callback;
  bool                                   inside;
  Ecore_X_Window                         windowId;
};

/**
 * DragAndDrop Implementation for Ubuntu X11 (XDND protocol via Ecore_X)
 */
class DragAndDropX11 : public Dali::Internal::Adaptor::DragAndDrop,
                       public Dali::ConnectionTracker
{
public:
  DragAndDropX11();
  ~DragAndDropX11() override;

  bool StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& data, SourceCallback callback) override;
  bool AddListener(Dali::Actor target, const Dali::String& mimeType, DragCallback callback) override;
  bool AddListener(Dali::Window target, const Dali::String& mimeType, DragCallback callback) override;
  bool RemoveListener(Dali::Actor target) override;
  bool RemoveListener(Dali::Window target) override;
  void SendData(void* event) override;
  void ReceiveData(void* event) override;
  bool CalculateDragEvent(void* event) override;
  bool CalculateViewRegion(void* event) override;

  // Called by static ecore event callbacks
  void HandleXdndEnter(void* event);
  void HandleXdndLeave(void* event);
  void HandleXdndFinished(void* event);
  void HandleMouseButtonUp(void* event);
  void CallSourceEvent(Dali::DragAndDrop::SourceEventType type);
  void ResetDropTargets();
  void UpdateDragWindowPosition(int x, int y);

  // Inline ENTER/MOVE/LEAVE using screen-position intersection (used by DragPosUpdateCb
  // because ecore_x does not deliver XDND target events for same-process drags)
  void DoPositionUpdate(int cx, int cy);

private:
  void DropTargetSceneOn(Dali::Actor target);

private:
  DragAndDropX11(const DragAndDropX11&)       = delete;
  DragAndDropX11& operator=(DragAndDropX11&)  = delete;
  DragAndDropX11(DragAndDropX11&&)            = delete;
  DragAndDropX11& operator=(DragAndDropX11&&) = delete;

private:
  static constexpr int MAX_MIME_SIZE = 10;
  static constexpr int MAX_MIME_LEN  = 128;

  // Current drag session: MIME types received from XDND_ENTER (copied strings)
  char        mCurrentMimeTypesBuf[MAX_MIME_SIZE][MAX_MIME_LEN]{};  // 1280 bytes
  const char* mCurrentMimeTypes[MAX_MIME_SIZE + 1]{};               //   88 bytes

  // Source drag state
  std::map<std::string, std::string> mDataMap;                       //   48 bytes
  SourceCallback                      mSourceCallback{};              //   32 bytes

  // Drop target lists
  std::vector<DropTarget>       mDropTargets;                        //   24 bytes
  std::vector<DropWindowTarget> mDropWindowTargets;                  //   24 bytes

  // Ecore event handlers
  Ecore_Event_Handler* mEnterHandler{nullptr};                       //    8 bytes
  Ecore_Event_Handler* mMotionHandler{nullptr};
  Ecore_Event_Handler* mDropHandler{nullptr};
  Ecore_Event_Handler* mLeaveHandler{nullptr};
  Ecore_Event_Handler* mFinishedHandler{nullptr};
  Ecore_Event_Handler* mSendHandler{nullptr};
  Ecore_Event_Handler* mReceiveHandler{nullptr};
  Ecore_Event_Handler* mMouseUpHandler{nullptr};

  // Source window handle and pending drop positions
  Dali::Window  mDragWindow;                                         //    8 bytes
  Dali::Vector2 mPosition;                                           //    8 bytes
  Dali::Vector2 mWindowPosition;                                     //    8 bytes

  // Window IDs (Ecore_X_Window = unsigned int, 4 bytes)
  Ecore_X_Window mSourceWindowId{0};
  Ecore_X_Window mCurrentSourceWindow{0};
  // Saved source window ID from the previous drag session.
  // Used to filter out spurious post-drop XDND_ENTER events that ecore_x
  // sends to our own windows (e.g. shadow) while closing the XDND protocol.
  Ecore_X_Window mPrevSourceWindowId{0};

  // Integer indices and counters (4 bytes)
  int mCurrentMimeCount{0};
  int mTargetIndex{-1};
  int mWindowTargetIndex{-1};
  int mLastDragX{-1};
  int mLastDragY{-1};
}; // class DragAndDropX11

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_DRAG_AND_DROP_X11_H
