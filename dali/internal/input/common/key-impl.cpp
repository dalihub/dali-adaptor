/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/input/common/key-impl.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>
#include <string.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace KeyLookup
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gKeyExtensionLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_KEY_EXTENSION");
#endif

// Path for loading extension keys
const char* KEY_EXTENSION_PLUGIN_SO("libdali2-key-extension.so");

class KeyMap
{
public:
  KeyMap()
  : mExtensionKeyLookupTable(NULL),
    mPlugin(NULL),
    mHandle(NULL),
    mCreateKeyExtensionPluginPtr(NULL),
    mDestroyKeyExtensionPluginPtr(NULL),
    mLookup(cmpString),
    mExtensionLookup(cmpString),
    mExtensionLookupCount(0),
    mIsLookupTableInitialized(false),
    mIsExtensionLookupTableInitialized(false),
    mIsExtensionSourceFromApi(false)
  {
  }

  ~KeyMap()
  {
    if(mHandle != NULL)
    {
      if(mDestroyKeyExtensionPluginPtr != NULL)
      {
        mDestroyKeyExtensionPluginPtr(mPlugin);
      }

      dlclose(mHandle);
    }
  }

  int GetDaliKeyEnum(const char* keyName)
  {
    // If lookup table is not initialized, initialize lookup table
    if(!mIsLookupTableInitialized)
    {
      InitializeLookupTable();
    }

    Lookup::const_iterator i = mLookup.find(keyName);

    if(i == mLookup.end())
    {
      // If cannot find target, find it at the extension
      // If extension lookup table is not initialized, initialize extension lookup table
      if(!mIsExtensionLookupTableInitialized)
      {
        InitializeExtensionLookupTable();
      }

      // Find at extension
      i = mExtensionLookup.find(keyName);

      if(i == mExtensionLookup.end())
      {
        return -1;
      }
      else
      {
        return (*i).second.first;
      }
    }
    else
    {
      return (*i).second.first;
    }
  }

  const char* GetKeyName(int daliKeyCode)
  {
    // If lookup table is not initialized, initialize lookup table
    if(!mIsLookupTableInitialized)
    {
      InitializeLookupTable();
    }

    for(size_t i = 0; i < KEY_LOOKUP_COUNT; ++i)
    {
      if(KeyLookupTable[i].daliKeyCode == daliKeyCode)
      {
        return KeyLookupTable[i].keyName;
      }
    }

    // If extension lookup table is not initialized, initialize extension lookup table
    if(!mIsExtensionLookupTableInitialized)
    {
      InitializeExtensionLookupTable();
    }

    for(size_t i = 0; i < mExtensionLookupCount; ++i)
    {
      if(mExtensionKeyLookupTable[i].daliKeyCode == daliKeyCode)
      {
        return mExtensionKeyLookupTable[i].keyName;
      }
    }

    return NULL;
  }

  bool IsDeviceButton(const char* keyName)
  {
    // If lookup table is not initialized, initialize lookup table
    if(!mIsLookupTableInitialized)
    {
      InitializeLookupTable();
    }

    Lookup::const_iterator i = mLookup.find(keyName);
    if(i == mLookup.end())
    {
      // If cannot find target, find it at the extension.
      // If extension lookup table is not initialized, initialize extension lookup table
      if(!mIsExtensionLookupTableInitialized)
      {
        InitializeExtensionLookupTable();
      }

      // Find at extension
      i = mExtensionLookup.find(keyName);

      if(i == mExtensionLookup.end())
      {
        return false;
      }
      else
      {
        return (*i).second.second;
      }
    }
    else
    {
      return (*i).second.second;
    }

    return false;
  }

  void SetKeyExtensionLookupTable(const Dali::KeyLookupEntry* table, uint32_t count)
  {
    // Only one extension source is used. If the plugin has already been loaded, ignore the API registration.
    if(mIsExtensionLookupTableInitialized || mHandle != NULL)
    {
      DALI_LOG_ERROR("Key extension plugin already loaded, ignoring SetKeyExtensionLookupTable()\n");
      return;
    }

    if(table == NULL)
    {
      DALI_LOG_ERROR("table is null, ignoring SetKeyExtensionLookupTable()\n");
      return;
    }

    // Deep-copy the name strings; the incoming keyName pointers may not outlive this call.
    mExtensionKeyNames.clear();
    mExtensionKeyNames.reserve(count);
    for(uint32_t i = 0; i < count; ++i)
    {
      mExtensionKeyNames.push_back(table[i].keyName ? table[i].keyName : "");
    }

    // Build an owned lookup table whose keyName points into the owned strings above.
    mExtensionKeyLookupStorage.clear();
    mExtensionKeyLookupStorage.reserve(count);
    for(uint32_t i = 0; i < count; ++i)
    {
      Dali::KeyLookupEntry entry{mExtensionKeyNames[i].c_str(), table[i].daliKeyCode, table[i].deviceButton};
      mExtensionKeyLookupStorage.push_back(entry);
    }

    mExtensionKeyLookupTable  = mExtensionKeyLookupStorage.data();
    mExtensionLookupCount     = count;
    mIsExtensionSourceFromApi = true;
  }

private:
  void InitializeLookupTable()
  {
    // create the lookup
    for(size_t i = 0; i < KEY_LOOKUP_COUNT; ++i)
    {
      mLookup[KeyLookupTable[i].keyName] = DaliKeyType(KeyLookupTable[i].daliKeyCode, KeyLookupTable[i].deviceButton);
    }

    mIsLookupTableInitialized = true;
  }

