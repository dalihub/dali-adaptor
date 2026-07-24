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
#include <dali/public-api/adaptor-framework/drag-data.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/common/dali-vector.h>

#define DALI_ASSERT_VALID_DRAG_DATA(impl) \
  DALI_ASSERT_ALWAYS((impl) && "Cannot use a moved-from DragData object")

namespace Dali
{
struct DragAndDrop::DragData::Impl
{
  struct Entry
  {
    Dali::String mimeType;
    Dali::String data;
  };

  Dali::Vector<Entry> entries;
};

DragAndDrop::DragData::DragData()
: mImpl(MakeUnique<Impl>())
{
}

DragAndDrop::DragData::DragData(const DragData& rhs)
: mImpl(nullptr)
{
  DALI_ASSERT_VALID_DRAG_DATA(rhs.mImpl);
  mImpl = MakeUnique<Impl>(*rhs.mImpl);
}

DragAndDrop::DragData::DragData(DragData&& rhs) noexcept = default;

DragAndDrop::DragData& DragAndDrop::DragData::operator=(const DragData& rhs)
{
  if(this != &rhs)
  {
    DALI_ASSERT_VALID_DRAG_DATA(rhs.mImpl);
    mImpl = MakeUnique<Impl>(*rhs.mImpl);
  }
  return *this;
}

DragAndDrop::DragData& DragAndDrop::DragData::operator=(DragData&& rhs) noexcept = default;

DragAndDrop::DragData::~DragData() = default;

void DragAndDrop::DragData::AddData(const Dali::String& mimeType, const Dali::String& data)
{
  DALI_ASSERT_VALID_DRAG_DATA(mImpl);
  mImpl->entries.PushBack({mimeType, data});
}

uint32_t DragAndDrop::DragData::GetDataCount() const
{
  DALI_ASSERT_VALID_DRAG_DATA(mImpl);
  return mImpl->entries.Count();
}

Dali::String DragAndDrop::DragData::GetMimeType(uint32_t index) const
{
  DALI_ASSERT_VALID_DRAG_DATA(mImpl);
  return index < mImpl->entries.Count() ? mImpl->entries[index].mimeType : Dali::String();
}

Dali::String DragAndDrop::DragData::GetData(uint32_t index) const
{
  DALI_ASSERT_VALID_DRAG_DATA(mImpl);
  return index < mImpl->entries.Count() ? mImpl->entries[index].data : Dali::String();
}

} // namespace Dali

#undef DALI_ASSERT_VALID_DRAG_DATA
