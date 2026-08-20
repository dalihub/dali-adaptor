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
#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/key-devel.h>
#include <dali/devel-api/events/key-event-devel.h>
#include <dali/extension-api/adaptor-framework/key-extension.h>
#include <dali/integration-api/string-utils.h>
#include <dali/public-api/adaptor-framework/key.h>
#include <string>
#include <vector>

using namespace Dali;

void utc_dali_key_extension_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_key_extension_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
// Generate a KeyPressEvent to send to Core
Dali::KeyEvent GenerateKeyPress(const std::string& keyName)
{
  return DevelKeyEvent::New(Integration::ToDaliString(keyName), "", "", -1, 0, 0lu, Dali::KeyEvent::DOWN, "", "", Device::Class::NONE, Device::Subclass::NONE);
}

} // unnamed namespace

int UtcDaliKeyExtensionSetLookupTableP(void)
{
  TestApplication application;

  const char* keyNameA = "XF86ExtensionTestKeyA";
  const char* keyNameB = "XF86ExtensionTestKeyB";
  const int   codeA    = 2001;
  const int   codeB    = 2002;

  // Before registration, the extension keys are unknown.
  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode(keyNameA), -1, TEST_LOCATION);
  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode(keyNameB), -1, TEST_LOCATION);

  // Register an extension table. The source strings and array are deliberately
  // scoped so they are destroyed before we query, proving the data is deep-copied.
  {
    std::vector<std::string> names = {keyNameA, keyNameB};

    Dali::Extension::KeyLookupEntry table[] =
      {
        {names[0].c_str(), codeA, false},
        {names[1].c_str(), codeB, true},
      };

    Dali::Extension::SetKeyExtensionLookupTable(table, 2u);
  }

  // After registration, the extension keys resolve to the custom codes (name -> code).
  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode(keyNameA), codeA, TEST_LOCATION);
  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode(keyNameB), codeB, TEST_LOCATION);

  // IsKey() goes through the same extension lookup.
  DALI_TEST_CHECK(IsKey(GenerateKeyPress(keyNameA), static_cast<Dali::KEY>(codeA)));
  DALI_TEST_CHECK(IsKey(GenerateKeyPress(keyNameB), static_cast<Dali::KEY>(codeB)));

  // A key not present in either the base or extension table is still unknown.
  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode("XF86ExtensionTestKeyMissing"), -1, TEST_LOCATION);

  END_TEST;
}

int UtcDaliKeyExtensionSetLookupTableNullN(void)
{
  TestApplication application;

  // Passing a null table with a non-zero count must not crash; nothing is registered.
  Dali::Extension::SetKeyExtensionLookupTable(NULL, 5u);

  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode("XF86ExtensionTestKeyA"), -1, TEST_LOCATION);

  END_TEST;
}

int UtcDaliKeyExtensionSetLookupTableEmptyP(void)
{
  TestApplication application;

  // An empty table is a valid registration; it simply adds no keys.
  Dali::Extension::KeyLookupEntry table[] = {{"XF86ExtensionUnused", 2100, false}};
  Dali::Extension::SetKeyExtensionLookupTable(table, 0u);

  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode("XF86ExtensionUnused"), -1, TEST_LOCATION);

  // Base keys keep working.
  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode("XF86Back"), static_cast<int>(DALI_KEY_BACK), TEST_LOCATION);

  END_TEST;
}

int UtcDaliKeyExtensionSetLookupTableTwiceN(void)
{
  TestApplication application;

  // Only one extension source is used. The registered table is materialised lazily on the
  // first lookup, and from that point on a later registration is ignored.
  Dali::Extension::KeyLookupEntry first[] = {{"XF86ExtensionFirst", 2201, false}};
  Dali::Extension::SetKeyExtensionLookupTable(first, 1u);

  // This lookup materialises the table.
  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode("XF86ExtensionFirst"), 2201, TEST_LOCATION);

  Dali::Extension::KeyLookupEntry second[] = {{"XF86ExtensionSecond", 2202, false}};
  Dali::Extension::SetKeyExtensionLookupTable(second, 1u);

  // The second table was ignored; the first one is still in effect.
  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode("XF86ExtensionFirst"), 2201, TEST_LOCATION);
  DALI_TEST_EQUALS(DevelKey::GetDaliKeyCode("XF86ExtensionSecond"), -1, TEST_LOCATION);

  END_TEST;
}

int UtcDaliKeyIntegSystemKeyCodePriorityP(void)
{
  TestApplication application;

  // Every backend resolves a key code from the look up table first, so the default is off.
  DALI_TEST_CHECK(!DevelKey::IsSystemKeyCodePriority());

  // The legacy Tizen order is opt-in, for a framework carrying applications written against it.
  DevelKey::SetSystemKeyCodePriority(true);
  DALI_TEST_CHECK(DevelKey::IsSystemKeyCodePriority());

  DevelKey::SetSystemKeyCodePriority(false);
  DALI_TEST_CHECK(!DevelKey::IsSystemKeyCodePriority());

  END_TEST;
}

int UtcDaliKeyExtensionKeyLookupEntryP(void)
{
  // KeyLookupEntry is a plain aggregate owned by extension-api.
  Dali::Extension::KeyLookupEntry entry{"XF86ExtensionEntry", 2300, true};

  DALI_TEST_EQUALS(std::string(entry.keyName), std::string("XF86ExtensionEntry"), TEST_LOCATION);
  DALI_TEST_EQUALS(entry.daliKeyCode, 2300, TEST_LOCATION);
  DALI_TEST_CHECK(entry.deviceButton);

  END_TEST;
}
