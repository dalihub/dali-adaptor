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
#include <dali/integration-api/adaptor-framework/application-controller.h>

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/application-controller-impl.h>

namespace Dali
{
ApplicationController ApplicationController::New(PositionSize windowPositionSize, bool useUiThread)
{
  Internal::Adaptor::ApplicationControllerPtr internal = Internal::Adaptor::ApplicationController::New(windowPositionSize, useUiThread);

  return ApplicationController(internal.Get());
}

ApplicationController::ApplicationController() = default;

ApplicationController::~ApplicationController() = default;

ApplicationController::ApplicationController(const ApplicationController& copy) = default;

ApplicationController& ApplicationController::operator=(const ApplicationController& rhs) = default;

ApplicationController::ApplicationController(ApplicationController&& rhs) noexcept = default;

ApplicationController& ApplicationController::operator=(ApplicationController&& rhs) noexcept = default;

void ApplicationController::PreInitialize()
{
  Internal::Adaptor::GetImplementation(*this).PreInitialize();
}

void ApplicationController::PostInitialize()
{
  Internal::Adaptor::GetImplementation(*this).PostInitialize();
}

void ApplicationController::PrePause()
{
  Internal::Adaptor::GetImplementation(*this).PrePause();
}

void ApplicationController::PostPause()
{
  Internal::Adaptor::GetImplementation(*this).PostPause();
}

void ApplicationController::PreResume()
{
  Internal::Adaptor::GetImplementation(*this).PreResume();
}

void ApplicationController::PostResume()
{
  Internal::Adaptor::GetImplementation(*this).PostResume();
}

void ApplicationController::PreTerminate()
{
  Internal::Adaptor::GetImplementation(*this).PreTerminate();
}

void ApplicationController::PostTerminate()
{
  Internal::Adaptor::GetImplementation(*this).PostTerminate();
}

void ApplicationController::PreLanguageChanged(const std::string& language)
{
  Internal::Adaptor::GetImplementation(*this).PreLanguageChanged(language);
}

ApplicationController::ApplicationController(Internal::Adaptor::ApplicationController* controller)
: BaseHandle(controller)
{
}

} // namespace Dali
