#ifndef DALI_INTERNAL_TEXT_LANGUAGE_H
#define DALI_INTERNAL_TEXT_LANGUAGE_H

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

// EXTERNAL INCLUDES
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace DALI_NAMESPACE::TextAbstraction::Internal
{
/**
 * @brief Validates RFC 5646 language-tag syntax and normalizes ASCII case.
 *
 * Accepts an empty string or a single tag of at most 255 bytes (a DALi limit).
 * Uses the syntax and grandfathered tags in RFC 5646, Section 2.1, and the
 * duplicate variant/singleton restrictions in Section 2.2.9.
 * Validation does not perform registry lookups or use locale-dependent character classes.
 * An empty input produces an empty NUL-terminated string.
 *
 * @param[in] input The language tag to normalize, or empty.
 * @param[out] output A 256-byte buffer receiving the NUL-terminated lowercase tag
 * on success. Its contents are unspecified on failure.
 * @return True if the input is accepted, false otherwise.
 */
inline bool NormalizeLanguageTag(const std::string& input, std::array<char, 256u>& output)
{
  if(input.size() > 255u)
  {
    return false;
  }
  for(size_t index = 0u; index < input.size(); ++index)
  {
    char c = input[index];
    if(c >= 'A' && c <= 'Z')
    {
      c += 'a' - 'A';
    }
    if(!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-'))
    {
      return false;
    }
    output[index] = c;
  }
  output[input.size()] = '\0';
  if(input.empty())
  {
    return true;
  }
  const std::string_view tag(output.data(), input.size());
  // Fixed grandfathered list from RFC 5646, Section 2.1 (irregular + regular).
  static const char* const grandfathered[] = {
    "en-gb-oed", "i-ami", "i-bnn", "i-default", "i-enochian", "i-hak", "i-klingon",
    "i-lux", "i-mingo", "i-navajo", "i-pwn", "i-tao", "i-tay", "i-tsu",
    "sgn-be-fr", "sgn-be-nl", "sgn-ch-de", "art-lojban", "cel-gaulish",
    "no-bok", "no-nyn", "zh-guoyu", "zh-hakka", "zh-min", "zh-min-nan", "zh-xiang"};
  for(const auto* legacy : grandfathered)
  {
    if(tag == legacy)
    {
      return true;
    }
  }
  // RFC 5646, Section 2.1 does not cap the number of private-use subtags.
  // DALi's 255-byte limit allows 128 one-byte subtags plus 127 separators.
  // Store byte offsets/lengths instead of a string_view for every subtag.
  struct Subtag
  {
    std::uint8_t offset;
    std::uint8_t length;
  };
  std::array<Subtag, 128u> parts;
  size_t                   count = 0u;
  size_t                   start = 0u;
  while(start <= tag.size())
  {
    size_t end = tag.find('-', start);
    if(end == std::string::npos)
    {
      end = tag.size();
    }
    if(end == start || end - start > 8u || count == parts.size())
    {
      return false;
    }
    // Nonempty subtags start at most at byte 254 and contain at most 8 bytes.
    parts[count++] = {static_cast<std::uint8_t>(start), static_cast<std::uint8_t>(end - start)};
    start          = end + 1u;
  }
  const auto subtag = [&](size_t index)
  {
    const auto& part = parts[index];
    return tag.substr(part.offset, part.length);
  };
  const auto alpha = [](std::string_view s)
  {
    return std::all_of(s.begin(), s.end(), [](char c)
    { return c >= 'a' && c <= 'z'; });
  };
  const auto digit = [](std::string_view s)
  {
    return std::all_of(s.begin(), s.end(), [](char c)
    { return c >= '0' && c <= '9'; });
  };
  size_t pos = 0u;
  if(parts[0].length != 1u || tag[0] != 'x')
  {
    if(parts[0].length < 2u || !alpha(subtag(0u)))
    {
      return false;
    }
    const auto primaryLength = parts[0].length;
    ++pos;
    if(primaryLength <= 3u)
    {
      for(unsigned n = 0u; n < 3u && pos < count && parts[pos].length == 3u && alpha(subtag(pos)); ++n)
      {
        ++pos;
      }
    }
    if(pos < count && parts[pos].length == 4u && alpha(subtag(pos)))
    {
      ++pos;
    }
    if(pos < count && ((parts[pos].length == 2u && alpha(subtag(pos))) || (parts[pos].length == 3u && digit(subtag(pos)))))
    {
      ++pos;
    }
    // RFC 5646, Section 2.2.9: variant subtags and extension singletons must be unique.
    const size_t firstVariant = pos;
    while(pos < count && (parts[pos].length >= 5u || (parts[pos].length == 4u && tag[parts[pos].offset] >= '0' && tag[parts[pos].offset] <= '9')))
    {
      for(size_t previous = firstVariant; previous < pos; ++previous)
      {
        if(subtag(previous) == subtag(pos))
        {
          return false;
        }
      }
      ++pos;
    }
    std::array<bool, 128u> extensions{};
    while(pos < count && parts[pos].length == 1u && tag[parts[pos].offset] != 'x')
    {
      const auto index = static_cast<unsigned char>(tag[parts[pos++].offset]);
      if(index >= extensions.size())
      {
        return false;
      }
      if(extensions[index])
      {
        return false;
      }
      extensions[index]  = true;
      const size_t first = pos;
      while(pos < count && parts[pos].length >= 2u)
      {
        ++pos;
      }
      if(pos == first)
      {
        return false;
      }
    }
  }
  if(pos < count && parts[pos].length == 1u && tag[parts[pos].offset] == 'x')
  {
    if(++pos == count)
    {
      return false;
    }
    pos = count;
  }
  if(pos != count)
  {
    return false;
  }
  return true;
}
} // namespace DALI_NAMESPACE::TextAbstraction::Internal
#endif // DALI_INTERNAL_TEXT_LANGUAGE_H
