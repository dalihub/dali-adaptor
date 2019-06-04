#ifndef  DALI_CLIPBOARD_H
#define  DALI_CLIPBOARD_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace Internal DALI_INTERNAL
{

namespace Adaptor
{
class Clipboard;
}
}

/**
 * @brief Interface to the device's clipboard.
 *
 * Clipboard can manage it's item and set show / hide status.
 */

class DALI_ADAPTOR_API Clipboard : public BaseHandle
{
public:
  /**
   * @brief Create an uninitialized Clipboard.
   *
   * this can be initialized with one of the derived Clipboard' New() methods
   */
  Clipboard();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~Clipboard();

  /**
   * @brief This constructor is used by Adaptor::GetClipboard().
   *
   * @param[in] clipboard A pointer to the clipboard.
   */
  explicit DALI_INTERNAL Clipboard( Internal::Adaptor::Clipboard* clipboard );

  /**
   * @brief Retrieve a handle to the ClipboardEventNotifier instance.
   *
   * @return A handle to the Clipboard
   */
  static Clipboard Get();

  /**
   * @brief Send the given string to the clipboard.
   *
   * @param[in] itemData string to send to clip board
   * @return bool true if the internal clip board sending was successful.
   */
  bool SetItem( const std::string& itemData );

  /**
   * @brief Request clipboard service to retrieve an item
   *
   * Calling this method will trigger a signal from the clipboard event notifier.
   * @see Dali::ClipboardEventNotifier::ContentSelectedSignal()
   */
  void RequestItem();

  /**
   * @brief Returns the number of item currently in the clipboard.
   *
   * @return unsigned int number of clipboard items
   */
  unsigned int NumberOfItems();

  /**
   * @brief Show the clipboard window.
   */
  void ShowClipboard();

  /**
   * @brief Hide the clipboard window.
   */
  void HideClipboard();

  /**
  * @brief Retrieves the clipboard's visibility
  * @return bool true if the clipboard is visible.
  */
  bool IsVisible() const;

};
} // namespace Dali

#endif // DALI_CLIPBOARD_H
