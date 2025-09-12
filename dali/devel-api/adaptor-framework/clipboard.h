#ifndef DALI_CLIPBOARD_H
#define DALI_CLIPBOARD_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>

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
} //namespace Internal DALI_INTERNAL

/**
 * @brief Interface to the device's clipboard.
 *
 * Clipboard class supports the copy and paste functionality for multi-window.
 */

class DALI_ADAPTOR_API Clipboard : public BaseHandle
{
public:
  /**
   * @brief Structure that contains information about the clipboard data information.
   */
  struct ClipData
  {
    ClipData(const char* mimeType = nullptr, const char* data = nullptr)
    {
      this->mimeType = mimeType;
      this->data     = data;
    }
    void SetMimeType(const char* mimeType)
    {
      this->mimeType = mimeType;
    }
    const char* GetMimeType() const
    {
      return mimeType;
    }
    void SetData(const char* data)
    {
      this->data = data;
    }
    const char* GetData() const
    {
      return data;
    }

  private:
    const char* mimeType{nullptr}; ///< The mime type of clipboard data.
    const char* data{nullptr};     ///< The clipboard data.
  };

  /// @brief Data send completed signal.
  typedef Signal<void(const char*, const char*)> DataSentSignalType;

  /// @brief Data receive completed signal.
  typedef Signal<void(uint32_t, const char*, const char*)> DataReceivedSignalType;

  /// @brief To catch data selection event.
  typedef Signal<void(const char*)> DataSelectedSignalType;

  /**
   * @brief Create an uninitialized Clipboard.
   * this can be initialized with one of the derived Clipboard's New() methods
   */
  Clipboard();

  /**
   * @brief Destructor
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~Clipboard();

  /**
   * @brief This constructor is used by Adaptor::GetClipboard().
   * @param[in] clipboard A pointer to the clipboard.
   */
  explicit DALI_INTERNAL Clipboard(Internal::Adaptor::Clipboard* clipboard);

  /**
   * @brief Retrieve a handle to the Clipboard instance.
   * @return A handle to the Clipboard
   */
  static Clipboard Get();

  /**
   * @brief Checks whether the clipboard is available.
   * @return true if it is available, false otherwise.
   */
  static bool IsAvailable();

  /**
   * @brief This signal is emitted when the data send complete.
   * @note
   * SetData() opertion does not follow a synchronous call.
   * It follows the sequence below.
   * SetData() -> EcoreEventDataSend() -> SendData() -> DataSentSignal() Emit
   */
  DataSentSignalType& DataSentSignal();

  /**
   * @brief This signal is emitted when the data receive complete.
   * @note
   * GetData() opertion does not follow a synchronous call.
   * It follows the sequence below.
   * GetData() -> EcoreEventOfferDataReady() -> ReceiveData() -> DataReceivedSignal() Emit
   */
  DataReceivedSignalType& DataReceivedSignal();

  /**
   * @brief This signal is emitted when the data seleted.
   */
  DataSelectedSignalType& DataSelectedSignal();

  /**
   * @brief Check if there is data in the clipboard with a given mime type.
   * @param[in] mimeType mime type to search for.
   * @return bool true if there is data, otherwise false.
   */
  bool HasType(const std::string& mimeType);

  /**
   * @brief Send the given data to the clipboard.
   * @param[in] clipData data to send to the clipboard
   * @return bool true if the internal clipboard sending was successful.
   */
  bool SetData(const ClipData& clipData);

  /**
   * @brief Request data from the clipboard.
   * @param[in] mimeType mime type of data to request.
   * @return uint32_t Returns the data request id.
   */
  uint32_t GetData(const std::string& mimeType);

  /**
   * @brief Returns the number of item currently in the clipboard.
   * @return size_t number of clipboard items.
   */
  size_t NumberOfItems();

  /**
   * @brief Show the clipboard window.
   */
  void ShowClipboard();

  /**
   * @brief Hide the clipboard window.
   */
  void HideClipboard();

  /**
   * @brief Retrieves the clipboard's visibility.
   * @return bool true if the clipboard is visible.
   */
  bool IsVisible() const;
};
} // namespace Dali

#endif // DALI_CLIPBOARD_H
