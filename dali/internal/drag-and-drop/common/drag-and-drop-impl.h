#ifndef DALI_INTERNAL_DRAG_AND_DROP_H
#define DALI_INTERNAL_DRAG_AND_DROP_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/adaptor-framework/drag-and-drop.h>

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
  virtual bool StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& data, Dali::DragAndDrop::SourceFunction callback) = 0;

  /**
   * @copydoc Dali::DragAndDrop::AddListener()
   */
  virtual bool AddListener(Dali::Actor target, char* mimeType, Dali::DragAndDrop::DragAndDropFunction callback) = 0;

  /**
   * @copydoc Dali::DragAndDrop::RemoveListener()
   */
  virtual bool RemoveListener(Dali::Actor target) = 0;

  /**
   * @copydoc Dali::DragAndDrop::AddListener()
   */
  virtual bool AddListener(Dali::Window window, char* mimeType, Dali::DragAndDrop::DragAndDropFunction callback) = 0;

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
  DragAndDrop(const DragAndDrop&) = delete;
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
