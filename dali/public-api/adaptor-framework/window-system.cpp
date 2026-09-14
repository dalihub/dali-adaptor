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
#include <dali/public-api/adaptor-framework/window-system.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-system-impl.h>

namespace DALI_NAMESPACE
{
namespace WindowSystem
{
Int32Pair GetMainScreenSize()
{
  int32_t width  = 0;
  int32_t height = 0;
  Dali::Internal::Adaptor::WindowSystem::GetScreenSize(width, height);
  return Int32Pair(width, height);
}

} // namespace WindowSystem

} // namespace DALI_NAMESPACE
