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

//INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/value.h>
#include <dali/internal/accessibility/bridge/accessibility-common.h>
#include <dali/internal/system/common/system-error-print.h>

#include <dlfcn.h>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <limits>
#include <mutex>
#include <vector>

namespace Dali::Accessibility
{

namespace
{
constexpr const char* ESCAPED_QUOTE{"\""};
constexpr const char* KEY_ROLE{"role"};
constexpr const char* KEY_TEXT{"text"};
constexpr const char* KEY_STATES{"states"};
constexpr const char* KEY_TYPE{"type"};
constexpr const char* KEY_AUTOMATION_ID{"automationId"};
constexpr const char* KEY_ATTRS{"attributes"};
constexpr const char* KEY_DESCRIPTION{"description"};
constexpr const char* KEY_TOOLKIT{"toolkit"};
constexpr const char* KEY_VALUE{"value"};
constexpr const char* KEY_CHILDREN{"children"};
constexpr const char* VAL_TOOLKIT{"dali"};
constexpr const char* KEY_APPNAME{"appname"};
constexpr const char* KEY_PATH{"path"};

constexpr const char* LZ4_MARKER{"lz4b64:"};

// Function to escape special characters in a string
std::string EscapeString(const std::string& input)
{
  std::string escaped;
  escaped.reserve(input.length()); // Reserve space to avoid reallocations

  for(char ch : input)
  {
    switch(ch)
    {
      case '\n':
        escaped.append("\\n");
        break;
      case '\r':
        escaped.append("\\r");
        break;
      case '\t':
        escaped.append("\\t");
        break;
      case '\\':
        escaped.append("\\\\");
        break;
      case '\"':
        escaped.append("\\\"");
        break;
      default:
        escaped.push_back(ch);
        break;
    }
  }

  return escaped;
}

// Helper function to quote strings properly for JSON output format.
const auto Quote = [](const std::string& input, bool escape = false) -> std::string
{
  std::string escapedQuote{ESCAPED_QUOTE};
  return escapedQuote + (escape ? EscapeString(input) : input) + ESCAPED_QUOTE;
};

// --------------------------
// Base64 + LZ4 helpers (no build-time headers required)
// --------------------------

const std::string BASE64_TABLE{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

std::string DumpJson(Accessible* node, Accessible::DumpDetailLevel detailLevel, bool isRoot);

std::string Base64Encode(const uint8_t* data, size_t len)
{
  if(len == 0) return {};

  std::string out;
  out.reserve(((len + 2) / 3) * 4);

  for(size_t i = 0; i < len; i += 3)
  {
    uint32_t n      = 0;
    size_t   remain = len - i;
    n |= static_cast<uint32_t>(data[i]) << 16;
    if(remain > 1) n |= static_cast<uint32_t>(data[i + 1]) << 8;
    if(remain > 2) n |= static_cast<uint32_t>(data[i + 2]);

    out.push_back(BASE64_TABLE[(n >> 18) & 0x3F]);
    out.push_back(BASE64_TABLE[(n >> 12) & 0x3F]);
    out.push_back(remain > 1 ? BASE64_TABLE[(n >> 6) & 0x3F] : '=');
    out.push_back(remain > 2 ? BASE64_TABLE[n & 0x3F] : '=');
  }
  return out;
}

struct Lz4Api
{
  void* handle{nullptr};

  int (*compressBound)(int)                            = nullptr;
  int (*compressDefault)(const char*, char*, int, int) = nullptr;
  int (*decompressSafe)(const char*, char*, int, int)  = nullptr;

  // This file only needs compression. Decompression is not used here.
  bool ok() const
  {
    return compressBound && compressDefault;
  }
};

Lz4Api& GetLz4Api()
{
  static Lz4Api         api;
  static std::once_flag once;
  std::call_once(once, []()
  {
    DALI_LOG_DEBUG_INFO("LZ4 library loading...\n");

    // Avoid link-time dependency by loading at runtime.
    const char* candidates[] = {"liblz4.so.1", "liblz4.so"};
    for(const char* name : candidates)
    {
      api.handle = dlopen(name, RTLD_LAZY);
      if(api.handle)
      {
        break;
      }
      else
      {
        DALI_LOG_RELEASE_INFO("LZ4 library not found \"%s\", dlerror(): %s\n", name, dlerror());
        DALI_PRINT_SYSTEM_ERROR_LOG();
      }
    }

    // If the library wasn't found by name, it might already be loaded by other components.
    // Try resolving from RTLD_DEFAULT as a best-effort fallback.
    if(!api.handle)
    {
      DALI_LOG_RELEASE_INFO("LZ4 library not found, but try to load from RTLD_DEFAULT: handle=%p\n", RTLD_DEFAULT);
      api.handle = RTLD_DEFAULT;
    }

    if(DALI_LIKELY(api.handle))
    {
      api.compressBound   = reinterpret_cast<int (*)(int)>(dlsym(api.handle, "LZ4_compressBound"));
      api.compressDefault = reinterpret_cast<int (*)(const char*, char*, int, int)>(dlsym(api.handle, "LZ4_compress_default"));
      api.decompressSafe  = reinterpret_cast<int (*)(const char*, char*, int, int)>(dlsym(api.handle, "LZ4_decompress_safe"));
      if(!api.ok())
      {
        DALI_LOG_ERROR("LZ4 library loaded but required functions not found: compressBound=%p compressDefault=%p decompressSafe=%p, dlerror(): %s\n",
                       api.compressBound,
                       api.compressDefault,
                       api.decompressSafe,
                       dlerror());
        DALI_PRINT_SYSTEM_ERROR_LOG();
        api.handle          = nullptr; // Mark as not usable
        api.compressBound   = nullptr;
        api.compressDefault = nullptr;
        api.decompressSafe  = nullptr;
      }
      DALI_LOG_DEBUG_INFO("LZ4 library loaded successfully: handle=%p\n", api.handle);
    }
    else
    {
      DALI_LOG_ERROR("LZ4 library not found!\n");
      DALI_PRINT_SYSTEM_ERROR_LOG();
    }
  });
  return api;
}

std::string Lz4CompressToBase64Payload(const std::string& input)
{
  auto& api = GetLz4Api();
  if(!api.ok())
  {
    return {};
  }

  const auto maxIntInputSize = static_cast<std::size_t>(static_cast<uint32_t>(std::numeric_limits<int>::max()));
  if(input.size() > maxIntInputSize)
  {
    DALI_LOG_RELEASE_INFO("LZ4 compress skipped: input too large input_bytes=%zu\n", input.size());
    return {};
  }

  const int srcSize    = static_cast<int>(input.size());
  const int maxDstSize = api.compressBound(srcSize);
  if(maxDstSize <= 0)
  {
    DALI_LOG_RELEASE_INFO("LZ4 compress failed: compressBound<=0 src_bytes=%d maxDstSize=%d\n", srcSize, maxDstSize);
    return {};
  }

  std::vector<char> dst(static_cast<size_t>(maxDstSize));
  const int         compressedSize = api.compressDefault(input.data(), dst.data(), srcSize, maxDstSize);
  if(compressedSize <= 0)
  {
    DALI_LOG_RELEASE_INFO(
      "LZ4 compress failed: compress_default<=0 src_bytes=%d maxDstSize=%d compressedSize=%d\n",
      srcSize,
      maxDstSize,
      compressedSize);
    return {};
  }

  // Payload format: [u32 little-endian original size] + compressed bytes
  std::vector<uint8_t> payload;
  payload.resize(4 + static_cast<size_t>(compressedSize));
  payload[0] = static_cast<uint8_t>(srcSize & 0xFF);
  payload[1] = static_cast<uint8_t>((srcSize >> 8) & 0xFF);
  payload[2] = static_cast<uint8_t>((srcSize >> 16) & 0xFF);
  payload[3] = static_cast<uint8_t>((srcSize >> 24) & 0xFF);
  std::memcpy(payload.data() + 4, dst.data(), static_cast<size_t>(compressedSize));

  const auto b64 = Base64Encode(payload.data(), payload.size());
  DALI_LOG_RELEASE_INFO("LZ4 compress: src_bytes=%d comp_bytes=%d payload_bytes=%zu b64_chars=%zu\n",
                        srcSize, compressedSize, payload.size(), b64.size());

  std::string out;
  out.reserve(std::strlen(LZ4_MARKER) + b64.size());
  out.append(LZ4_MARKER);
  out.append(b64);
  return out;
}

std::string DumpJsonWithLz4Compression(Accessible* root, Accessible::DumpDetailLevel jsonDetailLevel, const char* logTag)
{
  // Generate JSON with requested semantics, then compress final string
  // to reduce DBus payload size.
  const auto json = DumpJson(root, jsonDetailLevel, true);
  DALI_LOG_RELEASE_INFO("DumpTree(root,%s) json_chars=%zu\n", logTag, json.size());
  auto lz4Payload = Lz4CompressToBase64Payload(json);
  if(lz4Payload.empty())
  {
    DALI_LOG_RELEASE_INFO("DumpTree(root,%s) compression failed, json_chars=%zu\n", logTag, json.size());
    DALI_LOG_RELEASE_INFO("DumpTree(root,%s) out_chars=%zu\n", logTag, json.size());
    return json;
  }
  const auto markerLen = std::strlen(LZ4_MARKER);
  DALI_LOG_RELEASE_INFO("DumpTree(root,%s) payload_b64_chars=%zu total_xfer_chars=%zu\n", logTag, lz4Payload.size() - markerLen, lz4Payload.size());
  DALI_LOG_RELEASE_INFO("DumpTree(root,%s) out_chars=%zu\n", logTag, lz4Payload.size());
  return lz4Payload;
}

// Helper function to check if we should include only showing nodes or not.
const auto IncludeShowingOnly = [](Accessible::DumpDetailLevel detailLevel) -> bool
{
  return detailLevel == Accessible::DumpDetailLevel::DUMP_SHORT_SHOWING_ONLY || detailLevel == Accessible::DumpDetailLevel::DUMP_FULL_SHOWING_ONLY;
};

// Helper function to get type name from attributes map.
const auto GetTypeString = [](const Attributes& attrs) -> std::string
{
  std::ostringstream msg;
  if(auto iter = attrs.find("class"); iter != attrs.end())
  {
    msg << Quote(KEY_TYPE) << " : " << Quote(iter->second);
  }
  return msg.str();
};

// Helper function to get type name from attributes map.
const auto GetAutomationIdString = [](const Attributes& attrs) -> std::string
{
  std::ostringstream msg;
  if(auto iter = attrs.find(KEY_AUTOMATION_ID); iter != attrs.end())
  {
    msg << Quote(KEY_AUTOMATION_ID) << " : " << Quote(iter->second, true);
  }
  return msg.str();
};

// Helper function to get screen coordinates as a string.
const auto GetScreenCoordString = [](Accessible* node) -> std::string
{
  std::ostringstream msg;
  if(node)
  {
    auto rect = node->GetExtents(CoordinateType::SCREEN);
    msg << Quote("x") << ": " << rect.x << ", "
        << Quote("y") << ": " << rect.y << ", "
        << Quote("w") << ": " << rect.width << ", "
        << Quote("h") << ": " << rect.height;
  }
  return msg.str();
};

// Helper function to get attributes map as a string.
const auto GetOtherAttributesString = [](const Attributes& attrs) -> std::string
{
  std::ostringstream msg;
  for(const auto& iter : attrs)
  {
    if(iter.first != "class" && iter.first != KEY_AUTOMATION_ID)
    {
      if(!msg.str().empty())
      {
        msg << ", ";
      }
      msg << Quote(iter.first) << ": " << Quote(iter.second, true);
    }
  }
  return msg.str();
};

// Helper function to get value interface properties as a string. If there is no value interface, it will try to get value text instead.
const auto GetValueString = [](Accessible* node) -> std::string
{
  std::ostringstream msg;
  if(auto valueInterface = node->GetFeature<Value>())
  {
    msg << Quote(KEY_VALUE) << ": { "
        << Quote("current") << ": " << valueInterface->GetCurrent() << ", "
        << Quote("min") << ": " << valueInterface->GetMinimum() << ", "
        << Quote("max") << ": " << valueInterface->GetMaximum() << ", "
        << Quote("increment") << ": " << valueInterface->GetMinimumIncrement()
        << "}";
  }
  else if(auto valueText = node->GetValue(); !valueText.empty())
  {
    msg << Quote(KEY_VALUE) << ": " << Quote(valueText, true);
  }
  return msg.str();
};

// Recursive function to dump accessible tree as a JSON string.
std::string DumpJson(Accessible* node, Accessible::DumpDetailLevel detailLevel, bool isRoot)
{
  if(!node)
  {
    return {};
  }

  const auto address   = node->GetAddress();
  const auto states    = node->GetStates();
  const bool isShowing = states[State::SHOWING];
  if(!isShowing && IncludeShowingOnly(detailLevel))
  {
    return {};
  }

  std::ostringstream msg;
  msg << "{ " << Quote(KEY_APPNAME) << ": " << Quote(address.GetBus()) << ", "
      << Quote(KEY_PATH) << ": " << Quote(ATSPI_PREFIX_PATH + address.GetPath()) << ", "
      << Quote(KEY_ROLE) << ": " << Quote(node->GetRoleName()) << ", "
      << Quote(KEY_STATES) << ": " << states.GetRawData64();

  if(auto text = node->GetName(); !text.empty())
  {
    msg << ", " << Quote(KEY_TEXT) << ": " << Quote(text, true);
  }

  if(auto value = GetValueString(node); !value.empty())
  {
    msg << ", " << value;
  }

  const auto attributes = node->GetAttributes();
  if(auto type = GetTypeString(attributes); !type.empty())
  {
    msg << ", " << type;
  }

  if(auto automationId = GetAutomationIdString(attributes); !automationId.empty())
  {
    msg << ", " << automationId;
  }

  if(auto screenCoord = GetScreenCoordString(node); !screenCoord.empty())
  {
    msg << ", " << screenCoord;
  }

  if(isRoot)
  {
    msg << ", " << Quote(KEY_TOOLKIT) << ": " << Quote(VAL_TOOLKIT);
  }

  if(detailLevel == Accessible::DumpDetailLevel::DUMP_FULL || detailLevel == Accessible::DumpDetailLevel::DUMP_FULL_SHOWING_ONLY)
  {
    if(auto otherAttrs = GetOtherAttributesString(attributes); !otherAttrs.empty())
    {
      msg << ", " << Quote(KEY_ATTRS) << ": { " << otherAttrs << " }";
    }

    if(auto description = node->GetDescription(); !description.empty())
    {
      msg << ", " << Quote(KEY_DESCRIPTION) << ": " << Quote(description, true);
    }
  }

  auto children = node->GetChildren();
  if(!children.empty())
  {
    msg << ", " << Quote(KEY_CHILDREN) << ": [ ";

    // Recursively dump all the children as well
    std::string childDump;
    for(const auto& child : children)
    {
      auto curChildDump = DumpJson(child, detailLevel, false);
      if(!curChildDump.empty())
      {
        if(!childDump.empty())
        {
          msg << ", ";
        }
        msg << curChildDump;
        childDump = std::move(curChildDump);
      }
    }
    msg << "]";
  }
  msg << " }";

  return msg.str();
}

} // anonymous namespace

std::vector<Accessible*> Accessible::GetChildren()
{
  return {};
}

bool Accessible::IsHidden() const
{
  return false;
}

void Accessible::SetListenPostRender(bool enabled)
{
}

bool Accessible::IsProxy() const
{
  return false;
}

std::string Accessible::DumpTree(DumpDetailLevel detailLevel)
{
  if(detailLevel == DumpDetailLevel::DUMP_FULL_SHOWING_ONLY_LZ4)
  {
    return DumpJsonWithLz4Compression(this, DumpDetailLevel::DUMP_FULL_SHOWING_ONLY, "FULL_SHOWING_ONLY_LZ4");
  }

  if(detailLevel == DumpDetailLevel::DUMP_FULL_LZ4)
  {
    return DumpJsonWithLz4Compression(this, DumpDetailLevel::DUMP_FULL, "FULL_LZ4");
  }

  const auto out = DumpJson(this, detailLevel, true);
  DALI_LOG_RELEASE_INFO("DumpTree(root) detail_level=%d out_chars=%zu\n", static_cast<int>(detailLevel), out.size());
  return out;
}

bool Accessible::IsAccessibleContainingPoint(Point point, Dali::Accessibility::CoordinateType type) const
{
  auto extents = GetExtents(type);
  return point.x >= extents.x && point.y >= extents.y && point.x <= extents.x + extents.width && point.y <= extents.y + extents.height;
}

Accessible* Accessible::GetAccessibleAtPoint(Point point, Dali::Accessibility::CoordinateType type)
{
  auto children = GetChildren();
  for(auto childIt = children.rbegin(); childIt != children.rend(); childIt++)
  {
    auto accessible = *childIt;
    if(accessible && accessible->IsAccessibleContainingPoint(point, type))
    {
      return accessible;
    }
  }
  return nullptr;
}

} //namespace Dali::Accessibility
