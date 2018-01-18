/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/application-extensions.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/application-impl.h>
#include <dali/public-api/adaptor-framework/application.h>

namespace Dali
{

ApplicationExtensions::ApplicationExtensions()
: mApplication( NULL )
{
}

ApplicationExtensions::ApplicationExtensions(Dali::Application* application)
: mApplication( application )
{
}

ApplicationExtensions::~ApplicationExtensions()
{
}

void ApplicationExtensions::Init()
{
  Internal::Adaptor::GetImplementation(*mApplication).DoInit();
}

void ApplicationExtensions::Start()
{
  Internal::Adaptor::GetImplementation(*mApplication).DoStart();
}

void ApplicationExtensions::Terminate()
{
  Internal::Adaptor::GetImplementation(*mApplication).DoTerminate();
}

void ApplicationExtensions::Pause()
{
  Internal::Adaptor::GetImplementation(*mApplication).DoPause();
}

void ApplicationExtensions::Resume()
{
  Internal::Adaptor::GetImplementation(*mApplication).DoResume();
}

void ApplicationExtensions::LanguageChange()
{
  Internal::Adaptor::GetImplementation(*mApplication).DoLanguageChange();
}
} // namespace Dali
