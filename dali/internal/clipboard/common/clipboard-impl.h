#ifndef DALI_INTERNAL_CLIPBOARD_H
#define DALI_INTERNAL_CLIPBOARD_H

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
#include <dali/devel-api/adaptor-framework/clipboard.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Implementation of the Clipboard
 */
class Clipboard : public Dali::BaseObject, public ConnectionTracker
{
public:
  struct Impl;

  /**
   * @copydoc Dali::Clipboard::Get()
   */
  static Dali::Clipboard Get();

  /**
   * @copydoc Dali::Clipboard::IsAvailable()
   */
  static bool IsAvailable();

  /**
   * Constructor
   * @param[in] impl Clipboard impl.
   */
  Clipboard(Impl* impl);

  /**
   * Destructor
   */
  ~Clipboard() override;

  /**
   * @copydoc Dali::Clipboard::DataSentSignal()
   */
  Dali::Clipboard::DataSentSignalType& DataSentSignal();

  /**
   * @copydoc Dali::Clipboard::DataReceivedSignal()
   */
  Dali::Clipboard::DataReceivedSignalType& DataReceivedSignal();

  /**
   * @copydoc Dali::Clipboard::SetData()
   */
  bool SetData(const Dali::Clipboard::ClipData& clipData);

  /**
   * @copydoc Dali::Clipboard::GetData()
   */
  uint32_t GetData(const std::string &mimeType);

  /**
   * @copydoc Dali::Clipboard::NumberOfClipboardItems()
   */
  size_t NumberOfItems();

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
   * This is called after a timeout when no new data event is received for a certain period of time on X.
   * @return will return false; one-shot timer.
   */
  bool OnReceiveData();

private:
  // Undefined
  Clipboard(const Clipboard&);
  Clipboard& operator=(Clipboard&);

private:
  Impl* mImpl;

public:
}; // class Clipboard

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::Clipboard& GetImplementation(Dali::Clipboard& clipboard)
{
  DALI_ASSERT_ALWAYS(clipboard && "Clipboard handle is empty");
  BaseObject& handle = clipboard.GetBaseObject();
  return static_cast<Internal::Adaptor::Clipboard&>(handle);
}

inline static const Internal::Adaptor::Clipboard& GetImplementation(const Dali::Clipboard& clipboard)
{
  DALI_ASSERT_ALWAYS(clipboard && "Clipboard handle is empty");
  const BaseObject& handle = clipboard.GetBaseObject();
  return static_cast<const Internal::Adaptor::Clipboard&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_CLIPBOARD_H
