//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include <dali/public-api/adaptor-framework/common/adaptor.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/common/accessibility-manager.h>
#include <dali/public-api/adaptor-framework/common/imf-manager.h>
#include <dali/public-api/adaptor-framework/common/style-monitor.h>
#include <dali/public-api/adaptor-framework/common/window.h>

// INTERNAL INCLUDES
#include <internal/common/adaptor-impl.h>
#include <internal/common/render-surface-impl.h>
#include <internal/common/window-impl.h>

namespace Dali
{

Adaptor& Adaptor::New( Window window )
{
  return New( window, DeviceLayout::DEFAULT_BASE_LAYOUT );
}

Adaptor& Adaptor::New( Window window, const DeviceLayout& baseLayout )
{
  Internal::Adaptor::Window& windowImpl = GetImplementation(window);
  Adaptor* adaptor = Internal::Adaptor::Adaptor::New( windowImpl.GetSurface(), baseLayout );
  windowImpl.SetAdaptor(*adaptor);
  return *adaptor;
}

Adaptor::~Adaptor()
{
  delete mImpl;
}

void Adaptor::Start()
{
  mImpl->Start();
}

void Adaptor::Pause()
{
  mImpl->Pause();
}

void Adaptor::Resume()
{
  mImpl->Resume();
}

void Adaptor::Stop()
{
  mImpl->Stop();
}

bool Adaptor::AddIdle( boost::function<void(void)> callBack )
{
  return mImpl->AddIdle(callBack);
}

Adaptor::AdaptorSignalV2& Adaptor::ResizedSignal()
{
  return mImpl->ResizedSignal();
}

Adaptor::AdaptorSignalV2& Adaptor::LanguageChangedSignal()
{
  return mImpl->LanguageChangedSignal();
}

RenderSurface& Adaptor::GetSurface()
{
  return mImpl->GetSurface();
}

Adaptor& Adaptor::Get()
{
  return Internal::Adaptor::Adaptor::Get();
}

bool Adaptor::IsAvailable()
{
  return Internal::Adaptor::Adaptor::IsAvailable();
}

void Adaptor::RegisterSingleton(const std::type_info& info, BaseHandle singleton)
{
  mImpl->RegisterSingleton(info, singleton);
}

BaseHandle Adaptor::GetSingleton(const std::type_info& info) const
{
  return mImpl->GetSingleton(info);
}

void Adaptor::NotifyLanguageChanged()
{
  mImpl->NotifyLanguageChanged();
}

Adaptor::Adaptor()
: mImpl( NULL )
{
}

} // namespace Dali
