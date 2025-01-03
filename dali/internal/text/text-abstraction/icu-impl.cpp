/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// CLASS  HEADER
#include <dali/internal/text/text-abstraction/icu-impl.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>

namespace
{
const char* ICU_PLUGIN_SO("libdali2-icu-plugin.so");
} // namespace

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{

ICU::ICU()
: mPlugin(nullptr),
  mHandle(nullptr),
  mCreateICUPluginPtr(nullptr),
  mDestroyICUPluginPtr(nullptr),
  mInitialized(false)
{
}

ICU::~ICU()
{
  if(mHandle != nullptr)
  {
    if(mDestroyICUPluginPtr != nullptr)
    {
      mDestroyICUPluginPtr(mPlugin);
    }
    dlclose(mHandle);
  }
}

void ICU::Initialize()
{
  // Try once.
  mInitialized = true;

  char* error = nullptr;

  mHandle = dlopen(ICU_PLUGIN_SO, RTLD_LAZY);
  if(mHandle == nullptr)
  {
    error = dlerror();
    DALI_LOG_ERROR("ICU, dlopen error: %s\n", error ? error : "null");
    return;
  }

  mCreateICUPluginPtr = reinterpret_cast<CreateICUPluginFunction>(dlsym(mHandle, "CreateICUPlugin"));
  if(mCreateICUPluginPtr == nullptr)
  {
    error = dlerror();
    DALI_LOG_ERROR("Can't load symbol CreateICUPlugin(), error: %s\n", error ? error : "null");
    dlclose(mHandle);
    mHandle = nullptr;
    return;
  }

  mDestroyICUPluginPtr = reinterpret_cast<DestroyICUPluginFunction>(dlsym(mHandle, "DestroyICUPlugin"));
  if(mDestroyICUPluginPtr == nullptr)
  {
    error = dlerror();
    DALI_LOG_ERROR("Can't load symbol DestroyICUPlugin(), error: %s\n", error ? error : "null");
    dlclose(mHandle);
    mHandle             = nullptr;
    mCreateICUPluginPtr = nullptr;
    return;
  }

  mPlugin = mCreateICUPluginPtr();
  if(mPlugin == nullptr)
  {
    DALI_LOG_ERROR("Can't create the ICUPlugin object\n");
    dlclose(mHandle);
    mHandle              = nullptr;
    mCreateICUPluginPtr  = nullptr;
    mDestroyICUPluginPtr = nullptr;
    return;
  }
}

void ICU::UpdateLineBreakInfoByLocale(const std::string& text,
                                      Length             numberOfCharacters,
                                      const char*        locale,
                                      LineBreakInfo*     breakInfo)
{
  if(!mInitialized)
  {
    Initialize();
  }
  if(mPlugin != nullptr)
  {
    mPlugin->UpdateLineBreakInfoByLocale(text, numberOfCharacters, locale, breakInfo);
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
