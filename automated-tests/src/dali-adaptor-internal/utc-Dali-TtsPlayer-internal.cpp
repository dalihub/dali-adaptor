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
#include <dali/devel-api/adaptor-framework/tts-player.h>
#include <dali/internal/accessibility/common/tts-player-impl.h>
#include <string>
#include <type_traits>

void utc_dali_ttsplayer_internal_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_ttsplayer_internal_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
static_assert(!std::is_base_of<Dali::RefObject, Dali::Internal::Adaptor::TtsPlayer>::value, "TTS backends must not inherit RefObject");

class TestTtsPlayer : public Dali::Internal::Adaptor::TtsPlayer
{
public:
  TestTtsPlayer(bool& destroyed, bool& stopped)
  : mDestroyed(destroyed),
    mStopped(stopped)
  {
  }

  ~TestTtsPlayer() override
  {
    mDestroyed = true;
  }

  void Play(const std::string&) override
  {
  }

  void Stop() override
  {
    mStopped = true;
  }

  void Pause() override
  {
  }

  void Resume() override
  {
  }

  Dali::TtsPlayer::State GetState() override
  {
    return Dali::TtsPlayer::UNAVAILABLE;
  }

  Dali::TtsPlayer::StateChangedSignalType& StateChangedSignal() override
  {
    static Dali::TtsPlayer::StateChangedSignalType signal;
    return signal;
  }

private:
  bool& mDestroyed;
  bool& mStopped;
};
} // unnamed namespace

int UtcDaliTtsPlayerBackendLifetimeP(void)
{
  bool destroyed = false;
  bool stopped   = false;
  {
    Dali::TtsPlayer player(new TestTtsPlayer(destroyed, stopped));
    Dali::TtsPlayer copy(player);
    player.Reset();
    DALI_TEST_CHECK(!destroyed);
    copy.Stop();
    DALI_TEST_CHECK(stopped);
    copy.Reset();
    DALI_TEST_CHECK(destroyed);
  }
  END_TEST;
}
