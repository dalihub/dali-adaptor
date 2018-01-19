#ifndef __DALI_INTERNAL_CLIPBOARD_H__
#define __DALI_INTERNAL_CLIPBOARD_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/clipboard.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Implementation of the Clip Board
 */

class Clipboard :  public Dali::BaseObject
{
public:

  // Hide the specific windowing system
  struct Impl;

  /**
   * @copydoc Dali::ClipboardEventNotifier::Get()
   */
  static Dali::Clipboard Get();

  /**
   * Constructor
   * @param[in] impl Some data from a specific windowing system.
   */
  Clipboard(Impl* impl);

  /**
   * Destructor
   */
  virtual ~Clipboard();

  /**
   * @copydoc Dali::Clipboard::SetItem()
   */
  bool SetItem(const std::string &itemData);

  /**
   * @copydoc Dali::Clipboard::RequestItem()
   */
  void RequestItem();

  /**
   * @copydoc Dali::Clipboard::NumberOfClipboardItems()
   */
  unsigned int NumberOfItems();

  /**
   * @copydoc Dali::Clipboard::ShowClipboard()
   */
  void ShowClipboard();

  /**
   * @copydoc Dali::Clipboard::HideClipboard()
   */
  void HideClipboard(bool skipFirstHide);

  /**
  * @copydoc Dali::Clipboard::IsVisible()
  */
  bool IsVisible() const;

  /**
  * @brief exchange either sending or receiving buffered data
  *
  * @param[in] type true for send buffered data, false for receive data to buffer
  * @param[in] event information pointer
  * @return The buffer pointer for send or receive data
  */
  char* ExcuteBuffered( bool type, void *event );

private:

  // Undefined
  Clipboard( const Clipboard& );
  Clipboard& operator=( Clipboard& );

private:

  Impl* mImpl;

public:

}; // class clipboard

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::Clipboard& GetImplementation(Dali::Clipboard& clipboard)
{
  DALI_ASSERT_ALWAYS( clipboard && "Clipboard handle is empty" );
  BaseObject& handle = clipboard.GetBaseObject();
  return static_cast<Internal::Adaptor::Clipboard&>(handle);
}

inline static const  Internal::Adaptor::Clipboard& GetImplementation(const Dali::Clipboard& clipboard)
{
  DALI_ASSERT_ALWAYS( clipboard && "Clipboard handle is empty" );
  const BaseObject& handle = clipboard.GetBaseObject();
  return static_cast<const Internal::Adaptor::Clipboard&>(handle);
}

} // namespace Dali

#endif // __DALI_INTERNAL_CLIPBOARD_H__
