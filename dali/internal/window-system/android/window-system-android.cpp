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

// INTERNAL HEADERS
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/window-system/common/window-system.h>
#include <memory>

// EXTERNAL_HEADERS
#include <dali/devel-api/adaptor-framework/screen-information.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/android/android-framework.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/integration-api/debug.h>

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
class WindowSystemAndroid : public WindowSystemBase
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
    ANativeWindow* window = Dali::Integration::AndroidFramework::Get().GetApplicationWindow();
    width                 = ANativeWindow_getWidth(window);
    height                = ANativeWindow_getHeight(window);
    DALI_LOG_DEBUG_INFO("Native window width %d, height %d", width, height);
  }
};

std::unique_ptr<WindowSystemAndroid> gWindowSystem;

WindowSystemAndroid& GetImpl()
{
  if(!gWindowSystem)
  {
    gWindowSystem = std::make_unique<WindowSystemAndroid>();
  }
  return *gWindowSystem;
}
} //namespace

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

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
