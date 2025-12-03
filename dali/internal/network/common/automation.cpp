
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

// CLASS HEADER
#include <dali/internal/network/common/automation.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/rendering/frame-buffer-devel.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/dali-core.h>
#include <stdio.h>
#include <iomanip>
#include <sstream>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/network/common/network-service-impl.h>

using Dali::Actor;
using Dali::DecoratedVisualRenderer;
using Dali::FrameBuffer;
using Dali::Layer;
using Dali::Matrix;
using Dali::Matrix3;
using Dali::Property;
using Dali::Renderer;
using Dali::RenderTask;
using Dali::RenderTaskList;
using Dali::VisualRenderer;
using Dali::Window;
using namespace Dali::DevelFrameBuffer;

namespace // un-named namespace
{
const unsigned int MAX_SET_PROPERTY_STRING_LENGTH = 256; ///< maximum length of a set property command

class JsonPropertyValue
{
public:
  JsonPropertyValue(const std::string& str)
  {
    std::size_t strLength = str.length();

    mString.reserve(strLength);
    for(std::size_t i = 0; i < strLength; ++i)
    {
      const char c = str[i];
      if((c != '[') && c != ']')
      {
        mString.push_back(c);
      }
    }
  }
  std::string GetString() const
  {
    return mString;
  }
  float GetFloat() const
  {
    return atof(mString.c_str());
  }
  int GetInt()
  {
    return atoi(mString.c_str());
  }
  bool GetBoolean()
  {
    return (GetInt() != 0);
  }

  Dali::Vector2 GetVector2()
  {
    Dali::Vector2 vec2;

    int count = sscanf(mString.c_str(), "%f,%f", &vec2.x, &vec2.y);
    if(count != 2)
    {
      DALI_LOG_ERROR("Bad format\n");
    }
    return vec2;
  }

  Dali::Vector3 GetVector3()
  {
    Dali::Vector3 vec3;

    int count = sscanf(mString.c_str(), "%f,%f,%f", &vec3.x, &vec3.y, &vec3.z);
    if(count != 3)
    {
      DALI_LOG_ERROR("Bad format\n");
    }
    return vec3;
  }

  Dali::Vector4 GetVector4()
  {
    Dali::Vector4 vec4;

    int count = sscanf(mString.c_str(), "%f,%f,%f,%f", &vec4.x, &vec4.y, &vec4.z, &vec4.w);
    if(count != 4)
    {
      DALI_LOG_ERROR("Bad format\n");
    }
    return vec4;
  }

private:
  std::string mString;
};

void SetProperty(Dali::Handle handle, int propertyId, JsonPropertyValue& propertyValue)
{
  Dali::Property::Type type = handle.GetPropertyType(propertyId);
  switch(type)
  {
    case Dali::Property::FLOAT:
    {
      float val = propertyValue.GetFloat();
      handle.SetProperty(propertyId, Dali::Property::Value(val));
      break;
    }
    case Dali::Property::INTEGER:
    {
      int val = propertyValue.GetInt();
      handle.SetProperty(propertyId, Dali::Property::Value(val));
      break;
    }
    case Dali::Property::BOOLEAN:
    {
      bool val = propertyValue.GetBoolean();
      handle.SetProperty(propertyId, Dali::Property::Value(val));
      break;
    }
    case Dali::Property::STRING:
    {
      std::string str = propertyValue.GetString();
      handle.SetProperty(propertyId, Dali::Property::Value(str));
      break;
    }
    case Dali::Property::VECTOR2:
    {
      Dali::Vector2 val = propertyValue.GetVector2();
      handle.SetProperty(propertyId, Dali::Property::Value(val));
      break;
    }
    case Dali::Property::VECTOR3:
    {
      Dali::Vector3 val = propertyValue.GetVector3();
      handle.SetProperty(propertyId, Dali::Property::Value(val));
      break;
    }
    case Dali::Property::VECTOR4:
    {
      Dali::Vector4 val = propertyValue.GetVector4();
      handle.SetProperty(propertyId, Dali::Property::Value(val));
      break;
    }
    default:
    {
      break;
    }
  }
}

int SetProperties(const std::string& setPropertyMessage)
{
  Dali::Actor root = Dali::Internal::Adaptor::Adaptor::Get().GetWindows()[0].GetRootLayer();

  std::istringstream iss(setPropertyMessage);
  std::string        token;
  getline(iss, token, '|'); // swallow command name
  while(getline(iss, token, '|'))
  {
    std::string actorId, propName, propValue;
    if(token.compare("---") != 0)
    {
      std::istringstream propss(token);
      getline(propss, actorId, ';');
      getline(propss, propName, ';');
      getline(propss, propValue);

      int         id = atoi(actorId.c_str());
      Dali::Actor a  = root.FindChildById(id);
      if(a)
      {
        // lookup by name for custom properties
        int propId = a.GetPropertyIndex(propName);
        if(propId > 0)
        {
          JsonPropertyValue pv(propValue);
          SetProperty(a, propId, pv);
        }
      }
    }
  }

  return 0;
}

void MatrixToStream(Property::Value value, std::ostream& o)
{
  Matrix  m4(false);
  Matrix3 m3;

  if(value.Get(m4))
  {
    float* matrix = m4.AsFloat();
    o << "[ [" << matrix[0] << ", " << matrix[1] << ", " << matrix[2] << ", " << matrix[3] << "], "
      << "[" << matrix[4] << ", " << matrix[5] << ", " << matrix[6] << ", " << matrix[7] << "], "
      << "[" << matrix[8] << ", " << matrix[9] << ", " << matrix[10] << ", " << matrix[11] << "], "
      << "[" << matrix[12] << ", " << matrix[13] << ", " << matrix[14] << ", " << matrix[15] << "] ]";
  }
  else if(value.Get(m3))
  {
    float* matrix = m3.AsFloat();
    o << "[ [" << matrix[0] << ", " << matrix[1] << ", " << matrix[2] << "], "
      << "[" << matrix[3] << ", " << matrix[4] << ", " << matrix[5] << "], "
      << "[" << matrix[6] << ", " << matrix[7] << ", " << matrix[8] << "] ]";
  }
}

}; // namespace

