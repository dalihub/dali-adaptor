#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_FACTORY_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_FACTORY_H

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
 *
 */

// EXTERNAL INCLUDES
#include <memory>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class WindowBase;
class Window;
class WindowRenderSurface;

class WindowBaseFactory
{
public:

  WindowBaseFactory() = default;
  virtual ~WindowBaseFactory() = default;

  virtual std::unique_ptr< Dali::Internal::Adaptor::WindowBase > CreateWindowBase( Window* window, WindowRenderSurface* windowRenderSurface ) = 0;
};

extern std::unique_ptr< WindowBaseFactory > GetWindowBaseFactory();

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_FACTORY_H
