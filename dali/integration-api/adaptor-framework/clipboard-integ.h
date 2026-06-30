#ifndef DALI_INTEGRATION_CLIPBOARD_INTEG_H
#define DALI_INTEGRATION_CLIPBOARD_INTEG_H

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
#include <cstdint>
#include <dali/public-api/common/dali-string.h>
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
class Clipboard;

namespace Integration
{
namespace Clipboard
{
using DataSentSignalType     = Signal<void(const char*, const char*)>;
using DataReceivedSignalType = Signal<void(uint32_t, const char*, const char*)>;
using DataSelectedSignalType = Signal<void(const char*)>;

/**
 * @brief Checks whether the clipboard singleton is available.
 *
 * @return true if the clipboard is available, false otherwise.
 */
DALI_ADAPTOR_API bool IsAvailable();

/**
 * @brief This signal is emitted when clipboard data send is completed.
 *
 * This is an integration API for framework/internal use.
 *
 * @param[in] clipboard Clipboard handle.
 * @return The signal.
 */
DALI_ADAPTOR_API DataSentSignalType& DataSentSignal(Dali::Clipboard& clipboard);

/**
 * @brief This signal is emitted when requested clipboard data is received.
 *
 * This is an integration API for the legacy id-based GetData flow.
 *
 * @param[in] clipboard Clipboard handle.
 * @return The signal.
 */
DALI_ADAPTOR_API DataReceivedSignalType& DataReceivedSignal(Dali::Clipboard& clipboard);

/**
 * @brief This signal is emitted when clipboard data is selected.
 *
 * This is an integration API for framework/internal and legacy binding use.
 * The signal provides the MIME type of the selected clipboard data.
 *
 * @param[in] clipboard Clipboard handle.
 * @return The signal.
 */
DALI_ADAPTOR_API DataSelectedSignalType& DataSelectedSignal(Dali::Clipboard& clipboard);

/**
 * @brief Requests clipboard data with the given MIME type.
 *
 * This is the legacy id-based integration API. The result is delivered through
 * DataReceivedSignal().
 *
 * @param[in] clipboard Clipboard handle.
 * @param[in] mimeType MIME type to request.
 * @return The request id, or 0 if the request failed.
 */
DALI_ADAPTOR_API uint32_t GetData(Dali::Clipboard& clipboard, const Dali::String& mimeType);

/**
 * @brief Gets the number of clipboard items.
 *
 * @param[in] clipboard Clipboard handle.
 * @return The number of clipboard items.
 */
DALI_ADAPTOR_API uint32_t GetItemCount(Dali::Clipboard& clipboard);

/**
 * @brief Shows the clipboard window.
 *
 * @param[in] clipboard Clipboard handle.
 */
DALI_ADAPTOR_API void ShowClipboard(Dali::Clipboard& clipboard);

/**
 * @brief Hides the clipboard window.
 *
 * @param[in] clipboard Clipboard handle.
 * @param[in] skipFirstHide Whether to skip the first hide operation.
 */
DALI_ADAPTOR_API void HideClipboard(Dali::Clipboard& clipboard, bool skipFirstHide = false);

/**
 * @brief Returns whether the clipboard window is visible.
 *
 * @param[in] clipboard Clipboard handle.
 * @return true if the clipboard window is visible.
 */
DALI_ADAPTOR_API bool IsVisible(const Dali::Clipboard& clipboard);

} // namespace Clipboard
} // namespace Integration
} // namespace Dali

#endif // DALI_INTEGRATION_CLIPBOARD_INTEG_H
