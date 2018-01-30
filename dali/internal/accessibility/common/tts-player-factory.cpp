/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/internal/accessibility/common/tts-player-factory.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
class TtsPlayer;
namespace TtsPlayerFactory
{
/**
 * Factory function that ought to be overriden by platform implementation.
 * @return dummy implementation of Tts Player
 */
__attribute__((weak))
std::unique_ptr<TtsPlayer> New(Dali::TtsPlayer::Mode mode)
{
  return std::unique_ptr<TtsPlayer>(new Internal::Adaptor::TtsPlayer());
}
}

}

}

}