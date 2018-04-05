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

// CLASS HEADER
#include <base/display-connection.h>

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include <display-connection-impl.h>

namespace Dali
{

// TODO: Create DisplayInterface?
  class DisplayInterface;

DisplayConnection* DisplayConnection::New()
{
  Internal::Adaptor::DisplayConnection* internal(Internal::Adaptor::DisplayConnection::New());

  return new DisplayConnection(internal);
}

DisplayConnection* DisplayConnection::New( RenderSurface::Type type )
{
  Internal::Adaptor::DisplayConnection* internal(Internal::Adaptor::DisplayConnection::New());
  internal->SetSurfaceType( type );

  return new DisplayConnection(internal);
}

DisplayConnection::DisplayConnection()
: mImpl( NULL )
{
}

DisplayConnection::DisplayConnection(Internal::Adaptor::DisplayConnection* impl)
: mImpl(impl)
{
}

DisplayConnection::~DisplayConnection()
{
  if (mImpl)
  {
    delete mImpl;
    mImpl = NULL;
  }
}

Any DisplayConnection::GetDisplay()
{
  return mImpl->GetDisplay();
}

void DisplayConnection::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  Internal::Adaptor::DisplayConnection::GetDpi(dpiHorizontal, dpiVertical);
}

void DisplayConnection::ConsumeEvents()
{
  mImpl->ConsumeEvents();
}

bool DisplayConnection::Initialize(DisplayInterface& displayInterface)
{
  return mImpl->Initialize();
}

}
