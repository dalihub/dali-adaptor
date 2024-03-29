/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/internal/canvas-renderer/common/linear-gradient-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
LinearGradient::LinearGradient() = default;

LinearGradient::~LinearGradient() = default;

bool LinearGradient::SetBounds(Vector2 firstPoint, Vector2 SecondPoint)
{
  return false;
}

bool LinearGradient::GetBounds(Vector2& firstPoint, Vector2& SecondPoint) const
{
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
