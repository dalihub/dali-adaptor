#ifndef DALI_DRAG_DATA_H
#define DALI_DRAG_DATA_H

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/adaptor-framework/drag-and-drop.h>

namespace Dali
{
/**
 * @brief Describes the MIME types and data offered by a drag source.
 *
 * DragData owns the MIME type and data strings added to it.
 *
 * @SINCE_2_5.32
 */
class DALI_ADAPTOR_API DragAndDrop::DragData
{
public:
  /**
   * @brief Creates an empty DragData object.
   *
   * @SINCE_2_5.32
   */
  DragData();

  /**
   * @brief Copies a DragData object.
   *
   * @SINCE_2_5.32
   */
  DragData(const DragData& rhs);

  /**
   * @brief Moves a DragData object.
   *
   * @SINCE_2_5.32
   */
  DragData(DragData&& rhs) noexcept;

  /**
   * @brief Copies the contents of another DragData object.
   *
   * @SINCE_2_5.32
   */
  DragData& operator=(const DragData& rhs);

  /**
   * @brief Moves the contents of another DragData object.
   *
   * @SINCE_2_5.32
   */
  DragData& operator=(DragData&& rhs) noexcept;

  /**
   * @brief Destructor.
   *
   * @SINCE_2_5.32
   */
  ~DragData();

  /**
   * @brief Adds a MIME type and its corresponding data.
   *
   * @SINCE_2_5.32
   */
  void AddData(const Dali::String& mimeType, const Dali::String& data);

  /**
   * @brief Gets the number of MIME type/data entries.
   *
   * @SINCE_2_5.32
   */
  uint32_t GetDataCount() const;

  /**
   * @brief Gets a MIME type, or an empty string for an invalid index.
   *
   * @SINCE_2_5.32
   */
  Dali::String GetMimeType(uint32_t index) const;

  /**
   * @brief Gets data, or an empty string for an invalid index.
   *
   * @SINCE_2_5.32
   */
  Dali::String GetData(uint32_t index) const;

private:
  struct Impl;
  UniquePtr<Impl> mImpl;
};

} // namespace Dali

#endif // DALI_DRAG_DATA_H
