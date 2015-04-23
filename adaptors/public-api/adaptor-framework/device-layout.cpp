/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <device-layout.h>

namespace Dali
{

const DeviceLayout DeviceLayout::DEFAULT_BASE_LAYOUT
(
  Vector2(720.0f, 1280.0f), // The resolution of the screen
  4.65f,                    // The screen size
  Vector2(316.0f, 316.0f),  // The DPI
  30.0f                     // The Viewing Distance
);

DeviceLayout::DeviceLayout()
: resolution(),
  screenSize(0.0f),
  dpi(),
  viewingDistance(0.0f)
{
}

DeviceLayout::DeviceLayout(Vector2 resolution, float screenSize, Vector2 dpi, float viewingDistance)
: resolution(resolution),
  screenSize(screenSize),
  dpi(dpi),
  viewingDistance(viewingDistance)
{
}

DeviceLayout::~DeviceLayout()
{
}

} // namespace Dali
