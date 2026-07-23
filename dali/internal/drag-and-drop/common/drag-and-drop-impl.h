#ifndef DALI_INTERNAL_DRAG_AND_DROP_H
#define DALI_INTERNAL_DRAG_AND_DROP_H

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
#include <dali/public-api/object/base-object.h>
#include <functional>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/drag-and-drop.h>
#include <dali/public-api/adaptor-framework/drag-data.h>
#include <dali/public-api/adaptor-framework/drag-event.h>
#include <dali/internal/drag-and-drop/common/drag-event-builder.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Implementation of the DragAndDrop
 */
class DragAndDrop : public Dali::BaseObject
{
public:
  using SourceCallback = std::function<void(Dali::DragAndDrop::SourceEventType)>;
  using DragCallback   = std::function<void(const Dali::DragAndDrop::DragEvent&)>;
  /**
   * Constructor
   */
  DragAndDrop() = default;

  /**
   * Destructor
   */
  ~DragAndDrop() override = default;

  /**
   * @copydoc Dali::DragAndDrop::StartDragAndDrop()
   */
  virtual bool StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& data, SourceCallback callback) = 0;

  /**
   * @copydoc Dali::DragAndDrop::AddListener()
   */
  virtual bool AddListener(Dali::Actor target, const Dali::String& mimeType, DragCallback callback) = 0;

  /**
   * @copydoc Dali::DragAndDrop::RemoveListener()
   */
  virtual bool RemoveListener(Dali::Actor target) = 0;

  /**
   * @copydoc Dali::DragAndDrop::AddListener()
   */
  virtual bool AddListener(Dali::Window window, const Dali::String& mimeType, DragCallback callback) = 0;

  Dali::DragAndDrop::SourceEventSignalType& SourceEventSignal()
  {
    return mSourceEventSignal;
  }

  Dali::DragAndDrop::ActorDragEventSignalType& ActorDragEventSignal()
  {
    return mActorDragEventSignal;
  }

  Dali::DragAndDrop::WindowDragEventSignalType& WindowDragEventSignal()
  {
    return mWindowDragEventSignal;
  }

  /**
   * @copydoc Dali::DragAndDrop::RemoveListener()
   */
  virtual bool RemoveListener(Dali::Window target) = 0;

  /**
   * @copydoc Dali::DragAndDrop::SendData()
   */
  virtual void SendData(void* event) = 0;

  /**
   * @copydoc Dali::DragAndDrop::ReceiveData()
   */
  virtual void ReceiveData(void* event) = 0;

  /**
   * @copydoc Dali::DragAndDrop::CalculateDragEvent()
   */
  virtual bool CalculateDragEvent(void* event) = 0;

  /**
   * @copydoc Dali::DragAndDrop::CalculateViewRegion()
   */
  virtual bool CalculateViewRegion(void* event) = 0;

private:
  Dali::DragAndDrop::SourceEventSignalType     mSourceEventSignal;
  Dali::DragAndDrop::ActorDragEventSignalType  mActorDragEventSignal;
  Dali::DragAndDrop::WindowDragEventSignalType mWindowDragEventSignal;
  DragAndDrop(const DragAndDrop&)      = delete;
  DragAndDrop& operator=(DragAndDrop&) = delete;

}; // class DragAndDrop

extern Dali::DragAndDrop GetDragAndDrop();

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::DragAndDrop& GetImplementation(Dali::DragAndDrop& dnd)
{
  DALI_ASSERT_ALWAYS(dnd && "DragAndDrop handle is empty");
  BaseObject& handle = dnd.GetBaseObject();
  return static_cast<Internal::Adaptor::DragAndDrop&>(handle);
}

inline static const Internal::Adaptor::DragAndDrop& GetImplementation(const Dali::DragAndDrop& dnd)
{
  DALI_ASSERT_ALWAYS(dnd && "DragAndDrop handle is empty");
  const BaseObject& handle = dnd.GetBaseObject();
  return static_cast<const Internal::Adaptor::DragAndDrop&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_DRAG_AND_DROP_H
