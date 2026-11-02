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

#include <dali-test-suite-utils.h>
#include <dali/devel-api/text-abstraction/font-client.h>

#include <clocale>
#include <string>

namespace
{
struct LocaleRestore
{
  LocaleRestore()
  : localeName(Dali::TextAbstraction::GetLocaleFull())
  {
  }

  ~LocaleRestore()
  {
    Dali::TextAbstraction::SetLocale(localeName);
  }

  std::string localeName;
};
} // unnamed namespace

int UtcDaliLocaleLegacyParsing(void)
{
  LocaleRestore restore;
  struct Case
  {
    const char* input;
    const char* languageCode;
    const char* localeName;
  };
  const Case cases[] = {
    {"ko_KR.UTF-8", "ko", "ko_KR"},
    {"en_US", "en", "en_US"},
    {"en.UTF-8", "en.UTF-8", "en"},
    {"sr_RS@latin", "sr", "sr_RS@latin"},
    {"sr_RS.UTF-8@latin", "sr", "sr_RS"},
    {"C", "C", "C"},
    {"", "", ""},
  };
  for(const auto& item : cases)
  {
    Dali::TextAbstraction::SetLocale(item.input);
    DALI_TEST_EQUALS(Dali::TextAbstraction::GetLocale(), std::string(item.languageCode), TEST_LOCATION);
    DALI_TEST_EQUALS(Dali::TextAbstraction::GetLocaleFull(), std::string(item.localeName), TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliLocaleEnsureInitialization(void)
{
  LocaleRestore restore;
  const char* current = setlocale(LC_MESSAGES, NULL);
  DALI_TEST_CHECK(current);
  const std::string processLocale(current);

  Dali::TextAbstraction::SetLocale("");
  // Getters must not initialize implicitly.
  DALI_TEST_CHECK(Dali::TextAbstraction::GetLocale().empty());
  DALI_TEST_CHECK(Dali::TextAbstraction::GetLocaleFull().empty());
  Dali::TextAbstraction::EnsureLocale();
  DALI_TEST_EQUALS(Dali::TextAbstraction::GetLocaleFull(), processLocale.substr(0, processLocale.find('.')), TEST_LOCATION);
  DALI_TEST_EQUALS(Dali::TextAbstraction::GetLocale(), processLocale.substr(0, processLocale.find('_')), TEST_LOCATION);

  Dali::TextAbstraction::SetLocale("zz_ZZ");
  Dali::TextAbstraction::EnsureLocale();
  DALI_TEST_EQUALS(Dali::TextAbstraction::GetLocaleFull(), std::string("zz_ZZ"), TEST_LOCATION);
  DALI_TEST_EQUALS(std::string(setlocale(LC_MESSAGES, NULL)), processLocale, TEST_LOCATION);
  END_TEST;
}

int UtcDaliLocaleCopiedValueLifetime(void)
{
  LocaleRestore restore;
  Dali::TextAbstraction::SetLocale("ko_KR.UTF-8");
  const std::string saved(Dali::TextAbstraction::GetLocaleFull());
  const std::string& current = Dali::TextAbstraction::GetLocaleFull();
  Dali::TextAbstraction::SetLocale("en_US");
  DALI_TEST_EQUALS(saved, std::string("ko_KR"), TEST_LOCATION);
  DALI_TEST_EQUALS(current, std::string("en_US"), TEST_LOCATION);
  END_TEST;
}
