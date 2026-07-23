#ifndef DALI_INTERNAL_DRAG_EVENT_BUILDER_H
#define DALI_INTERNAL_DRAG_EVENT_BUILDER_H

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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/drag-event.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Builds a DragEvent from platform event data.
 *
 * This class is the only writer of DragEvent state. Public consumers receive
 * a read-only DragEvent through DragAndDrop signals.
 */
class DragEventBuilder
{
public:
  DragEventBuilder() = default;

  DragEventBuilder(Dali::DragAndDrop::DragType type, Dali::Vector2 position)
  : mEvent(type, position)
  {
  }

  void SetAction(Dali::DragAndDrop::DragType type)
  {
    mEvent.SetAction(type);
  }

  void SetPosition(Dali::Vector2 position)
  {
    mEvent.SetPosition(position);
  }

  void AddMimeType(const Dali::String& mimeType)
  {
    mEvent.AddMimeType(mimeType);
  }

  void SetData(const Dali::String& data)
  {
    mEvent.SetData(data);
  }

  operator Dali::DragAndDrop::DragEvent&()
  {
    return mEvent;
  }

  operator const Dali::DragAndDrop::DragEvent&() const
  {
    return mEvent;
  }

private:
  Dali::DragAndDrop::DragEvent mEvent;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_DRAG_EVENT_BUILDER_H
