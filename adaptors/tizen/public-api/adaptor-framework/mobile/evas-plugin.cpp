/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/evas-plugin.h>

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

#include <internal/mobile/evas-plugin-impl.h>

namespace Dali
{

EvasPlugin::EvasPlugin(Evas_Object* parent, bool isTransparent, unsigned int initialWidth, unsigned int initialHeight)
{
  mImpl = new Internal::Adaptor::EvasPlugin(*this, parent, isTransparent, initialWidth, initialHeight);
}

EvasPlugin::~EvasPlugin()
{
  delete mImpl;
}

void EvasPlugin::Run()
{
  mImpl->Run();
}

void EvasPlugin::Pause()
{
  mImpl->Pause();
}

void EvasPlugin::Resume()
{
  mImpl->Resume();
}

void EvasPlugin::Stop()
{
  mImpl->Stop();
}

Evas_Object* EvasPlugin::GetEvasImageObject()
{
  return mImpl->GetEvasImageObject();
}

Evas_Object*  EvasPlugin::GetElmAccessObject()
{
  return mImpl->GetElmAccessObject();
}

Evas_Object*  EvasPlugin::GetElmFocusObject()
{
  return mImpl->GetElmFocusObject();
}

Dali::Adaptor* EvasPlugin::GetAdaptor()
{
  return mImpl->GetAdaptor();
}

EvasPlugin::EvasPluginSignalV2& EvasPlugin::InitSignal()
{
  return mImpl->InitSignal();
}

EvasPlugin::EvasPluginSignalV2& EvasPlugin::FirstRenderCompletedSignal()
{
  return mImpl->FirstRenderCompletedSignal();
}

EvasPlugin::EvasPluginSignalV2& EvasPlugin::TerminateSignal()
{
  return mImpl->TerminateSignal();
}

EvasPlugin::EvasPluginSignalV2& EvasPlugin::PauseSignal()
{
  return mImpl->PauseSignal();
}

EvasPlugin::EvasPluginSignalV2& EvasPlugin::ResumeSignal()
{
  return mImpl->ResumeSignal();
}

EvasPlugin::EvasPluginSignalV2& EvasPlugin::ResizeSignal()
{
  return mImpl->ResizeSignal();
}

EvasPlugin::EvasPluginSignalV2& EvasPlugin::FocusedSignal()
{
  return mImpl->FocusedSignal();
}

EvasPlugin::EvasPluginSignalV2& EvasPlugin::UnFocusedSignal()
{
  return mImpl->UnFocusedSignal();
}

/**
 * @copydoc ConnectionTrackerInterface::SignalConnected
 */
void EvasPlugin::SignalConnected( SlotObserver* slotObserver, CallbackBase* callback )
{
  mImpl->SignalConnected(slotObserver, callback );
}

/**
 * @copydoc ConnectionTrackerInterface::SignalDisconnected
 */
void EvasPlugin::SignalDisconnected( SlotObserver* slotObserver, CallbackBase* callback )
{
  mImpl->SignalDisconnected(slotObserver, callback );
}

/**
 * @copydoc ConnectionTrackerInterface::GetConnectionCount
 */
std::size_t EvasPlugin::GetConnectionCount() const
{
  return mImpl->GetConnectionCount( );
}

} // namespace Dali
