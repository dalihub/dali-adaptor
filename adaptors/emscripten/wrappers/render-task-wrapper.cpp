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

#include "render-task-wrapper.h"

// EXTERNAL INCLUDES

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

Dali::Vector2 ScreenToLocal(Dali::RenderTask self, Dali::Actor actor, float screenX, float screenY)
{
  float localX = 0;
  float localY = 0;
  self.ViewportToLocal(actor, screenX, screenY, localX, localY);
  return Dali::Vector2(localX,localY);
}

Dali::Vector2 WorldToScreen(Dali::RenderTask self, const Dali::Vector3 &position)
{
  float screenX = 0;
  float screenY = 0;
  self.WorldToViewport(position, screenX, screenY);
  return Dali::Vector2(screenX, screenY);
}


}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