inline std::string Quote(const std::string& in)
{
  return (std::string("\"") + in + std::string("\""));
}

template<class T>
std::string ToString(T i)
{
  std::stringstream ss;
  std::string       s;
  ss << i;
  s = ss.str();

  return s;
}

std::string GetPropertyValueString(Dali::Handle handle, int propertyIndex)
{
  std::ostringstream valueStream;
  if(propertyIndex != Dali::Property::INVALID_INDEX)
  {
    Dali::Property::Value value = handle.GetProperty(propertyIndex);

    switch(value.GetType())
    {
      case Dali::Property::STRING:
      case Dali::Property::MAP:
      case Dali::Property::ARRAY:
      {
        // Escape the string (to ensure valid json)
        // Write out quotes, escapes and control characters using unicode syntax \uXXXX
        std::ostringstream unescapedValue;
        unescapedValue << value;
        std::string        valueString = unescapedValue.str();
        std::ostringstream escapedValue;
        for(std::string::iterator c = valueString.begin(); c != valueString.end(); ++c)
        {
          if(*c == '"')
          {
            escapedValue << "\\\"";
          }
          else if(*c == '\\')
          {
            escapedValue << "\\\\";
          }
          else if(*c == '\r')
          {
            escapedValue << "\\\n";
          }
          else if('\x00' <= *c && *c <= '\x1f')
          {
            escapedValue << "\\u" << std::hex << std::setw(4) << std::setfill('0') << int(*c);
          }
          else
          {
            escapedValue << *c;
          }
        }
        valueStream << escapedValue.str();
        break;
      }
      case Dali::Property::MATRIX:
      case Dali::Property::MATRIX3:
      {
        MatrixToStream(value, valueStream);
        break;
      }
      default:
      {
        valueStream << value;
        break;
      }
    }
  }
  else
  {
    valueStream << "INVALID";
  }

  return valueStream.str();
}

// currently rotations are output in Euler format ( may change)
void AppendPropertyNameAndValue(Dali::Handle handle, int propertyIndex, std::ostringstream& outputStream)
{
  // get the property name and the value as a string
  std::string propertyName(handle.GetPropertyName(propertyIndex));

  // Apply quotes around the property name
  outputStream << "\"" << propertyName << "\""
               << ",";

  // Convert value to a string
  std::string valueString = GetPropertyValueString(handle, propertyIndex);

  outputStream << "\"" << valueString << "\"";
}

