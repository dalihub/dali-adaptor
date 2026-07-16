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

// CLASS HEADER
#include <dali/public-api/adaptor-framework/drag-event.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/common/dali-vector.h>

#define DALI_ASSERT_VALID_DRAG_EVENT(impl) \
  DALI_ASSERT_ALWAYS((impl) && "Cannot use a moved-from DragEvent object")

namespace Dali
{
struct DragAndDrop::DragEvent::Impl
{
  DragType                   type{DragType::DROP};
  Dali::Vector2              position;
  Dali::Vector<Dali::String> mimeTypes;
  Dali::String               data;
};

DragAndDrop::DragEvent::DragEvent()
: mImpl(MakeUnique<Impl>())
{
}

DragAndDrop::DragEvent::DragEvent(DragType type, Dali::Vector2 position)
: mImpl(MakeUnique<Impl>())
{
  mImpl->type     = type;
  mImpl->position = position;
}

DragAndDrop::DragEvent::DragEvent(const DragEvent& rhs)
: mImpl(nullptr)
{
  DALI_ASSERT_VALID_DRAG_EVENT(rhs.mImpl);
  mImpl = MakeUnique<Impl>(*rhs.mImpl);
}

DragAndDrop::DragEvent::DragEvent(DragEvent&& rhs) noexcept = default;

DragAndDrop::DragEvent& DragAndDrop::DragEvent::operator=(const DragEvent& rhs)
{
  if(this != &rhs)
  {
    DALI_ASSERT_VALID_DRAG_EVENT(rhs.mImpl);
    mImpl = MakeUnique<Impl>(*rhs.mImpl);
  }
  return *this;
}

DragAndDrop::DragEvent& DragAndDrop::DragEvent::operator=(DragEvent&& rhs) noexcept = default;

DragAndDrop::DragEvent::~DragEvent() = default;

void DragAndDrop::DragEvent::SetAction(DragType type)
{
  DALI_ASSERT_VALID_DRAG_EVENT(mImpl);
  mImpl->type = type;
}

DragAndDrop::DragType DragAndDrop::DragEvent::GetDragType() const
{
  DALI_ASSERT_VALID_DRAG_EVENT(mImpl);
  return mImpl->type;
}

void DragAndDrop::DragEvent::SetPosition(Dali::Vector2 position)
{
  DALI_ASSERT_VALID_DRAG_EVENT(mImpl);
  mImpl->position = position;
}

Dali::Vector2 DragAndDrop::DragEvent::GetPosition() const
{
  DALI_ASSERT_VALID_DRAG_EVENT(mImpl);
  return mImpl->position;
}

void DragAndDrop::DragEvent::AddMimeType(const Dali::String& mimeType)
{
  DALI_ASSERT_VALID_DRAG_EVENT(mImpl);
  mImpl->mimeTypes.PushBack(mimeType);
}

uint32_t DragAndDrop::DragEvent::GetMimeTypeCount() const
{
  DALI_ASSERT_VALID_DRAG_EVENT(mImpl);
  return mImpl->mimeTypes.Count();
}

Dali::String DragAndDrop::DragEvent::GetMimeType(uint32_t index) const
{
  DALI_ASSERT_VALID_DRAG_EVENT(mImpl);
  return index < mImpl->mimeTypes.Count() ? mImpl->mimeTypes[index] : Dali::String();
}

void DragAndDrop::DragEvent::SetData(const Dali::String& data)
{
  DALI_ASSERT_VALID_DRAG_EVENT(mImpl);
  mImpl->data = data;
}

Dali::String DragAndDrop::DragEvent::GetData() const
{
  DALI_ASSERT_VALID_DRAG_EVENT(mImpl);
  return mImpl->data;
}

} // namespace Dali

#undef DALI_ASSERT_VALID_DRAG_EVENT
