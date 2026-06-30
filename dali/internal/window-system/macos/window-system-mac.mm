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

#import <Cocoa/Cocoa.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-system.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/integration-api/debug.h>
#include <memory>

namespace Dali::Internal::Adaptor::WindowSystem
{
namespace
{
class WindowSystemMac : public WindowSystemBase
{
public:
  void Initialize()
  {
  }

  void Shutdown()
  {
  }

  void GetScreenSize(int32_t& width, int32_t& height) override
  {
    NSRect r = [[NSScreen mainScreen] frame];
    width    = static_cast<int32_t>(r.size.width);
    height   = static_cast<int32_t>(r.size.height);
  }
};

std::unique_ptr<WindowSystemMac> gWindowSystem;

WindowSystemMac& GetImpl()
{
  if(!gWindowSystem)
  {
    gWindowSystem = std::make_unique<WindowSystemMac>();
  }
  return *gWindowSystem;
}
} // unnamed namespace

void Initialize()
{
  GetImpl().Initialize();
}

void Shutdown()
{
  if(gWindowSystem)
  {
    gWindowSystem->Shutdown();
    gWindowSystem.reset();
  }
}

WindowSystemBase* GetWindowSystem()
{
  return &GetImpl();
}

} // namespace Dali::Internal::Adaptor::WindowSystem
