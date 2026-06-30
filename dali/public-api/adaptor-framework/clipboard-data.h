#ifndef DALI_CLIPBOARD_DATA_H
#define DALI_CLIPBOARD_DATA_H

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

namespace Dali
{
/**
 * @brief ClipboardData stores clipboard MIME type and content.
 *
 * ClipboardData is used with Dali::Clipboard to set or receive clipboard
 * content. The MIME type describes the format of the content.
 *
 * @SINCE_2_5.28
 */
class DALI_ADAPTOR_API ClipboardData
{
public:
  /**
   * @brief Creates an empty ClipboardData object.
   *
   * @SINCE_2_5.28
   */
  ClipboardData();

  /**
   * @brief Creates a ClipboardData object with MIME type and content.
   *
   * @param[in] mimeType The MIME type of the clipboard content.
   * @param[in] content The clipboard content.
   * @SINCE_2_5.28
   */
  ClipboardData(const Dali::String& mimeType, const Dali::String& content);

  /**
   * @brief Copy constructor.
   *
   * @param[in] rhs ClipboardData to copy.
   * @SINCE_2_5.28
   */
  ClipboardData(const ClipboardData& rhs);

  /**
   * @brief Move constructor.
   *
   * @param[in] rhs ClipboardData to move.
   * @SINCE_2_5.28
   */
  ClipboardData(ClipboardData&& rhs) noexcept;

  /**
   * @brief Copy assignment operator.
   *
   * @param[in] rhs ClipboardData to copy.
   * @return Reference to this object.
   * @SINCE_2_5.28
   */
  ClipboardData& operator=(const ClipboardData& rhs);

  /**
   * @brief Move assignment operator.
   *
   * @param[in] rhs ClipboardData to move.
   * @return Reference to this object.
   * @SINCE_2_5.28
   */
  ClipboardData& operator=(ClipboardData&& rhs) noexcept;

  /**
   * @brief Destructor.
   *
   * @SINCE_2_5.28
   */
  ~ClipboardData();

  /**
   * @brief Sets the MIME type of the clipboard content.
   *
   * @param[in] mimeType The MIME type.
   * @SINCE_2_5.28
   */
  void SetMimeType(const Dali::String& mimeType);

  /**
   * @brief Gets the MIME type of the clipboard content.
   *
   * @return The MIME type.
   * @SINCE_2_5.28
   */
  Dali::String GetMimeType() const;

  /**
   * @brief Sets the clipboard content.
   *
   * @param[in] content The clipboard content.
   * @SINCE_2_5.28
   */
  void SetContent(const Dali::String& content);

  /**
   * @brief Gets the clipboard content.
   *
   * @return The clipboard content.
   * @SINCE_2_5.28
   */
  Dali::String GetContent() const;

private:
  struct Impl;
  UniquePtr<Impl> mImpl;
};

} // namespace Dali

#endif // DALI_CLIPBOARD_DATA_H
