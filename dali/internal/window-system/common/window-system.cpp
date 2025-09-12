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

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-system.h>

// EXTERNAL_HEADERS
#include <cstdio>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace WindowSystem
{
namespace
{
static uint32_t gDpiHorizontal = 0u;
static uint32_t gDpiVertical   = 0u;
} //namespace

void SetDpi(uint32_t dpiHorizontal, uint32_t dpiVertical)
{
  gDpiHorizontal = dpiHorizontal;
  gDpiVertical   = dpiVertical;
}

void GetDpi(uint32_t& dpiHorizontal, uint32_t& dpiVertical)
{
  dpiHorizontal = gDpiHorizontal;
  dpiVertical   = gDpiVertical;
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
