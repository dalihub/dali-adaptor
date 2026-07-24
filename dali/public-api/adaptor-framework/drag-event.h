#ifndef DALI_DRAG_EVENT_H
#define DALI_DRAG_EVENT_H

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
#include <dali/public-api/common/dali-string.h>
#include <dali/public-api/common/unique-ptr.h>
#include <dali/public-api/math/vector2.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/adaptor-framework/drag-and-drop.h>

namespace Dali
{
/**
 * @brief Describes an event delivered to a drop target.
 *
 * DragEvent owns the MIME types and dropped data supplied by the platform.
 * When supplied through a drag event signal, the reference is valid only for
 * the duration of the signal callback. Copy the event to retain it.
 *
 * @SINCE_2_5.32
 */
class DALI_ADAPTOR_API DragAndDrop::DragEvent
{
public:
  /**
   * @brief Copies a DragEvent object.
   *
   * @SINCE_2_5.32
   */
  DragEvent(const DragEvent& rhs);

  /**
   * @brief Moves a DragEvent object.
   *
   * @SINCE_2_5.32
   */
  DragEvent(DragEvent&& rhs) noexcept;

  /**
   * @brief Copies the contents of another DragEvent object.
   *
   * @SINCE_2_5.32
   */
  DragEvent& operator=(const DragEvent& rhs);

  /**
   * @brief Moves the contents of another DragEvent object.
   *
   * @SINCE_2_5.32
   */
  DragEvent& operator=(DragEvent&& rhs) noexcept;

  /**
   * @brief Destructor.
   *
   * @SINCE_2_5.32
   */
  ~DragEvent();

  /**
   * @brief Gets the drag event type.
   *
   * @SINCE_2_5.32
   */
  DragType GetDragType() const;

  /**
   * @brief Gets the target-relative drag position.
   *
   * @SINCE_2_5.32
   */
  Dali::Vector2 GetPosition() const;

  /**
   * @brief Gets the number of offered MIME types.
   *
   * @SINCE_2_5.32
   */
  uint32_t GetMimeTypeCount() const;

  /**
   * @brief Gets an offered MIME type, or an empty string for an invalid index.
   *
   * @SINCE_2_5.32
   */
  Dali::String GetMimeType(uint32_t index) const;

  /**
   * @brief Gets the dropped data.
   *
   * @SINCE_2_5.32
   */
  Dali::String GetData() const;

private:
  friend class Internal::Adaptor::DragEventBuilder;

  DragEvent();
  DragEvent(DragType type, Dali::Vector2 position);

  void SetAction(DragType type);
  void SetPosition(Dali::Vector2 position);
  void AddMimeType(const Dali::String& mimeType);
  void SetData(const Dali::String& data);

  struct Impl;
  UniquePtr<Impl> mImpl;
};

} // namespace Dali

#endif // DALI_DRAG_EVENT_H
