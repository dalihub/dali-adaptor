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

#include <dali-test-suite-utils.h>
#include <dali/devel-api/adaptor-framework/application.h>
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-direct-rendering-connection.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-direct-rendering-session.h>
#include <dali/internal/adaptor/common/lifecycle-controller-impl.h>
#include <atomic>
#include <memory>

// Unit tests of state logic using explicit deliveries, not core dispatch.
// The dummy plugin integration tests cover real asynchronous termination.
// Application::New alone does not initialize an Adaptor in these tests.
#define private public
#include <dali/internal/web-engine/common/web-engine-direct-rendering-connection-impl.h>
#undef private

using namespace Dali;
using State = Internal::Adaptor::WebEngineDirectRenderingState;

namespace
{
struct Counts
{
  unsigned renders{0};
  unsigned releases{0};
  unsigned stops{0};
  unsigned callsAfterStop{0};
  unsigned destroyed{0};
  unsigned completions{0};
};

class Session : public WebEngineDirectRenderingSession
{
public:
  explicit Session(Counts& counts)
  : mCounts(counts)
  {
  }
  void RegisterRenderRequestedCallback(RenderRequestedCallback callback) override
  {
    mRequest = std::move(callback);
  }
  bool Render(const RenderCallbackInput&) override
  {
    mCounts.callsAfterStop += mCounts.stops != 0u;
    ++mCounts.renders;
    return true;
  }
  void ReleaseGraphicsResources(const RenderCallbackInput&) override
  {
    mCounts.callsAfterStop += mCounts.stops != 0u;
    ++mCounts.releases;
  }
  void OnStopped() override
  {
    ++mCounts.stops;
  }

private:
  ~Session() override
  {
    ++mCounts.destroyed;
  }
  Counts&                 mCounts;
  RenderRequestedCallback mRequest;
};

IntrusivePtr<State> MakeState(Counts& counts)
{
  return new State(new Session(counts), [&counts](State&)
  {
    // The session must be gone before the engine can destroy its plugin.
    DALI_TEST_EQUALS(counts.destroyed, 1u, TEST_LOCATION);
    DALI_TEST_EQUALS(counts.stops, 1u, TEST_LOCATION);
    DALI_TEST_EQUALS(counts.callsAfterStop, 0u, TEST_LOCATION);
    ++counts.completions;
  });
}
} //namespace

int UtcDaliWebEngineDirectRenderingStopWithoutAdaptorP(void)
{
  TestApplication app;
  auto            application = Application::New(nullptr, nullptr);
  Counts          counts;
  auto            state = MakeState(counts);
  state->Stop();
  state->Stop();
  state->OnEvent();
  DALI_TEST_EQUALS(counts.releases, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.stops, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.destroyed, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.completions, 1u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliWebEngineDirectRenderingMultipleTargetsP(void)
{
  TestApplication app;
  auto            application = Application::New(nullptr, nullptr);
  Counts          counts;
  auto            state  = MakeState(counts);
  unsigned        frames = 0u;
  state->SetCallbacks([]() {}, [&frames]()
  { ++frames; });
  RenderCallbackInput input{};
  state->OnRender(input);
  state->OnEvent();
  DALI_TEST_EQUALS(frames, 1u, TEST_LOCATION);
  // Core may offer several visited targets but the session owns shared resources.
  input.isTerminated      = true;
  input.isNativeApiUsable = true;
  state->OnRender(input);
  state->OnRender(input);
  input.isTerminated = false;
  state->OnRender(input);
  DALI_TEST_EQUALS(counts.renders, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.releases, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.stops, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.destroyed, 0u, TEST_LOCATION);
  state->OnEvent();
  state->OnEvent();
  DALI_TEST_EQUALS(counts.destroyed, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.completions, 1u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliWebEngineDirectRenderingApplicationTerminateP(void)
{
  TestApplication app;
  auto            application = Application::New(nullptr, nullptr);
  Counts          counts;
  auto            state     = MakeState(counts);
  auto            lifecycle = LifecycleController::Get();
  GetImplementation(lifecycle).OnTerminate();
  state->Stop();
  state->OnEvent();
  DALI_TEST_EQUALS(counts.releases, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.stops, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.destroyed, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.completions, 1u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliWebEngineDirectRenderingCpuOnlyTerminationP(void)
{
  TestApplication     app;
  auto                application = Application::New(nullptr, nullptr);
  Counts              counts;
  auto                state = MakeState(counts);
  RenderCallbackInput input{};
  input.isTerminated      = true;
  input.isNativeApiUsable = false;
  state->OnRender(input);
  // The first CPU-only delivery retires the session; a later usable delivery
  // must not call graphics cleanup after OnStopped.
  input.isNativeApiUsable = true;
  state->OnRender(input);
  DALI_TEST_EQUALS(counts.renders, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.releases, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.stops, 1u, TEST_LOCATION);
  state->OnEvent();
  DALI_TEST_EQUALS(counts.destroyed, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(counts.completions, 1u, TEST_LOCATION);
  END_TEST;
}