void AppendPropertyAsObject(Dali::Handle handle, int propertyIndex, std::ostringstream& outputStream)
{
  // get the property name and the value as a string
  std::string propertyName(handle.GetPropertyName(propertyIndex));

  // Apply quotes around the property name
  outputStream << "{\"" << propertyName << "\":";

  // Convert value to a string
  std::string valueString = GetPropertyValueString(handle, propertyIndex);

  outputStream << "\"" << valueString << "\"}";
}

void AppendRendererPropertyNameAndValue(Dali::Renderer renderer, int rendererIndex, const std::string& name, std::ostringstream& outputStream)
{
  outputStream << ",[\"renderer[" << rendererIndex << "]." << name << "\""
               << ",";
  std::string valueString = GetPropertyValueString(renderer, renderer.GetPropertyIndex(name));
  outputStream << "\"" << valueString << "\"]";
}

bool ExcludeProperty(int propIndex)
{
  // all of these are repeat properties of values in vectors....
  // We don't really need these in the UI
  return (propIndex == Dali::Actor::Property::NAME ||
          propIndex == Dali::Actor::Property::ANCHOR_POINT_X ||
          propIndex == Dali::Actor::Property::ANCHOR_POINT_Y ||
          propIndex == Dali::Actor::Property::ANCHOR_POINT_Z ||
          propIndex == Dali::Actor::Property::PARENT_ORIGIN_X ||
          propIndex == Dali::Actor::Property::PARENT_ORIGIN_Y ||
          propIndex == Dali::Actor::Property::PARENT_ORIGIN_Z ||
          propIndex == Dali::Actor::Property::COLOR_RED ||
          propIndex == Dali::Actor::Property::COLOR_GREEN ||
          propIndex == Dali::Actor::Property::COLOR_BLUE ||
          propIndex == Dali::Actor::Property::COLOR_ALPHA ||
          propIndex == Dali::Actor::Property::POSITION_X ||
          propIndex == Dali::Actor::Property::POSITION_Y ||
          propIndex == Dali::Actor::Property::POSITION_Z ||
          propIndex == Dali::Actor::Property::SIZE_WIDTH ||
          propIndex == Dali::Actor::Property::SIZE_HEIGHT ||
          propIndex == Dali::Actor::Property::SCALE_X ||
          propIndex == Dali::Actor::Property::SCALE_Y ||
          propIndex == Dali::Actor::Property::SCALE_Z ||
          propIndex == Dali::Actor::Property::SIZE_DEPTH);
}

std::string DumpJson(Dali::Actor actor, int level)
{
  // All the information about this actor
  std::ostringstream msg;
  int                id = actor["id"];
  msg << "{ " << Quote("Name") << " : " << Quote(actor.GetProperty<std::string>(Dali::Actor::Property::NAME)) << ", " << Quote("level") << " : " << level << ", " << Quote("id") << " : " << id << ", " << Quote("IsVisible")
      << " : " << actor.GetCurrentProperty<bool>(Dali::Actor::Property::VISIBLE) << ", " << Quote("IsSensitive") << " : " << actor.GetProperty<bool>(Dali::Actor::Property::SENSITIVE);

  msg << ", " << Quote("properties") << ": [ ";

  Dali::Property::IndexContainer indices;
  actor.GetPropertyIndices(indices);

  Dali::Property::IndexContainer::Iterator iter      = indices.Begin();
  int                                      numCustom = 0;
  for(; iter != indices.End(); iter++)
  {
    int i = *iter;
    if(!ExcludeProperty(i))
    {
      if(numCustom++ != 0)
      {
        msg << ", ";
      }
      msg << "[";

      AppendPropertyNameAndValue(actor, i, msg);

      msg << "]";
    }
  }
  if(actor.GetRendererCount() > 0)
  {
    for(unsigned int i = 0; i < actor.GetRendererCount(); ++i)
    {
      auto renderer = actor.GetRendererAt(i);
      AppendRendererPropertyNameAndValue(renderer, i, "opacity", msg);
      AppendRendererPropertyNameAndValue(renderer, i, "blendMode", msg);
      if(auto visualRenderer = VisualRenderer::DownCast(renderer))
      {
        AppendRendererPropertyNameAndValue(visualRenderer, i, "transformOffset", msg);
        AppendRendererPropertyNameAndValue(visualRenderer, i, "transformSize", msg);
        AppendRendererPropertyNameAndValue(visualRenderer, i, "transformOffsetSizeMode", msg);
        AppendRendererPropertyNameAndValue(visualRenderer, i, "transformOrigin", msg);
        AppendRendererPropertyNameAndValue(visualRenderer, i, "transformAnchorPoint", msg);
        AppendRendererPropertyNameAndValue(visualRenderer, i, "extraSize", msg);
        AppendRendererPropertyNameAndValue(visualRenderer, i, "visualMixColor", msg);
        AppendRendererPropertyNameAndValue(visualRenderer, i, "visualPreMultipliedAlpha", msg);
      }
      if(auto decoratedRenderer = DecoratedVisualRenderer::DownCast(renderer))
      {
        AppendRendererPropertyNameAndValue(decoratedRenderer, i, "cornerRadius", msg);
        AppendRendererPropertyNameAndValue(decoratedRenderer, i, "cornerRadiusPolicy", msg);
        AppendRendererPropertyNameAndValue(decoratedRenderer, i, "borderlineWidth", msg);
        AppendRendererPropertyNameAndValue(decoratedRenderer, i, "borderlineColor", msg);
        AppendRendererPropertyNameAndValue(decoratedRenderer, i, "blurRadius", msg);
        AppendRendererPropertyNameAndValue(decoratedRenderer, i, "cornerSquareness", msg);
      }
    }
  }

  msg << "]";
  msg << ", " << Quote("children") << " : [ ";

  // Recursively dump all the children as well
  for(unsigned int i = 0; i < actor.GetChildCount(); ++i)
  {
    if(i)
    {
      msg << " , ";
    }
    msg << DumpJson(actor.GetChildAt(i), level + 1);
  }
  msg << "] }";

  return msg.str();
}

