/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include "tizen-web-engine-lite.h"

// The plugin factories
extern "C" DALI_EXPORT_API Dali::WebEngineLitePlugin* CreateWebEngineLitePlugin( void )
{
  return new Dali::Plugin::TizenWebEngineLite;
}

extern "C" DALI_EXPORT_API void DestroyWebEngineLitePlugin( Dali::WebEngineLitePlugin* plugin )
{
  if( plugin != NULL )
  {
    delete plugin;
  }
}

namespace Dali
{

namespace Plugin
{

namespace
{
} // unnamed namespace

TizenWebEngineLite::TizenWebEngineLite()
: mWebEngineLiteInstance( NULL )
{
}

TizenWebEngineLite::~TizenWebEngineLite()
{
}

void TizenWebEngineLite::CreateInstance(int width, int height, int windowX, int windowY, const std::string& locale, const std::string& timezoneID)
{
  mWebEngineLiteInstance = starfishCreate(NULL, width, height, windowX, windowY, locale.c_str(), timezoneID.c_str(), 1);
}

void TizenWebEngineLite::DestroyInstance()
{
  starfishRemove(mWebEngineLiteInstance);
}

void TizenWebEngineLite::LoadHtml(const std::string& path)
{
  starfishLoadHTMLDocument(mWebEngineLiteInstance, path.c_str());
}

Dali::WebEngineLitePlugin::WebEngineLiteSignalType& TizenWebEngineLite::FinishedSignal()
{
  return mFinishedSignal;
}

} // namespace Plugin
} // namespace Dali;
