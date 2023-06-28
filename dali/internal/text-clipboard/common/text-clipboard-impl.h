#ifndef DALI_INTERNAL_TEXT_CLIPBOARD_H
#define DALI_INTERNAL_TEXT_CLIPBOARD_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/text-clipboard.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Implementation of the Clip Board
 */

class TextClipboard : public Dali::BaseObject
{
public:
  // Hide the specific windowing system
  struct Impl;

  /**
   * @copydoc Dali::TextClipboard::Get()
   */
  static Dali::TextClipboard Get();

  /**
   * @copydoc Dali::TextClipboard::IsAvailable()
   */
  static bool IsAvailable();

  /**
   * Constructor
   * @param[in] impl Some data from a specific windowing system.
   */
  TextClipboard(Impl* impl);

  /**
   * Destructor
   */
  ~TextClipboard() override;

  /**
   * @copydoc Dali::TextClipboard::SetItem()
   */
  bool SetItem(const std::string& itemData);

  /**
   * @copydoc Dali::TextClipboard::RequestItem()
   */
  void RequestItem();

  /**
   * @copydoc Dali::TextClipboard::NumberOfClipboardItems()
   */
  unsigned int NumberOfItems();

  /**
   * @copydoc Dali::TextClipboard::ShowClipboard()
   */
  void ShowClipboard();

  /**
   * @copydoc Dali::TextClipboard::HideClipboard()
   */
  void HideClipboard(bool skipFirstHide);

  /**
  * @copydoc Dali::TextClipboard::IsVisible()
  */
  bool IsVisible() const;

  /**
  * @brief sending buffered data
  *
  * @param[in] event information pointer
  */
  void ExcuteSend(void* event);

  /**
  * @brief receiving buffered data
  *
  * @param[in] event information pointer
  * @param[out] data The buffer pointer for receive data
  * @param[out] length The buffer length for receive data
  */
  void ExcuteReceive(void* event, char*& data, int& length);

private:
  // Undefined
  TextClipboard(const TextClipboard&);
  TextClipboard& operator=(TextClipboard&);

private:
  Impl* mImpl;

public:
}; // class TextClipboard

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::TextClipboard& GetImplementation(Dali::TextClipboard& clipboard)
{
  DALI_ASSERT_ALWAYS(clipboard && "TextClipboard handle is empty");
  BaseObject& handle = clipboard.GetBaseObject();
  return static_cast<Internal::Adaptor::TextClipboard&>(handle);
}

inline static const Internal::Adaptor::TextClipboard& GetImplementation(const Dali::TextClipboard& clipboard)
{
  DALI_ASSERT_ALWAYS(clipboard && "TextClipboard handle is empty");
  const BaseObject& handle = clipboard.GetBaseObject();
  return static_cast<const Internal::Adaptor::TextClipboard&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_CLIPBOARD_H
