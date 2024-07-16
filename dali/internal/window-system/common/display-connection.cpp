/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/common/display-connection-factory.h>
#include <dali/internal/window-system/common/display-connection.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/display-connection-impl.h>

namespace Dali
{
DisplayConnection* DisplayConnection::New()
{
  auto factory           = Dali::Internal::Adaptor::GetDisplayConnectionFactory();
  auto displayConnection = factory->CreateDisplayConnection();

  Internal::Adaptor::DisplayConnection* internal(displayConnection.release());

  return new DisplayConnection(internal);
}

DisplayConnection* DisplayConnection::New(Dali::Integration::RenderSurfaceInterface::Type type)
{
  auto factory           = Dali::Internal::Adaptor::GetDisplayConnectionFactory();
  auto displayConnection = factory->CreateDisplayConnection();

  Internal::Adaptor::DisplayConnection* internal(displayConnection.release());

  internal->SetSurfaceType(type);

  return new DisplayConnection(internal);
}

DisplayConnection::DisplayConnection() = default;

DisplayConnection::~DisplayConnection() = default;

DisplayConnection::DisplayConnection(Internal::Adaptor::DisplayConnection* impl)
{
  mImpl.reset(impl);
}

Any DisplayConnection::GetDisplay() const
{
  return mImpl->GetDisplay();
}

Any DisplayConnection::GetNativeGraphicsDisplay() const
{
  return mImpl->GetNativeGraphicsDisplay();
}

void DisplayConnection::ConsumeEvents()
{
  mImpl->ConsumeEvents();
}

} // namespace Dali
