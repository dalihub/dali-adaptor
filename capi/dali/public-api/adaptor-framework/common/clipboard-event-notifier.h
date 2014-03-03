#ifndef __DALI_CLIPBOARD_EVENT_NOTIFIER_H__
#define __DALI_CLIPBOARD_EVENT_NOTIFIER_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @addtogroup CAPI_DALI_FRAMEWORK
 * @{
 */

// EXTERNAL INCLUDES
#include <boost/function.hpp>

#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal-v2.h>

namespace Dali DALI_IMPORT_API
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class ClipboardEventNotifier;
}
}

/**
 * The ClipboardEventNotifier provides signals when clipboard events are received from the device.
 */
class ClipboardEventNotifier : public BaseHandle
{
public:

  // Typedefs

  // Clipboard event
  typedef SignalV2< void ( ClipboardEventNotifier& ) > ClipboardEventSignalV2;

  // Signal Names
  static const char* const SIGNAL_CONTENT_SELECTED;

  /**
   * Create an uninitialized handle.
   * This can be initialized by getting the notifier from Dali::Adaptor.
   */
  ClipboardEventNotifier();

  /**
   * Retrieve a handle to the ClipboardEventNotifier instance.
   * @return A handle to the ClipboardEventNotifier
   */
  static ClipboardEventNotifier Get();

  /**
   * Virtual Destructor.
   */
  virtual ~ClipboardEventNotifier();

  /**
   * Returns the selected content.
   * @return A reference to the string representing the selected content.
   */
  const std::string& GetContent() const;

  /**
   * Sets the selected content.
   * @param[in] content  A string that represents the content that has been selected.
   */
  void SetContent( const std::string& content );

  /**
   * Clears the stored content.
   */
  void ClearContent();

  /**
   * Called when content is selected in the clipboard.
   */
  void EmitContentSelectedSignal();

public:  // Signals

  /**
   * This is emitted when content is selected from the clipboard
   * A callback of the following type may be connected:
   * @code
   *   void YourCallback( ClipboardEventNotifier& notifier );
   * @endcode
   * @return The signal to connect to.
   */
  ClipboardEventSignalV2& ContentSelectedSignal();

public: // Not intended for application developers

  /**
   * This constructor is used by ClipboardEventNotifier::Get().
   * @param[in] notifier A pointer to the drag and drop notifier.
   */
  ClipboardEventNotifier( Internal::Adaptor::ClipboardEventNotifier* notifier );
};

} // namespace Dali

/**
 * @}
 */
#endif // __DALI_CLIPBOARD_EVENT_NOTIFIER_H__
