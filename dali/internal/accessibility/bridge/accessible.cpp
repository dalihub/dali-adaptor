/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/atspi-interfaces/socket.h>
#include <dali/internal/accessibility/bridge/accessibility-common.h>

using namespace Dali::Accessibility;

Accessible::Accessible()
{
}

Accessible::~Accessible() noexcept
{
  auto handle = mBridgeData.lock();
  if(handle)
  {
    handle->mKnownObjects.erase(this);
  }
}

void Accessible::EmitActiveDescendantChanged(Accessible* child)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitActiveDescendantChanged(this, child);
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

void Accessible::EmitSocketAvailable()
{
  DALI_ASSERT_DEBUG(Socket::DownCast(this));

  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitSocketAvailable(this);
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
  std::ostringstream tmp;
  tmp << this;
  return {handle ? handle->mBusName : "", tmp.str()};
}

void Bridge::RegisterOnBridge(const Accessible* object)
{
  assert(!object->mBridgeData.lock() || object->mBridgeData.lock() == mData);
  if(!object->mBridgeData.lock())
  {
    assert(mData);
    mData->mKnownObjects.insert(object);
    object->mBridgeData = mData;
  }
}

bool Accessible::IsHidden() const
{
  return false;
}

bool Accessible::IsProxy() const
{
  return false;
}

void Accessible::NotifyAccessibilityStateChange(Dali::Accessibility::States states, bool isRecursive)
{
  if(auto data = GetBridgeData())
  {
    for(auto i = 0u; i < static_cast<unsigned int>(Dali::Accessibility::State::MAX_COUNT); i++)
    {
      auto index = static_cast<Dali::Accessibility::State>(i);
      if(states[index])
      {
        data->mBridge->EmitStateChanged(this, index, GetStates()[index], 0);
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
