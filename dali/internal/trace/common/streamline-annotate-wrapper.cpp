/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
 */

#include <third-party/streamline/streamline-annotate.h>

ANNOTATE_DEFINE;

namespace Dali
{
namespace Integration
{
namespace Trace
{

void AnnotateChannelColor(unsigned int channel, unsigned int annotateColor, char *tag)
{
  ANNOTATE_SETUP;
  ANNOTATE_CHANNEL_COLOR(channel, annotateColor, tag);
}

void AnnotateChannelEnd(unsigned int channel)
{
  ANNOTATE_CHANNEL_END(channel);
}

void AnnotateMarkerStr(char *tag)
{
  ANNOTATE_SETUP;
  ANNOTATE_MARKER_STR(tag);
}

} //namespace Trace
} //namespace Integration
} //namespace Dali
