#ifndef DALI_INTERNAL_DRAG_AND_DROP_ECORE_WL2_H
#define DALI_INTERNAL_DRAG_AND_DROP_ECORE_WL2_H

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

// INTERNAL INCLUDES
#include <dali/internal/drag-and-drop/common/drag-and-drop-impl.h>
#include <dali/internal/system/linux/dali-ecore.h>

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
  Dali::DragAndDrop::DragAndDropFunction callback;
  bool                                   inside;
};

/**
 * DragAndDrop Implementation
 */
class DragAndDropEcoreWl : public Dali::Internal::Adaptor::DragAndDrop
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
  bool AddListener(Dali::Actor target, Dali::DragAndDrop::DragAndDropFunction callback) override;

  /**
   * @copydoc Dali::DragAndDrop::RemoveListener()
   */
  bool RemoveListener(Dali::Actor target) override;

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
   * @brief Call drag source events.
   */
  void CallSourceEvent(Dali::DragAndDrop::SourceEventType type);

private:
  DragAndDropEcoreWl(const DragAndDropEcoreWl&) = delete;
  DragAndDropEcoreWl& operator=(DragAndDropEcoreWl&) = delete;
  DragAndDropEcoreWl(DragAndDropEcoreWl&&)           = delete;
  DragAndDropEcoreWl& operator=(DragAndDropEcoreWl&&) = delete;

private:
  Dali::Window                      mDragWindow;
  uint32_t                          mSerial{0u};
  Ecore_Event_Handler*              mSendHandler{nullptr};
  Ecore_Event_Handler*              mSourceEndHandler{nullptr};
  Ecore_Event_Handler*              mSourceDropHandler{nullptr};
  Ecore_Event_Handler*              mReceiveHandler{nullptr};
  Ecore_Event_Handler*              mMotionHandler{nullptr};
  Ecore_Event_Handler*              mDropHandler{nullptr};
  Ecore_Event_Handler*              mEnterHandler{nullptr};
  int                               mTargetIndex{0};
  std::string                       mMimeType;
  std::string                       mData;
  int                               mDataSize{0};
  Dali::Vector2                     mPosition;
  Dali::DragAndDrop::SourceFunction mSourceCallback{nullptr};
  std::vector<DropTarget> mDropTargets;
}; // class DragAndDropEcoreWl

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_DRAG_AND_DROP_ECORE_WL2_H
