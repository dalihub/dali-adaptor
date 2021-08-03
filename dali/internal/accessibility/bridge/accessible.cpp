/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/internal/accessibility/bridge/accessibility-common.h>
#include <third-party/libunibreak/linebreak.h>
#include <third-party/libunibreak/wordbreak.h>

using namespace Dali::Accessibility;

std::vector<std::string> Accessible::GetInterfaces()
{
  std::vector<std::string> tmp;
  tmp.push_back(AtspiDbusInterfaceAccessible);
  if(dynamic_cast<Collection*>(this))
  {
    tmp.push_back(AtspiDbusInterfaceCollection);
  }
  if(dynamic_cast<Text*>(this))
  {
    tmp.push_back(AtspiDbusInterfaceText);
  }
  if(dynamic_cast<EditableText*>(this))
  {
    tmp.push_back(AtspiDbusInterfaceEditableText);
  }
  if(dynamic_cast<Value*>(this))
  {
    tmp.push_back(AtspiDbusInterfaceValue);
  }
  if(dynamic_cast<Component*>(this))
  {
    tmp.push_back(AtspiDbusInterfaceComponent);
  }
  if(auto action = dynamic_cast<Action*>(this))
  {
    if(action->GetActionCount() > 0)
    {
      tmp.push_back(AtspiDbusInterfaceAction);
    }
  }
  if(dynamic_cast<Selection*>(this))
  {
    tmp.push_back(AtspiDbusInterfaceSelection);
  }
  return tmp;
}

Accessible::Accessible()
{
}

Accessible::~Accessible()
{
  auto handle = mBridgeData.lock();
  if(handle)
  {
    handle->mKnownObjects.erase(this);
  }
}

void Accessible::EmitActiveDescendantChanged(Accessible* obj, Accessible* child)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitActiveDescendantChanged(obj, child);
  }
}

void Accessible::EmitStateChanged(State state, int newValue, int reserved)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitStateChanged(this, state, newValue, reserved);
  }
}

void Accessible::EmitShowing(bool isShowing)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitStateChanged(this, State::SHOWING, isShowing ? 1 : 0, 0);
  }
}

void Accessible::EmitVisible(bool isVisible)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitStateChanged(this, State::VISIBLE, isVisible ? 1 : 0, 0);
  }
}

void Accessible::EmitHighlighted(bool isHighlighted)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitStateChanged(this, State::HIGHLIGHTED, isHighlighted ? 1 : 0, 0);
  }
}

void Accessible::EmitFocused(bool isFocused)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitStateChanged(this, State::FOCUSED, isFocused ? 1 : 0, 0);
  }
}
void Accessible::EmitTextInserted(unsigned int position, unsigned int length, const std::string& content)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitTextChanged(this, TextChangedState::INSERTED, position, length, content);
  }
}
void Accessible::EmitTextDeleted(unsigned int position, unsigned int length, const std::string& content)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitTextChanged(this, TextChangedState::DELETED, position, length, content);
  }
}
void Accessible::EmitTextCursorMoved(unsigned int cursorPosition)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitCursorMoved(this, cursorPosition);
  }
}

void Accessible::EmitMovedOutOfScreen(ScreenRelativeMoveType type)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitMovedOutOfScreen(this, type);
  }
}

void Accessible::Emit(WindowEvent event, unsigned int detail)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->Emit(this, event, detail);
  }
}
void Accessible::Emit(ObjectPropertyChangeEvent event)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->Emit(this, event);
  }
}

void Accessible::EmitBoundsChanged(Rect<> rect)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitBoundsChanged(this, rect);
  }
}

std::vector<Accessible*> Accessible::GetChildren()
{
  std::vector<Accessible*> tmp(GetChildCount());
  for(auto i = 0u; i < tmp.size(); ++i)
  {
    tmp[i] = GetChildAtIndex(i);
  }
  return tmp;
}

std::shared_ptr<Bridge::Data> Accessible::GetBridgeData()
{
  auto handle = mBridgeData.lock();
  if(!handle)
  {
    auto bridge = Bridge::GetCurrentBridge();
    handle      = bridge->mData;
  }
  return handle;
}

Address Accessible::GetAddress()
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
  std::ostringstream tmp;
  tmp << this;
  return {handle ? handle->mBusName : "", tmp.str()};
}

void Bridge::RegisterOnBridge(Accessible* object)
{
  assert(!object->mBridgeData.lock() || object->mBridgeData.lock() == mData);
  if(!object->mBridgeData.lock())
  {
    assert(mData);
    mData->mKnownObjects.insert(object);
    object->mBridgeData = mData;
  }
}

bool Accessible::IsProxy()
{
  return false;
}

Accessible* Accessible::GetDefaultLabel()
{
  return this;
}

void Accessible::NotifyAccessibilityStateChange(Dali::Accessibility::States states, bool isRecursive)
{
  if(auto data = GetBridgeData())
  {
    auto currentState = GetStates() & states;
    for(auto i = 0u; i < currentState.size(); i++)
    {
      auto index = static_cast<Dali::Accessibility::State>(i);
      if(currentState[index])
      {
        data->mBridge->EmitStateChanged(this, index, 1, 0);
      }
    }

    if(isRecursive)
    {
      auto children = GetChildren();
      for(auto iter : children)
      {
        iter->NotifyAccessibilityStateChange(states, isRecursive);
      }
    }
  }
}

void Accessible::FindWordSeparationsUtf8(const utf8_t* string, size_t length, const char* language, char* breaks)
{
  set_wordbreaks_utf8(string, length, language, breaks);
}

void Accessible::FindLineSeparationsUtf8(const utf8_t* string, size_t length, const char* language, char* breaks)
{
  set_linebreaks_utf8(string, length, language, breaks);
}
