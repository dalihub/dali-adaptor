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
#include <dali/public-api/adaptor-framework/drag-and-drop.h>
#include <dali/public-api/adaptor-framework/drag-data.h>
#include <dali/public-api/adaptor-framework/drag-event.h>

// INTERNAL INCLUDES
#include <dali/internal/drag-and-drop/common/drag-and-drop-impl.h>
#include <dali/internal/drag-and-drop/common/drag-and-drop-factory.h>

namespace Dali
{
DragAndDrop::DragAndDrop()
{
}
DragAndDrop::~DragAndDrop()
{
}
DragAndDrop::DragAndDrop(Internal::Adaptor::DragAndDrop* impl)
: BaseHandle(impl)
{
}

DragAndDrop DragAndDrop::Get()
{
  // Delegate creation to the platform-specific DragAndDropFactory.
  return Internal::Adaptor::GetDragAndDropFactory()->CreateDragAndDrop();
}

bool DragAndDrop::StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const DragData& dragData)
{
  if(!source || !shadowWindow || dragData.GetDataCount() == 0u)
  {
    return false;
  }

  for(uint32_t index = 0u; index < dragData.GetDataCount(); ++index)
  {
    if(dragData.GetMimeType(index).Empty())
    {
      return false;
    }
  }

  auto& implementation = GetImplementation(*this);
  return implementation.StartDragAndDrop(source, shadowWindow, dragData, [&implementation, source](SourceEventType type) {
    implementation.SourceEventSignal().Emit(DragAndDrop(&implementation), source, type);
  });
}

bool DragAndDrop::AddListener(Dali::Actor target, const Dali::String& mimeType)
{
  if(!target || mimeType.Empty())
  {
    return false;
  }

  auto& implementation = GetImplementation(*this);
  return implementation.AddListener(target, mimeType, [&implementation, target](const DragEvent& event) {
    implementation.ActorDragEventSignal().Emit(DragAndDrop(&implementation), target, event);
  });
}

bool DragAndDrop::RemoveListener(Dali::Actor target)
{
  return GetImplementation(*this).RemoveListener(target);
}

bool DragAndDrop::AddListener(Dali::Window target, const Dali::String& mimeType)
{
  if(!target || mimeType.Empty())
  {
    return false;
  }

  auto& implementation = GetImplementation(*this);
  return implementation.AddListener(target, mimeType, [&implementation, target](const DragEvent& event) {
    implementation.WindowDragEventSignal().Emit(DragAndDrop(&implementation), target, event);
  });
}

bool DragAndDrop::RemoveListener(Dali::Window target)
{
  return GetImplementation(*this).RemoveListener(target);
}

DragAndDrop::SourceEventSignalType& DragAndDrop::SourceEventSignal()
{
  return GetImplementation(*this).SourceEventSignal();
}

DragAndDrop::ActorDragEventSignalType& DragAndDrop::ActorDragEventSignal()
{
  return GetImplementation(*this).ActorDragEventSignal();
}

DragAndDrop::WindowDragEventSignalType& DragAndDrop::WindowDragEventSignal()
{
  return GetImplementation(*this).WindowDragEventSignal();
}

} // namespace Dali
