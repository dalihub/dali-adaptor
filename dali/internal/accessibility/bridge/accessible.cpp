/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/atspi-interfaces/component.h>
#include <dali/devel-api/atspi-interfaces/value.h>
#include <dali/internal/accessibility/bridge/accessibility-common.h>

using namespace Dali::Accessibility;

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
const auto Quote = [](const std::string& input, bool escape = false) -> std::string {
  std::string escapedQuote{ESCAPED_QUOTE};
  return escapedQuote + (escape ? EscapeString(input) : input) + ESCAPED_QUOTE;
};

// Helper function to check if we should include only showing nodes or not.
const auto IncludeShowingOnly = [](Accessible::DumpDetailLevel detailLevel) -> bool {
  return detailLevel == Accessible::DumpDetailLevel::DUMP_SHORT_SHOWING_ONLY || detailLevel == Accessible::DumpDetailLevel::DUMP_FULL_SHOWING_ONLY;
};

// Helper function to get type name from attributes map.
const auto GetTypeString = [](const Attributes& attrs) -> std::string {
  std::ostringstream msg;
  if(auto iter = attrs.find("class"); iter != attrs.end())
  {
    msg << Quote(KEY_TYPE) << " : " << Quote(iter->second);
  }
  return msg.str();
};

// Helper function to get type name from attributes map.
const auto GetAutomationIdString = [](const Attributes& attrs) -> std::string {
  std::ostringstream msg;
  if(auto iter = attrs.find(KEY_AUTOMATION_ID); iter != attrs.end())
  {
    msg << Quote(KEY_AUTOMATION_ID) << " : " << Quote(iter->second, true);
  }
  return msg.str();
};

// Helper function to get screen coordinates as a string.
const auto GetScreenCoordString = [](Accessible* node) -> std::string {
  std::ostringstream msg;
  auto*              component = Component::DownCast(node);
  if(component)
  {
    auto rect = component->GetExtents(CoordinateType::SCREEN);
    msg << Quote("x") << ": " << rect.x << ", "
        << Quote("y") << ": " << rect.y << ", "
        << Quote("w") << ": " << rect.width << ", "
        << Quote("h") << ": " << rect.height;
  }
  return msg.str();
};

// Helper function to get attributes map as a string.
const auto GetOtherAttributesString = [](const Attributes& attrs) -> std::string {
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
const auto GetValueString = [](Accessible* node) -> std::string {
  std::ostringstream msg;
  auto*              valueInterface = Value::DownCast(node);
  if(valueInterface)
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

  const auto states    = node->GetStates();
  const bool isShowing = states[State::SHOWING];
  if(!isShowing && IncludeShowingOnly(detailLevel))
  {
    return {};
  }

  std::ostringstream msg;
  msg << "{ " << Quote(KEY_ROLE) << ": " << Quote(node->GetRoleName()) << ", "
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

std::shared_ptr<Bridge::Data> Accessible::GetBridgeData() const
{
  auto handle = mBridgeData.lock();
  if(!handle)
  {
    auto bridge = Bridge::GetCurrentBridge();
    handle      = bridge->mData;
  }
  return handle;
}

Address Accessible::GetAddress() const
{
  auto handle = mBridgeData.lock();
  if(!handle)
  {
    handle = GetBridgeData();
    if(handle)
    {
      handle->mBridge->RegisterOnBridge(this);
    }
  }
  std::string path;
  auto        actor = GetInternalActor();
  if(actor)
  {
    uint32_t actorId = actor.GetProperty<int>(Dali::Actor::Property::ID);
    path             = std::to_string(actorId);
  }
  return {handle ? handle->mBusName : "", path};
}

void Bridge::RegisterOnBridge(const Accessible* object)
{
  assert(!object->mBridgeData.lock() || object->mBridgeData.lock() == mData);
  if(!object->mBridgeData.lock())
  {
    assert(mData);
    object->mBridgeData = mData;
  }
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
  return DumpJson(this, detailLevel, true);
}