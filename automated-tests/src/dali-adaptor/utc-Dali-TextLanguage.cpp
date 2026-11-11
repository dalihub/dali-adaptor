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
 */

#include <dali-test-suite-utils.h>
#include <dali/devel-api/text-abstraction/shaping.h>
#include <algorithm>
#include <string>
#include <vector>

using namespace Dali;

int UtcDaliTextLanguageRfc5646P(void)
{
  std::string result;
  // All grandfathered tags from RFC 5646, Section 2.1, including case folding.
  for(const char* tag : {"en-GB-oed", "i-ami", "i-bnn", "i-default", "i-enochian", "i-hak", "i-klingon",
                         "i-lux", "i-mingo", "i-navajo", "i-pwn", "i-tao", "i-tay", "i-tsu",
                         "sgn-BE-FR", "sgn-BE-NL", "sgn-CH-DE", "art-lojban", "cel-gaulish",
                         "no-bok", "no-nyn", "zh-guoyu", "zh-hakka", "zh-min", "zh-min-nan", "zh-xiang"})
  {
    std::string lower(tag);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](char c)
    { return c >= 'A' && c <= 'Z' ? static_cast<char>(c + ('a' - 'A')) : c; });
    DALI_TEST_CHECK(TextAbstraction::NormalizeLanguageTag(tag, result));
    DALI_TEST_EQUALS(result, lower, TEST_LOCATION);
    std::string upper(tag);
    std::transform(upper.begin(), upper.end(), upper.begin(), [](char c)
    { return c >= 'a' && c <= 'z' ? static_cast<char>(c - ('a' - 'A')) : c; });
    DALI_TEST_CHECK(TextAbstraction::NormalizeLanguageTag(upper, result));
    DALI_TEST_EQUALS(result, lower, TEST_LOCATION);
  }
  // Representative script, extlang, region, variant and extension examples from Appendix A.
  for(const char* tag : {"zh-Hant", "zh-Hans", "sr-Cyrl", "sr-Latn", "zh-cmn-Hans-CN", "cmn-Hans-CN",
                         "zh-yue-HK", "yue-HK", "zh-Hans-CN", "sr-Latn-RS", "sl-rozaj-biske", "sl-nedis",
                         "de-CH-1901", "sl-IT-nedis", "hy-Latn-IT-arevela", "de-DE", "en-US", "es-419",
                         "de-CH-x-phonebk", "az-Arab-x-AZE-derbend", "x-whatever", "qaa-Qaaa-QM-x-southern",
                         "en-US-u-islamcal", "zh-CN-a-myext-x-private", "en-a-myext-b-another"})
  {
    DALI_TEST_CHECK(TextAbstraction::NormalizeLanguageTag(tag, result));
  }
  // Appendix A invalid examples, plus case-insensitive duplicate variants (Section 2.2.9).
  for(const char* tag : {"de-419-DE", "a-DE", "ar-a-aaa-b-bbb-a-ccc", "sl-rozaj-ROZAJ"})
  {
    result = "preserved";
    DALI_TEST_CHECK(!TextAbstraction::NormalizeLanguageTag(tag, result));
    DALI_TEST_EQUALS(result, std::string("preserved"), TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliTextLanguageInputContractP(void)
{
  std::string result;
  for(const char* valid : {"", "und", "ru", "sr-Cyrl", "SR-cYRL-rs", "uk", "bg", "mk", "en-u-ca-gregory", "sl-rozaj-biske-1994", "x-local", "i-klingon", "es-419"})
  {
    DALI_TEST_CHECK(TextAbstraction::NormalizeLanguageTag(valid, result));
  }
  DALI_TEST_CHECK(TextAbstraction::NormalizeLanguageTag("SR-cYRL-rs", result));
  DALI_TEST_EQUALS(result, std::string("sr-cyrl-rs"), TEST_LOCATION);
  for(const auto& invalid : std::vector<std::string>{"sr_RS", "sr_RS.UTF-8", " ru", "ru ", "ru,sr", "ru--RU", "x", "en-u", "en-a-foo-A-bar", "sl-rozaj-rozaj", std::string("sr\0RS", 5u), std::string(256u, 'a'), std::string(1u, static_cast<char>(0xff))})
  {
    result = "preserved";
    DALI_TEST_CHECK(!TextAbstraction::NormalizeLanguageTag(invalid, result));
    DALI_TEST_EQUALS(result, std::string("preserved"), TEST_LOCATION);
  }
  for(size_t length : {63u, 64u, 255u})
  {
    std::string tag = "x";
    while(tag.size() < length)
    {
      tag += '-';
      tag.append(std::min(size_t(8u), length - tag.size()), 'a');
    }
    DALI_TEST_CHECK(TextAbstraction::NormalizeLanguageTag(tag, result));
    DALI_TEST_EQUALS(result, tag, TEST_LOCATION);
    DALI_TEST_CHECK(TextAbstraction::NormalizeLanguageTag(result, result));
    DALI_TEST_EQUALS(result, tag, TEST_LOCATION);
  }
  std::string alias = "sr_RS";
  DALI_TEST_CHECK(!TextAbstraction::NormalizeLanguageTag(alias, alias));
  DALI_TEST_EQUALS(alias, std::string("sr_RS"), TEST_LOCATION);
  std::string maximum = "x";
  for(unsigned i = 0u; i < 127u; ++i) maximum += "-a";
  // 254 bytes / 127 subtags and 255 bytes / 128 subtags exercise the last offsets.
  for(const auto& tag : {maximum.substr(0u, 253u) + 'a', maximum})
  {
    DALI_TEST_CHECK(TextAbstraction::NormalizeLanguageTag(tag, result));
    DALI_TEST_EQUALS(result, tag, TEST_LOCATION);
    DALI_TEST_CHECK(TextAbstraction::NormalizeLanguageTag(result, result));
    DALI_TEST_EQUALS(result, tag, TEST_LOCATION);
  }
  // Reject 256 bytes, a 129th subtag and a trailing separator; preserve aliases too.
  for(const auto& tag : {maximum + 'a', maximum + "-a", maximum.substr(0u, 254u)})
  {
    result = "preserved";
    DALI_TEST_CHECK(!TextAbstraction::NormalizeLanguageTag(tag, result));
    DALI_TEST_EQUALS(result, std::string("preserved"), TEST_LOCATION);
    result = tag;
    DALI_TEST_CHECK(!TextAbstraction::NormalizeLanguageTag(result, result));
    DALI_TEST_EQUALS(result, tag, TEST_LOCATION);
  }
  END_TEST;
}