  void InitializeExtensionLookupTable()
  {
    // If the table was not registered via the API, try to load extension keys from the plugin.
    // (mExtensionKeyLookupTable / mExtensionLookupCount are set by SetKeyExtensionLookupTable() for the API source.)
    if(!mIsExtensionSourceFromApi)
    {
      // Try to load extension keys
      char* error = NULL;
      mHandle     = dlopen(KEY_EXTENSION_PLUGIN_SO, RTLD_NOW);
      error       = dlerror();

      if(mHandle == NULL)
      {
        DALI_LOG_INFO(gKeyExtensionLogFilter, Debug::General, "Failed to get handle from libdali2-key-extension.so\n");
        return;
      }

      if(error != NULL)
      {
        DALI_LOG_INFO(gKeyExtensionLogFilter, Debug::General, "dlopen got error: %s  \n", error);
        return;
      }

      mCreateKeyExtensionPluginPtr = reinterpret_cast<CreateKeyExtensionPluginFunction>(dlsym(mHandle, "CreateKeyExtensionPlugin"));
      if(mCreateKeyExtensionPluginPtr == NULL)
      {
        DALI_LOG_INFO(gKeyExtensionLogFilter, Debug::General, "Failed to get CreateKeyExtensionPlugin function\n");
        return;
      }

      mPlugin = mCreateKeyExtensionPluginPtr();
      if(mPlugin == NULL)
      {
        DALI_LOG_INFO(gKeyExtensionLogFilter, Debug::General, "Failed to create plugin object\n");
        return;
      }

      mDestroyKeyExtensionPluginPtr = reinterpret_cast<DestroyKeyExtensionPluginFunction>(dlsym(mHandle, "DestroyKeyExtensionPlugin"));
      if(mDestroyKeyExtensionPluginPtr == NULL)
      {
        DALI_LOG_INFO(gKeyExtensionLogFilter, Debug::General, "Failed to get DestroyKeyExtensionPlugin function\n");
        return;
      }

      mExtensionKeyLookupTable = mPlugin->GetKeyLookupTable();
      mExtensionLookupCount    = mPlugin->GetKeyLookupTableCount();
    }

    // Add extension keys to lookup (common to both the plugin and API sources)
    for(size_t i = 0; i < mExtensionLookupCount; ++i)
    {
      mExtensionLookup[mExtensionKeyLookupTable[i].keyName] = DaliKeyType(mExtensionKeyLookupTable[i].daliKeyCode, mExtensionKeyLookupTable[i].deviceButton);
    }

    mIsExtensionLookupTableInitialized = true;
  }

  /**
   * compare function, to compare string by pointer
   */
  static bool cmpString(const char* a, const char* b)
  {
    return strcmp(a, b) < 0;
  }

  Dali::KeyLookupEntry*     mExtensionKeyLookupTable;                                  ///< Lookup table for extension keys
  Dali::KeyExtensionPlugin* mPlugin;                                                   ///< Key extension plugin handle
  void*                     mHandle;                                                   ///< Handle for the loaded library
  typedef Dali::KeyExtensionPlugin* (*CreateKeyExtensionPluginFunction)();             ///< Type of function pointer to get KeyExtensionPlugin object
  typedef void (*DestroyKeyExtensionPluginFunction)(Dali::KeyExtensionPlugin* plugin); ///< Type of function pointer to delete KeyExtensionPlugin object
  CreateKeyExtensionPluginFunction  mCreateKeyExtensionPluginPtr;                      ///< Function pointer to get KeyExtensionPlugin object
  DestroyKeyExtensionPluginFunction mDestroyKeyExtensionPluginPtr;                     ///< Function pointer to delete KeyExtensionPlugin object

  typedef std::pair<int, bool> DaliKeyType;

  typedef std::map<const char* /* key name */, DaliKeyType /* key code */, bool (*)(char const* a, char const* b)> Lookup;

  Lookup mLookup;
  Lookup mExtensionLookup;
  size_t mExtensionLookupCount;              ///< count of extension lookup table
  bool   mIsLookupTableInitialized;          ///< flag for basic lookup table initialization
  bool   mIsExtensionLookupTableInitialized; ///< flag for extension lookup table initialization
  bool   mIsExtensionSourceFromApi;          ///< true if the extension table was registered via API instead of the plugin

  std::vector<std::string>          mExtensionKeyNames;         ///< Owned copies of extension key name strings (API registration)
  std::vector<Dali::KeyLookupEntry> mExtensionKeyLookupStorage; ///< Owned copy of the extension key lookup table (API registration)
};

KeyMap& GetKeyMap()
{
  static KeyMap globalKeyLookup;
  return globalKeyLookup;
}
} // namespace

bool IsKey(const Dali::KeyEvent& keyEvent, Dali::KEY daliKey)
{
  int key = GetKeyMap().GetDaliKeyEnum(keyEvent.GetKeyName().CStr());
  return daliKey == key;
}

bool IsDeviceButton(const char* keyName)
{
  return GetKeyMap().IsDeviceButton(keyName);
}

const char* GetKeyName(Dali::KEY daliKey)
{
  return GetKeyMap().GetKeyName(daliKey);
}

int GetDaliKeyCode(const char* keyName)
{
  return GetKeyMap().GetDaliKeyEnum(keyName);
}

void SetKeyExtensionLookupTable(const Dali::KeyLookupEntry* table, uint32_t count)
{
  GetKeyMap().SetKeyExtensionLookupTable(table, count);
}

} // namespace KeyLookup

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
