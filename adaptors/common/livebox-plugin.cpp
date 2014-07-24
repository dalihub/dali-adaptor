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
#include "livebox-plugin.h"

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include <livebox-plugin-impl.h>

namespace Dali
{

LiveboxPlugin::LiveboxPlugin( int* argc, char **argv[] )
{
  mImpl = new Internal::Adaptor::LiveboxPlugin(*this, argc, argv, "Dali Livebox", DeviceLayout::DEFAULT_BASE_LAYOUT);
}

LiveboxPlugin::LiveboxPlugin( int* argc, char **argv[], const std::string& name )
{
  mImpl = new Internal::Adaptor::LiveboxPlugin(*this, argc, argv, name, DeviceLayout::DEFAULT_BASE_LAYOUT);
}

LiveboxPlugin::LiveboxPlugin(int* argc, char **argv[], const DeviceLayout& baseLayout)
{
  mImpl = new Internal::Adaptor::LiveboxPlugin(*this, argc, argv, "Dali Livebox", baseLayout);
}

LiveboxPlugin::LiveboxPlugin(int* argc, char **argv[], const std::string& name, const DeviceLayout& baseLayout)
{
  mImpl = new Internal::Adaptor::LiveboxPlugin(*this, argc, argv, name, baseLayout);
}

LiveboxPlugin::~LiveboxPlugin()
{
  delete mImpl;
}

void LiveboxPlugin::SetTitle(const std::string& title)
{
  mImpl->SetTitle(title);
}

void LiveboxPlugin::SetContent(const std::string& content)
{
  mImpl->SetContent(content);
}

const PositionSize& LiveboxPlugin::GetGlanceBarGeometry() const
{
  return mImpl->GetGlanceBarGeometry();
}

const GlanceBarEventInfo& LiveboxPlugin::GetGlanceBarEventInfo() const
{
  return mImpl->GetGlanceBarEventInfo();
}

LiveboxSizeType LiveboxPlugin::GetLiveboxSizeType() const
{
  return mImpl->GetLiveboxSizeType();
}

void LiveboxPlugin::Run()
{
  mImpl->Run();
}

void LiveboxPlugin::Quit()
{
  mImpl->Quit();
}

bool LiveboxPlugin::AddIdle(boost::function<void(void)> callBack)
{
  return mImpl->AddIdle(callBack);
}

LiveboxPlugin& LiveboxPlugin::Get()
{
  return Internal::Adaptor::LiveboxPlugin::Get();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::InitializedSignal()
{
  return mImpl->InitializedSignal();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::TerminatedSignal()
{
  return mImpl->TerminatedSignal();
}

LiveboxPlugin::LiveboxPluginSignal LiveboxPlugin::SignalTerminated()
{
  return mImpl->SignalTerminated();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::PausedSignal()
{
  return mImpl->PausedSignal();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::ResumedSignal()
{
  return mImpl->ResumedSignal();
}
LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::ResizedSignal()
{
  return mImpl->ResizedSignal();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::GlanceCreatedSignal()
{
  return mImpl->GlanceCreatedSignal();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::GlanceDestroyedSignal()
{
  return mImpl->GlanceDestroyedSignal();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::GlanceTouchedSignal()
{
  return mImpl->GlanceTouchedSignal();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::GlanceMovedSignal()
{
  return mImpl->GlanceMovedSignal();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::GlanceScriptEventSignal()
{
  return mImpl->GlanceScriptEventSignal();
}

LiveboxPlugin::LiveboxPluginSignalV2& LiveboxPlugin::LanguageChangedSignal()
{
  return mImpl->LanguageChangedSignal();
}

} // namespace Dali