std::string GetActorTree()
{
  Dali::Actor actor = Dali::Internal::Adaptor::Adaptor::Get().GetWindows()[0].GetRootLayer();
  std::string str   = DumpJson(actor, 0);
  return str;
}

void DumpFrameBuffer(std::ostringstream& msg, FrameBuffer fbo)
{
  msg << "{";
  if(!fbo)
  {
    msg << "";
  }
  else
  {
    Dali::Uint16Pair size = Dali::DevelFrameBuffer::GetSize(fbo);
    msg << "\"Size\":[" << size.GetWidth() << ", " << size.GetHeight() << "],\n";
    msg << "\"ColorAttachmentCount\":" << int(Dali::DevelFrameBuffer::GetColorAttachmentCount(fbo)) << ",\n";
    FrameBuffer::Attachment::Mask mask = Dali::DevelFrameBuffer::GetMask(fbo);

    msg << "\"DepthAttachment\":" << Quote(Dali::DevelFrameBuffer::GetDepthTexture(fbo) ? "Explicit" : ((mask & FrameBuffer::Attachment::Mask::DEPTH) > 0 ? "Implicit" : "None")) << ",\n";
    msg << "\"DepthStencilAttachment\":" << Quote(Dali::DevelFrameBuffer::GetDepthStencilTexture(fbo) ? "Explicit" : ((mask & FrameBuffer::Attachment::Mask::STENCIL) > 0 ? "Implicit" : "None")) << "\n";
  }
  msg << "}\n";
}

