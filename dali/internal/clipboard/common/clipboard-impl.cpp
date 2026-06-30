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

// CLASS HEADER
#include <dali/internal/clipboard/common/clipboard-impl.h>

// EXTERNAL INCLUDES
#include <memory>
#include <string>
#include <utility>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

void Clipboard::DoGetData(const Dali::String& mimeType,
                          Dali::ConnectionTrackerInterface* connectionTracker,
                          Dali::CallbackBase* callback)
{
  std::unique_ptr<Dali::CallbackBase> callbackHolder(callback);

  if(!connectionTracker || !callback)
  {
    DALI_LOG_ERROR("Clipboard::DoGetData invalid tracked callback request.\n");
    return;
  }

  auto request = std::make_unique<PendingRequest>(connectionTracker, callbackHolder.release());

  if(mimeType.Empty())
  {
    QueueCompletion(std::move(request), false, Dali::ClipboardData());
    return;
  }

  if(!mDataReceivedConnected)
  {
    DataReceivedSignal().Connect(this, &Clipboard::OnPublicDataReceived);
    mDataReceivedConnected = true;
  }

  BeginSubmission();

  const std::string requestMimeType = mimeType.CStr();
  const uint32_t    id              = GetData(requestMimeType);

  ReceivedResult earlyResult;
  const bool hasEarlyResult = TakeEarlyResult(id, earlyResult);

  EndSubmission();

  if(id == 0u)
  {
    QueueCompletion(std::move(request), false, Dali::ClipboardData());
    return;
  }

  if(hasEarlyResult)
  {
    QueueCompletion(std::move(request),
                    earlyResult.succeeded,
                    earlyResult.data);
    return;
  }

  if(request->completionSignal.Empty())
  {
    return;
  }

  if(mPendingRequests.count(id) != 0u)
  {
    DALI_LOG_ERROR("Clipboard DoGetData request id collision, id:%u\n", id);
    QueueCompletion(std::move(request),
                    false,
                    Dali::ClipboardData());
    return;
  }

  mPendingRequests.emplace(id, std::move(request));

  StartTimeoutTimerIfNeeded();
}

void Clipboard::QueueCompletion(std::unique_ptr<PendingRequest> request,
                                bool succeeded,
                                const Dali::ClipboardData& data)
{
  if(!request)
  {
    return;
  }

  if(request->completionSignal.Empty())
  {
    return;
  }

  mDeferredCompletions.push_back(
    DeferredCompletion{std::move(request), succeeded, data});

  ScheduleDispatch();
}

void Clipboard::ScheduleDispatch()
{
  if(mDispatchRequested)
  {
    return;
  }

  if(!Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_RELEASE_INFO("Clipboard GetData completion dropped because Adaptor is not available.\n");

    // There is no safe way to deliver these public completions without breaking
    // the deferred-callback contract, so drop them. Destroying each request also
    // disconnects and deletes its tracked callback.
    mDeferredCompletions.clear();
    return;
  }

  mDispatchRequested = true;

  Dali::Adaptor::Get().RegisterProcessorOnce(*this);
  Dali::Adaptor::Get().RequestProcessEventsAndUpdate();
}

void Clipboard::DispatchCompletions()
{
  const size_t dispatchCount = mDeferredCompletions.size();

  // Completions queued by callbacks are scheduled for a later processor cycle.
  for(size_t i = 0u; i < dispatchCount; ++i)
  {
    if(mDeferredCompletions.empty())
    {
      break;
    }

    DeferredCompletion completion = std::move(mDeferredCompletions.front());
    mDeferredCompletions.pop_front();

    if(completion.request && !completion.request->completionSignal.Empty())
    {
      completion.request->completionSignal.Emit<bool, const Dali::ClipboardData&>(
        completion.succeeded,
        completion.data);
    }
  }
}

void Clipboard::Process(bool postProcessor)
{
  if(postProcessor || !mDispatchRequested)
  {
    return;
  }

  mDispatchRequested = false;

  DispatchCompletions();

  if(!mDeferredCompletions.empty())
  {
    ScheduleDispatch();
  }
}

