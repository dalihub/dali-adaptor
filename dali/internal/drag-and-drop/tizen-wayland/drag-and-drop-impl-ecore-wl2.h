#ifndef DALI_INTERNAL_DRAG_AND_DROP_ECORE_WL2_H
#define DALI_INTERNAL_DRAG_AND_DROP_ECORE_WL2_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/internal/adaptor/tizen-wayland/dali-ecore-wl2.h>
#include <dali/internal/drag-and-drop/common/drag-and-drop-impl.h>
#include <dali/internal/system/linux/dali-ecore.h>
#include <map>

namespace Dali
{
class DragAndDrop;

namespace Internal
{
namespace Adaptor
{

struct DelayedWritingData
{
  Eina_Rw_Slice slice;
  unsigned int  writtenBytes;
};

struct DropTarget
{
  Dali::Actor                            target;
  std::string                            mimeType;
  Dali::DragAndDrop::DragAndDropFunction callback;
  bool                                   inside;
  int                                    parentWindowId;
};

struct DropWindowTarget
{
  Dali::Window                           target;
  std::string                            mimeType;
  Dali::DragAndDrop::DragAndDropFunction callback;
  bool                                   inside;
  int                                    windowId;
};

/**
 * DragAndDrop Implementation
 */
class DragAndDropEcoreWl : public Dali::Internal::Adaptor::DragAndDrop,
                           public Dali::ConnectionTracker
{
public:
  /**
   * Constructor
   */
  DragAndDropEcoreWl();

  /**
   * Destructor
   */
  ~DragAndDropEcoreWl() override;

  /**
   * @copydoc Dali::DragAndDrop::StartDragAndDrop()
   */
  bool StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& data, Dali::DragAndDrop::SourceFunction callback) override;

  /**
   * @copydoc Dali::DragAndDrop::AddListener()
   */
  bool AddListener(Dali::Actor target, char* mimeType, Dali::DragAndDrop::DragAndDropFunction callback) override;

  /**
   * @copydoc Dali::DragAndDrop::AddListener()
   */
  bool AddListener(Dali::Window target, char* mimeType, Dali::DragAndDrop::DragAndDropFunction callback) override;

  /**
   * @copydoc Dali::DragAndDrop::RemoveListener()
   */
  bool RemoveListener(Dali::Actor target) override;

  /**
   * @copydoc Dali::DragAndDrop::RemoveListener()
   */
  bool RemoveListener(Dali::Window target) override;

  /**
   * @copydoc Dali::DragAndDrop::SendData()
   */
  void SendData(void* event) override;

  /**
   * @copydoc Dali::DragAndDrop::ReceiveData()
   */
  void ReceiveData(void* event) override;

  /**
   * @copydoc Dali::DragAndDrop::CalculateDragEvent()
   */
  bool CalculateDragEvent(void* event) override;

  /**
   * @copydoc Dali::DragAndDrop::CalculateViewRegion()
   */
  bool CalculateViewRegion(void* event) override;

  /**
   * @brief Trigger drag event for target.
   */
  void TriggerDragEventForTarget(int targetIndex, Ecore_Wl2_Event_Dnd_Motion* event, Eina_Array* mimes, Dali::DragAndDrop::DragEvent& dragEvent);

  /**
   * @brief Trigger drag event for window target.
   */
  void TriggerDragEventForWindowTarget(int targetIndex, Ecore_Wl2_Event_Dnd_Motion* event, Eina_Array* mimes, Dali::DragAndDrop::DragEvent& dragEvent);

  /**
   * @brief Process drag events for targets.
   */
  void ProcessDragEventsForTargets(Ecore_Wl2_Event_Dnd_Motion* event, Dali::DragAndDrop::DragEvent& dragEvent, Eina_Array* mimes);

  /**
   * @brief Process drag events for window targets.
   */
  void ProcessDragEventsForWindowTargets(Ecore_Wl2_Event_Dnd_Motion* event, Dali::DragAndDrop::DragEvent& dragEvent, Eina_Array* mimes);

  /**
   * @brief Process drop events for targets.
   */
  bool ProcessDropEventsForTargets(Ecore_Wl2_Event_Dnd_Drop* event, Eina_Array* mimes);

  /**
   * @brief Process drop events for window targets.
   */
  bool ProcessDropEventsForWindowTargets(Ecore_Wl2_Event_Dnd_Drop* event, Eina_Array* mimes);

  /**
   * @brief Call drag source events.
   */
  void CallSourceEvent(Dali::DragAndDrop::SourceEventType type);

  /**
   * @brief Reset drop targets.
   */
  void ResetDropTargets();

private:
  /**
   * @brief Callback function to check whether listening target is scene on.
   * Note that This signal will be called only if AddListener target is not scene on.
   */
  void DropTargetSceneOn(Dali::Actor target);

  /**
   * @brief Recalculate cursor position by orientation.
   *
   * @param[in] x The x value of given position
   * @param[in] y The y value of given position
   * @param[in] window The window instance
   * @return The recalculated position by orientation
   */
  Vector2 RecalculatePositionByOrientation(int x, int y, Dali::Window window);

private:
  DragAndDropEcoreWl(const DragAndDropEcoreWl&)       = delete;
  DragAndDropEcoreWl& operator=(DragAndDropEcoreWl&)  = delete;
  DragAndDropEcoreWl(DragAndDropEcoreWl&&)            = delete;
  DragAndDropEcoreWl& operator=(DragAndDropEcoreWl&&) = delete;

private:
  Dali::Window                       mDragWindow;
  uint32_t                           mSerial{std::numeric_limits<uint32_t>::max()};
  Ecore_Event_Handler*               mSendHandler{nullptr};
  Ecore_Event_Handler*               mSourceEndHandler{nullptr};
  Ecore_Event_Handler*               mSourceDropHandler{nullptr};
  Ecore_Event_Handler*               mReceiveHandler{nullptr};
  Ecore_Event_Handler*               mMotionHandler{nullptr};
  Ecore_Event_Handler*               mDropHandler{nullptr};
  Ecore_Event_Handler*               mEnterHandler{nullptr};
  Ecore_Event_Handler*               mLeaveHandler{nullptr};
  int                                mTargetIndex{-1};
  int                                mWindowTargetIndex{-1};
  Dali::Vector2                      mPosition;
  Dali::Vector2                      mWindowPosition;
  Dali::DragAndDrop::SourceFunction  mSourceCallback{nullptr};
  std::vector<DropTarget>            mDropTargets;
  std::vector<DropWindowTarget>      mDropWindowTargets;
  std::map<std::string, std::string> mDataMap;
}; // class DragAndDropEcoreWl

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_DRAG_AND_DROP_ECORE_WL2_H
