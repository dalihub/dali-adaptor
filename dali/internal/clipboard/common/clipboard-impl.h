#ifndef DALI_INTERNAL_CLIPBOARD_H
#define DALI_INTERNAL_CLIPBOARD_H

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
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include <dali/integration-api/processor-interface.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/common/dali-string.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/signals/base-signal.h>
#include <dali/public-api/signals/callback.h>
#include <dali/public-api/signals/connection-tracker-interface.h>
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/internal/clipboard/common/clipboard-factory.h>
#include <dali/public-api/adaptor-framework/clipboard-data.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Implementation of the Clipboard
 */
class Clipboard : public Dali::BaseObject,
                  public ConnectionTracker,
                  public Dali::Integration::Processor
{
public:
  using DataSentSignalType     = Signal<void(const char*, const char*)>;
  using DataReceivedSignalType = Signal<void(uint32_t, const char*, const char*)>;
  using DataOfferedSignalType  = Signal<void(const Dali::String&)>;
  using DataSelectedSignalType = Signal<void(const char*)>;

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
   * @brief Signal emitted when clipboard data send is completed.
   */
  DataSentSignalType& DataSentSignal();

  /**
   * @brief Signal emitted when clipboard data receive is completed.
   */
  DataReceivedSignalType& DataReceivedSignal();

  /**
   * @brief Signal emitted when clipboard data is available to request.
   */
  DataOfferedSignalType& DataOfferedSignal();

  /**
   * @brief Signal emitted when clipboard data is selected or offered.
   */
  DataSelectedSignalType& DataSelectedSignal();

  /**
   * @brief Sets clipboard data.
   * @param[in] clipboardData Clipboard data containing MIME type and content.
   * @return true if the clipboard set request was accepted.
   */
  bool SetData(const Dali::ClipboardData& clipboardData);

  /**
   * @brief Checks whether clipboard data with the given MIME type is available.
   * @param[in] mimeType MIME type to check.
   * @return true if data with the MIME type is available.
   */
  bool HasType(const std::string& mimeType);

  /**
   * @brief Requests clipboard data with tracked callback (public GetData bridge).
   *
   * This is the common implementation used by public Clipboard::GetData().
   * The callback is wrapped in a request-specific BaseSignal for lifetime tracking.
   * The actual callback delivery is deferred through a one-shot processor.
   *
   * @param[in] mimeType MIME type to request.
   * @param[in] connectionTracker Connection tracker for lifetime management.
   * @param[in] callback Callback to receive the requested clipboard data (ownership transferred).
   */
  void DoGetData(const Dali::String& mimeType,
                 Dali::ConnectionTrackerInterface* connectionTracker,
                 Dali::CallbackBase* callback);

  /**
   * @brief Requests clipboard data with the given MIME type (id-based).
   * @param[in] mimeType MIME type to request.
   * @return The request id, or 0 if the request failed.
   */
  uint32_t GetData(const std::string& mimeType);

  /**
   * @brief Returns the number of clipboard items.
   * @return The number of clipboard items.
   */
  uint32_t GetItemCount();

  /**
   * @brief Shows the clipboard window.
   */
  void ShowClipboard();

  /**
   * @brief Hides the clipboard window.
   * @param[in] skipFirstHide Whether to skip the first hide operation.
   */
  void HideClipboard(bool skipFirstHide);

  /**
   * @brief Returns whether the clipboard window is visible.
   * @return true if the clipboard window is visible.
   */
  bool IsVisible() const;

  /**
   * This is called after a timeout when no new data event is received for a certain period of time on X.
   * @return will return false; one-shot timer.
   */
  bool OnReceiveData();

  /**
   * This is called after a timeout when no new data set.
   * @return will return false; one-shot timer.
   */
  bool OnMultiSelectionTimeout();

  /**
   * @brief Cleans up pending public GetData requests and processor dispatch state.
   *
   * Called from the destructor before deleting mImpl. Pending callbacks are not
   * invoked.
   */
  void FinalizeGetDataCallbacks();

private:
  Clipboard(const Clipboard&);
  Clipboard& operator=(Clipboard&);

  // Tracked callback request bridge helpers
  static constexpr uint32_t REQUEST_TIMEOUT_INTERVAL_MS = 1000u;
  static constexpr uint32_t REQUEST_TIMEOUT_TICKS       = 6u; ///< Coarse watchdog tick count.

  // MIME type constants
  static constexpr const char* MIME_TYPE_TEXT_PLAIN = "text/plain;charset=utf-8";
  static constexpr const char* MIME_TYPE_HTML       = "application/xhtml+xml";
  static constexpr const char* MIME_TYPE_TEXT_URI   = "text/uri-list";

  /**
   * @brief Result data copied from a received clipboard response.
   */
  struct ReceivedResult
  {
    bool                succeeded{false};
    Dali::ClipboardData data;
  };

  /**
   * @brief Pending public GetData request with tracked callback connection.
   *
   * The request owns a BaseSignal that owns the callback and tracks the
   * receiver lifetime through ConnectionTrackerInterface.
   */
  struct PendingRequest
  {
    explicit PendingRequest(Dali::ConnectionTrackerInterface* connectionTracker,
                            Dali::CallbackBase* callback)
    : remainingTicks(REQUEST_TIMEOUT_TICKS)
    {
      completionSignal.OnConnect(connectionTracker, callback);
    }

    PendingRequest(const PendingRequest&)            = delete;
    PendingRequest& operator=(const PendingRequest&) = delete;
    PendingRequest(PendingRequest&&)                 = delete;
    PendingRequest& operator=(PendingRequest&&)      = delete;

    Dali::BaseSignal completionSignal;
    uint32_t         remainingTicks;
  };

  /**
   * @brief Deferred public GetData completion waiting for processor dispatch.
   */
  struct DeferredCompletion
  {
    std::unique_ptr<PendingRequest> request;
    bool                            succeeded{false};
    Dali::ClipboardData             data;
  };

  /**
   * @brief Queues a public GetData completion for processor delivery.
   */
  void QueueCompletion(std::unique_ptr<PendingRequest> request,
                       bool succeeded,
                       const Dali::ClipboardData& data);

  /**
   * @brief Schedules processor dispatch for queued public GetData completions.
   */
  void ScheduleDispatch();

  /**
   * @brief Dispatches queued public GetData completions.
   */
  void DispatchCompletions();

  /**
   * @brief Removes a pending one-shot processor dispatch, if any.
   */
  void RemoveDispatchProcessor();

  /**
   * @copydoc Dali::Integration::Processor::Process()
   */
  void Process(bool postProcessor) override;

  /**
   * @copydoc Dali::Integration::Processor::GetProcessorName()
   */
  std::string_view GetProcessorName() const override
  {
    return "Clipboard";
  }

  /**
   * @brief Handles legacy DataReceivedSignal for public tracked GetData requests.
   */
  void OnPublicDataReceived(uint32_t id, const char* mimeType, const char* content);

  /**
   * @brief Checks pending public GetData requests for timeout.
   */
  bool OnTimeoutTick();

  /**
   * @brief Starts the public GetData request timeout timer if needed.
   */
  void StartTimeoutTimerIfNeeded();

  /**
   * @brief Stops the timeout timer when no public GetData requests remain.
   */
  void StopTimeoutTimerIfNoPending();

  /**
   * @brief Marks the beginning of a legacy GetData submission.
   */
  void BeginSubmission();

  /**
   * @brief Marks the end of a legacy GetData submission.
   */
  void EndSubmission();

  /**
   * @brief Takes an early result emitted during legacy GetData submission.
   */
  bool TakeEarlyResult(uint32_t id, ReceivedResult& result);

  /**
   * @brief Stores an early result emitted during legacy GetData submission.
   */
  void StoreEarlyResult(uint32_t id, const char* mimeType, const char* content);

  Impl* mImpl;

  std::map<uint32_t, std::unique_ptr<PendingRequest>> mPendingRequests;     // Pending public GetData requests keyed by legacy request id.
  std::deque<DeferredCompletion>                      mDeferredCompletions; // Public GetData completions waiting for processor delivery.
  std::map<uint32_t, ReceivedResult>                  mEarlyResults;        // Early results emitted while submitting a legacy GetData request.

  Dali::Timer mTimeoutTimer;                 // Timeout watchdog for public GetData requests.
  uint32_t    mSubmissionDepth{0u};          // Nesting depth while submitting legacy GetData requests.
  bool        mDataReceivedConnected{false}; // Whether the public bridge is connected to the legacy DataReceivedSignal.
  bool        mDispatchRequested{false};     // Whether one-shot processor dispatch is registered.
}; // class Clipboard

inline Dali::Clipboard Clipboard::Get()
{
  // Delegate creation to the platform-specific ClipboardFactory.
  return GetClipboardFactory()->CreateClipboard();
}

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
