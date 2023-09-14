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
#include <dali/devel-api/adaptor-framework/drag-and-drop.h>

// INTERNAL INCLUDES
#include <dali/internal/drag-and-drop/common/drag-and-drop-impl.h>

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
  return Internal::Adaptor::GetDragAndDrop();
}

bool DragAndDrop::StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const DragData& dragData, Dali::DragAndDrop::SourceFunction callback)
{
  return GetImplementation(*this).StartDragAndDrop(source, shadowWindow, dragData, callback);
}

bool DragAndDrop::AddListener(Dali::Actor target, DragAndDropFunction callback)
{
  return GetImplementation(*this).AddListener(target, callback);
}

bool DragAndDrop::RemoveListener(Dali::Actor target)
{
  return GetImplementation(*this).RemoveListener(target);
}

bool DragAndDrop::AddListener(Dali::Window target, DragAndDropFunction callback)
{
  return GetImplementation(*this).AddListener(target, callback);
}

bool DragAndDrop::RemoveListener(Dali::Window target)
{
  return GetImplementation(*this).RemoveListener(target);
}

} // namespace Dali
