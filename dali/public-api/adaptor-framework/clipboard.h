#ifndef DALI_CLIPBOARD_H
#define DALI_CLIPBOARD_H

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
#include <type_traits>
#include <dali/public-api/common/dali-string.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/callback.h>
#include <dali/public-api/signals/connection-tracker-interface.h>
#include <dali/public-api/signals/dali-signal.h>
#include <dali/public-api/signals/slot-delegate.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/clipboard-data.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class Clipboard;
}
} // namespace Internal DALI_INTERNAL

/**
 * @brief Interface to the device clipboard.
 *
 * Clipboard supports setting and requesting clipboard content.
 *
 * @SINCE_2_5.28
 */
class DALI_ADAPTOR_API Clipboard : public BaseHandle
{
public:
  /**
   * @brief Signal type emitted when clipboard data is available to request.
   *
   * @SINCE_2_5.28
   */
  using DataOfferedSignalType = Signal<void(const Dali::String&)>;

  /**
   * @brief Creates an uninitialized Clipboard handle.
   *
   * @SINCE_2_5.28
   */
  Clipboard();

  /**
   * @brief Destructor.
   *
   * @SINCE_2_5.28
   */
  ~Clipboard();

  /**
   * @brief Retrieves a handle to the Clipboard singleton.
   *
   * @return A handle to the Clipboard.
   * @SINCE_2_5.28
   */
  static Clipboard Get();

  /**
   * @brief Sets clipboard data.
   *
   * @param[in] data Clipboard data containing MIME type and content.
   * @return true if the clipboard set request was accepted, false otherwise.
   * @SINCE_2_5.28
   */
  bool SetData(const ClipboardData& data);

  /**
   * @brief Checks whether clipboard data with the given MIME type is available.
   *
   * @param[in] mimeType MIME type to check.
   * @return true if data with the MIME type is available, false otherwise.
   * @SINCE_2_5.28
   */
  bool HasType(const Dali::String& mimeType);

  /**
   * @brief Requests clipboard data with the given MIME type.
   *
   * The callback must have the following signature:
   *
   *   void OnClipboardDataReceived(bool succeeded, const Dali::ClipboardData& data);
   *
   * The callback is invoked at most once and never before this method returns.
   * Delivery is deferred until a later event-processing cycle.
   *
   * If the request fails to start or times out while the adaptor is running,
   * the callback is invoked with @a succeeded set to false and @a data set to
   * an empty Dali::ClipboardData.
   *
   * If the receiver is destroyed or the callback is disconnected before delivery,
   * the callback will not be invoked.
   *
   * Each request is independent. Completion order may differ from request order.
   * The @a data reference is valid only during callback invocation; make a copy
   * if you need to retain the data.
   *
   * @tparam Receiver The callback receiver type. It must inherit
   *   Dali::ConnectionTrackerInterface, typically via Dali::ConnectionTracker.
   * @param[in] mimeType MIME type to request. If empty, callback is invoked with false.
   * @param[in] receiver The object that owns the callback.
   * @param[in] callback Member function pointer for the callback.
   * @SINCE_2_5.28
   */
  template<class Receiver>
  void GetData(const Dali::String& mimeType,
               Receiver* receiver,
               void (Receiver::*callback)(bool, const Dali::ClipboardData&))
  {
    static_assert(std::is_base_of<Dali::ConnectionTrackerInterface, Receiver>::value,
                  "Clipboard::GetData receiver must inherit Dali::ConnectionTrackerInterface");

    DALI_ASSERT_ALWAYS(receiver && "Clipboard::GetData receiver is null");
    DALI_ASSERT_ALWAYS(callback && "Clipboard::GetData callback is null");

    DoGetData(mimeType,
              static_cast<Dali::ConnectionTrackerInterface*>(receiver),
              MakeCallback(receiver, callback));
  }

  /**
   * @brief Requests clipboard data using a SlotDelegate callback.
   *
   * This overload follows the same callback signature and delivery rules as
   * GetData(mimeType, receiver, callback), but uses the SlotDelegate's slot and
   * connection tracker for callback delivery and lifetime tracking.
   *
   * Use this overload when the callback receiver cannot inherit
   * Dali::ConnectionTrackerInterface directly.
   *
   * @note The SlotDelegate must remain valid while callback delivery is required.
   * Keep the SlotDelegate as a class member. If it is created as a local variable,
   * the function returns, the local SlotDelegate is destroyed, and the callback
   * connection is removed before delivery can happen.
   *
   * If the SlotDelegate is destroyed or disconnected before delivery, the callback
   * will not be invoked.
   *
   * @tparam Receiver The callback receiver type wrapped by the SlotDelegate.
   * @param[in] mimeType MIME type to request. If empty, callback is invoked with false.
   * @param[in] delegate The SlotDelegate that provides the callback object and connection tracker.
   * @param[in] callback Member function pointer for the callback.
   * @SINCE_2_5.28
   */
  template<class Receiver>
  void GetData(const Dali::String& mimeType,
               Dali::SlotDelegate<Receiver>& delegate,
               void (Receiver::*callback)(bool, const Dali::ClipboardData&))
  {
    DALI_ASSERT_ALWAYS(delegate.GetSlot() && "Clipboard::GetData SlotDelegate has no slot");
    DALI_ASSERT_ALWAYS(callback && "Clipboard::GetData callback is null");

    DoGetData(mimeType,
              delegate.GetConnectionTracker(),
              MakeCallback(delegate.GetSlot(), callback));
  }

  /**
   * @brief Emitted when the platform notifies this process that clipboard data is available to request.
   *
   * This signal does not deliver clipboard content. It provides the MIME type
   * of data that can be requested. Applications may call GetData() with the
   * given MIME type to receive the actual content.
   *
   * @note This signal is intended for special clipboard listener roles, such as
   * secondary-selection or KVM-service style components. On supported platforms,
   * such a listener process may receive this signal when clipboard data is copied
   * anywhere in the system, regardless of whether the source process uses DALi.
   *
   * General applications should normally use SetData() and GetData() directly
   * and may not receive this signal unless the platform grants such a listener role.
   *
   * @note The emitted MIME type reference is valid only during signal emission.
   * Applications should copy it if they need to keep it.
   *
   * @return The signal.
   * @SINCE_2_5.28
   */
  DataOfferedSignalType& DataOfferedSignal();

private:
  /**
   * @brief Not intended for application developers.
   *
   * Used by the template GetData() overloads to forward the tracked callback
   * request to the internal clipboard implementation.
   *
   * @param[in] mimeType MIME type to request.
   * @param[in] connectionTracker Connection tracker for lifetime management.
   * @param[in] callback The callback. Ownership is transferred.
   */
  void DoGetData(const Dali::String& mimeType,
                 Dali::ConnectionTrackerInterface* connectionTracker,
                 Dali::CallbackBase* callback);

public: // Not intended for application developers
  /// @cond internal
  explicit DALI_INTERNAL Clipboard(Internal::Adaptor::Clipboard* clipboard);
  /// @endcond
};

} // namespace Dali

#endif // DALI_CLIPBOARD_H
