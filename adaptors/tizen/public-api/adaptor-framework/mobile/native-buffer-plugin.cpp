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
#include <dali/public-api/adaptor-framework/native-buffer-plugin.h>

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include <internal/mobile/native-buffer-plugin-impl.h>

namespace Dali
{

NativeBufferPlugin::NativeBufferPlugin( unsigned int initialWidth, unsigned int initialHeight, bool isTransparent, unsigned int maxBufferCount, RenderSurface::RenderMode mode, const DeviceLayout& baseLayout )
{
  mImpl = new Internal::Adaptor::NativeBufferPlugin( *this, initialWidth, initialHeight, isTransparent, maxBufferCount, mode, baseLayout );
}

NativeBufferPlugin::~NativeBufferPlugin()
{
  delete mImpl;
}

void NativeBufferPlugin::Run()
{
  mImpl->Run();
}

void NativeBufferPlugin::Pause()
{
  mImpl->Pause();
}

void NativeBufferPlugin::Resume()
{
  mImpl->Resume();
}

void NativeBufferPlugin::Stop()
{
  mImpl->Stop();
}

Dali::Adaptor* NativeBufferPlugin::GetAdaptor()
{
  return mImpl->GetAdaptor();
}

native_buffer* NativeBufferPlugin::GetNativeBufferFromOutput()
{
  return mImpl->GetNativeBufferFromOutput();
}

bool NativeBufferPlugin::AddNativeBufferToInput(native_buffer* nativeBuffer)
{
  return mImpl->AddNativeBufferToInput(nativeBuffer);
}

void NativeBufferPlugin::ChangeSurfaceSize(unsigned int width, unsigned int height)
{
  mImpl->ChangeSurfaceSize(width, height);
}

Vector2 NativeBufferPlugin::GetBufferCount()
{
  return mImpl->GetBufferCount();
}

NativeBufferPlugin::NativeBufferPluginSignalV2& NativeBufferPlugin::InitSignal()
{
  return mImpl->InitSignal();
}

NativeBufferPlugin::NativeBufferPluginSignalV2& NativeBufferPlugin::TerminateSignal()
{
  return mImpl->TerminateSignal();
}

NativeBufferPlugin::NativeBufferPluginSignalV2& NativeBufferPlugin::PauseSignal()
{
  return mImpl->PauseSignal();
}

NativeBufferPlugin::NativeBufferPluginSignalV2& NativeBufferPlugin::ResumeSignal()
{
  return mImpl->ResumeSignal();
}

NativeBufferPlugin::NativeBufferPluginSignalV2& NativeBufferPlugin::FirstRenderCompletedSignal()
{
  return mImpl->FirstRenderCompletedSignal();
}

NativeBufferPlugin::NativeBufferPluginSignalV2& NativeBufferPlugin::RenderSignal()
{
  return mImpl->RenderSignal();
}

} // namespace Dali
