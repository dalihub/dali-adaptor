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
#include <dali/internal/drag-and-drop/generic/drag-and-drop-impl-generic.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

Dali::DragAndDrop GetDragAndDrop()
{
  Dali::DragAndDrop dnd;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::DragAndDrop));
    if(handle)
    {
      // If so, downcast the handle
      dnd = Dali::DragAndDrop(dynamic_cast<DragAndDrop*>(handle.GetObjectPtr()));
    }
    else
    {
      // Create a singleon instance
      dnd = Dali::DragAndDrop(new DragAndDropGeneric());
      service.Register(typeid(Dali::DragAndDrop), dnd);
    }
  }

  return dnd;
}

DragAndDropGeneric::DragAndDropGeneric()
{
}

DragAndDropGeneric::~DragAndDropGeneric()
{
}

bool DragAndDropGeneric::StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& dragData, Dali::DragAndDrop::SourceFunction callback)
{
  return true;
}

bool DragAndDropGeneric::AddListener(Dali::Actor target, char* miemType, Dali::DragAndDrop::DragAndDropFunction callback)
{
  return true;
}

bool DragAndDropGeneric::RemoveListener(Dali::Actor target)
{
  return true;
}

bool DragAndDropGeneric::AddListener(Dali::Window target, char* miemType, Dali::DragAndDrop::DragAndDropFunction callback)
{
  return true;
}

bool DragAndDropGeneric::RemoveListener(Dali::Window target)
{
  return true;
}

void DragAndDropGeneric::SendData(void* event)
{
  return;
}

void DragAndDropGeneric::ReceiveData(void* event)
{
  return;
}

bool DragAndDropGeneric::CalculateDragEvent(void* event)
{
  return true;
}

bool DragAndDropGeneric::CalculateViewRegion(void* event)
{
  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