void DumpRenderTaskList(std::ostringstream& msg, RenderTaskList tasks)
{
  msg << "[";
  uint32_t taskCount = tasks.GetTaskCount();
  for(uint32_t i = 0; i < taskCount; ++i)
  {
    auto renderTask = tasks.GetTask(i);
    if(i > 0)
    {
      msg << ",\n";
    }
    msg << "{";
    msg << "\"RenderTaskId\":" << renderTask.GetRenderTaskId() << ",\n";
    Actor src = renderTask.GetSourceActor();
    msg << "\"SourceActor\":\"" << src.GetProperty<std::string>(Actor::Property::NAME) << "[" << src.GetProperty<int>(Actor::Property::ID) << "]" << "\",\n";
    if(src.GetProperty<bool>(Actor::Property::IS_LAYER))
    {
      Dali::Layer srcLayer = Layer::DownCast(src);
      if(srcLayer)
      {
        msg << "\"Layer Properties\":[{\"depth\":" << srcLayer.GetProperty<int>(Layer::Property::DEPTH)
            << "},{\"depthTest\":" << srcLayer.GetProperty<bool>(Layer::Property::DEPTH_TEST)
            << "}],\n";
      }
    }
    Dali::Property::IndexContainer indices;
    renderTask.GetPropertyIndices(indices);
    Dali::Property::IndexContainer::Iterator iter  = indices.Begin();
    int                                      first = 1;
    msg << "\"Properties\":[";
    for(; iter != indices.End(); iter++)
    {
      if(!first)
      {
        msg << ",";
      }
      first     = 0;
      int index = *iter;
      AppendPropertyAsObject(renderTask, index, msg);
    }
    msg << "],";
    msg << "\"IsExclusive\":" << (renderTask.IsExclusive() ? "true" : "false") << ",\n";
    msg << "\"InputEnabled\":" << (renderTask.GetInputEnabled() ? "true" : "false") << ",\n";
    auto fbo = renderTask.GetFrameBuffer();
    msg << "\"Framebuffer\":";
    DumpFrameBuffer(msg, fbo);
    msg << ",\n";
    msg << "\"ClearColor\":" << renderTask.GetClearColor() << ",\n";
    msg << "\"ClearEnabled\":" << (renderTask.GetClearEnabled() ? "true" : "false") << ",\n";
    msg << "\"CullMode\":" << (renderTask.GetCullMode() ? "true" : "false") << ",\n";
    msg << "\"RefreshRate\":" << renderTask.GetRefreshRate() << ",\n";
    msg << "\"RenderPassTag\":" << renderTask.GetRenderPassTag() << ",\n";
    msg << "\"OrderIndex\":" << renderTask.GetOrderIndex() << "\n";
    msg << "}\n";
  }
  msg << "]";
}

void DumpWindow(std::ostringstream& msg, Window window)
{
  msg << "{";
  window.GetRootLayer();
  auto size = window.GetSize();
  auto pos  = window.GetPosition();
  msg << "\"LayerCount\":" << window.GetLayerCount() << ",\n";
  msg << "\"IsVisible\":" << (window.IsVisible() ? "true" : "false") << ",\n";
  msg << "\"Size\":[" << size.GetWidth() << "," << size.GetHeight() << "],\n";
  msg << "\"Position\":[" << pos.GetX() << "," << pos.GetY() << "],\n";
  msg << "\"PartialUpdate\":" << (window.IsPartialUpdateEnabled() ? "true" : "false") << ",\n";

  msg << "\"RenderTaskList\":";
  DumpRenderTaskList(msg, window.GetRenderTaskList());
  msg << "\n}\n";
}

std::string GetRenderTasks()
{
  std::ostringstream msg;
  int                windowIndex = 0;
  msg << "{";
  for(auto& window : Dali::Internal::Adaptor::Adaptor::Get().GetWindows())
  {
    if(windowIndex > 0)
    {
      msg << ",";
    }
    msg << "\"Window " << windowIndex << "\":";
    DumpWindow(msg, window);
  }
  msg << "}\n";
  return msg.str();
}

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace Automation
{
void SetProperty(const std::string& message)
{
  // check the set property length is within range
  if(message.length() > MAX_SET_PROPERTY_STRING_LENGTH)
  {
    DALI_LOG_ERROR("SetProperty message length too long, size = %ul\n", message.length());
    return;
  }

  SetProperties(message);
}

void DumpScene(unsigned int clientId, ClientSendDataInterface* sendData)
{
  char        buf[32];
  std::string json   = GetActorTree();
  int         length = json.length();
  snprintf(buf, 32, "%d\n", length);
  std::string header(buf);
  json = buf + json;
  sendData->SendData(json.c_str(), json.length(), clientId);
}

void DumpRenderTasks(unsigned clientId, ClientSendDataInterface* sendData)
{
  char        buf[32];
  std::string json   = GetRenderTasks();
  int         length = json.length();
  snprintf(buf, 32, "%d\n", length);
  std::string header(buf);
  json = buf + json;
  sendData->SendData(json.c_str(), json.length(), clientId);
}

void SetCustomCommand(const std::string& message)
{
  if(Adaptor::IsAvailable())
  {
    Internal::Adaptor::NetworkServicePtr networkService = Internal::Adaptor::NetworkService::Get();
    if(networkService)
    {
      networkService->EmitCustomCommandReceivedSignal(message);
    }
  }
}

} // namespace Automation

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
