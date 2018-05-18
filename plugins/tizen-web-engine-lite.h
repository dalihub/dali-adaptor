#ifndef __DALI_TIZEN_WebEngineLite_PLUGIN_H__
#define __DALI_TIZEN_WebEngineLite_PLUGIN_H__

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

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/mutex.h>
#include <string>

// INTERNAL INCLUDES
#include "StarFishPublic.h"
#include <dali/devel-api/adaptor-framework/web-engine-lite-plugin.h>
#include <timer.h>

namespace Dali
{

namespace Plugin
{

/**
 * @brief Implementation of the Tizen WebEngineLite class which has Tizen platform dependency.
 */
class TizenWebEngineLite : public Dali::WebEngineLitePlugin, public Dali::ConnectionTracker
{
public:

  /**
   * @brief Constructor.
   */
  TizenWebEngineLite();

  /**
   * @brief Destructor.
   */
  virtual ~TizenWebEngineLite();

  /**
   * @copydoc Dali::WebEngineLitePlugin::CreateInstance()
   */
  virtual void CreateInstance(int width, int height, int windowX, int windowY, const std::string& locale, const std::string& timezoneID);

  /**
   * @copydoc Dali::WebEngineLitePlugin::DestroyInstance()
   */
  virtual void DestroyInstance();

  /**
   * @copydoc Dali::WebEngineLitePlugin::LoadHtml()
   */
  virtual void LoadHtml(const std::string& path);

  /**
   * @copydoc Dali::WebEngineLitePlugin::FinishedSignal()
   */
  virtual Dali::WebEngineLitePlugin::WebEngineLiteSignalType& FinishedSignal();

private:
  StarFishInstance* mWebEngineLiteInstance;

public:

  Dali::WebEngineLitePlugin::WebEngineLiteSignalType mFinishedSignal;
};

} // namespace Plugin
} // namespace Dali;

#endif