void Clipboard::RemoveDispatchProcessor()
{
  if(!mDispatchRequested)
  {
    return;
  }

  if(Dali::Adaptor::IsAvailable())
  {
    Dali::Adaptor::Get().UnregisterProcessorOnce(*this);
  }
  else
  {
    DALI_LOG_RELEASE_INFO("Clipboard GetData dispatch processor cleanup skipped because Adaptor is not available.\n");
  }

  mDispatchRequested = false;
}

void Clipboard::OnPublicDataReceived(uint32_t id, const char* mimeType, const char* content)
{
  auto iter = mPendingRequests.find(id);

  if(iter != mPendingRequests.end())
  {
    auto request = std::move(iter->second);
    mPendingRequests.erase(iter);

    const bool succeeded = mimeType && mimeType[0] != '\0' && content && content[0] != '\0';

    QueueCompletion(std::move(request),
                    succeeded,
                    Dali::ClipboardData(succeeded ? mimeType : "", succeeded ? content : ""));

    StopTimeoutTimerIfNoPending();
    return;
  }

  if(mSubmissionDepth > 0u)
  {
    StoreEarlyResult(id, mimeType, content);
    return;
  }
}

bool Clipboard::OnTimeoutTick()
{
  for(auto it = mPendingRequests.begin(); it != mPendingRequests.end();)
  {
    auto& request = it->second;

    if(!request || request->completionSignal.Empty())
    {
      it = mPendingRequests.erase(it);
      continue;
    }

    if(request->remainingTicks > 0u)
    {
      --request->remainingTicks;
    }

    if(request->remainingTicks == 0u)
    {
      auto timedOutRequest = std::move(request);
      it                   = mPendingRequests.erase(it);

      QueueCompletion(std::move(timedOutRequest),
                      false,
                      Dali::ClipboardData());
    }
    else
    {
      ++it;
    }
  }

  return !mPendingRequests.empty();
}

void Clipboard::StartTimeoutTimerIfNeeded()
{
  if(!mTimeoutTimer)
  {
    mTimeoutTimer = Dali::Timer::New(REQUEST_TIMEOUT_INTERVAL_MS);
    mTimeoutTimer.TickSignal().Connect(this, &Clipboard::OnTimeoutTick);
  }

  if(!mTimeoutTimer.IsRunning())
  {
    mTimeoutTimer.Start();
  }
}

void Clipboard::StopTimeoutTimerIfNoPending()
{
  if(mTimeoutTimer && mPendingRequests.empty())
  {
    mTimeoutTimer.Stop();
  }
}

void Clipboard::BeginSubmission()
{
  ++mSubmissionDepth;
}

void Clipboard::EndSubmission()
{
  if(mSubmissionDepth > 0u)
  {
    --mSubmissionDepth;
  }

  if(mSubmissionDepth == 0u && !mEarlyResults.empty())
  {
    mEarlyResults.clear();
  }
}

bool Clipboard::TakeEarlyResult(uint32_t id, ReceivedResult& result)
{
  auto iter = mEarlyResults.find(id);
  if(iter != mEarlyResults.end())
  {
    result = iter->second;
    mEarlyResults.erase(iter);
    return true;
  }
  return false;
}

void Clipboard::StoreEarlyResult(uint32_t id, const char* mimeType, const char* content)
{
  if(mSubmissionDepth == 0u)
  {
    return;
  }

  ReceivedResult result;
  result.succeeded = mimeType && mimeType[0] != '\0' && content && content[0] != '\0';
  result.data      = Dali::ClipboardData(result.succeeded ? mimeType : "", result.succeeded ? content : "");

  mEarlyResults[id] = result;
}

void Clipboard::FinalizeGetDataCallbacks()
{
  if(mDataReceivedConnected)
  {
    DataReceivedSignal().Disconnect(this, &Clipboard::OnPublicDataReceived);
    mDataReceivedConnected = false;
  }

  RemoveDispatchProcessor();

  if(mTimeoutTimer)
  {
    mTimeoutTimer.TickSignal().Disconnect(this, &Clipboard::OnTimeoutTick);

    if(mTimeoutTimer.IsRunning())
    {
      mTimeoutTimer.Stop();
    }
  }

  mPendingRequests.clear();
  mDeferredCompletions.clear();
  mEarlyResults.clear();
  mSubmissionDepth   = 0u;
  mDispatchRequested = false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
