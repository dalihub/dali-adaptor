#ifndef DALI_INTERNAL_DRAG_AND_DROP_GENERIC_H
#define DALI_INTERNAL_DRAG_AND_DROP_GENERIC_H

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

namespace Dali
{
class DragAndDrop;

namespace Internal
{
namespace Adaptor
{

/**
 * DragAndDrop Implementation
 */
class DragAndDropGeneric : public Dali::Internal::Adaptor::DragAndDrop
{
public:
  /**
   * Constructor
   */
  DragAndDropGeneric();

  /**
   * Destructor
   */
  ~DragAndDropGeneric() override;

  /**
   * @copydoc Dali::DragAndDrop::StartDragAndDrop()
   */
  bool StartDragAndDrop(Dali::Actor source, Dali::Actor shadow, const Dali::DragAndDrop::DragData& data) override;

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

private:
  DragAndDropGeneric(const DragAndDropGeneric&) = delete;
  DragAndDropGeneric& operator=(DragAndDropGeneric&) = delete;
  DragAndDropGeneric(DragAndDropGeneric&&) = delete;
  DragAndDropGeneric& operator=(DragAndDropGeneric&&) = delete;

public:
}; // class DragAndDropGeneric

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_DRAG_AND_DROP_GENERIC_H
