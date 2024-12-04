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

void Accessible::SetListenPostRender(bool enabled)
{
}

bool Accessible::IsProxy() const
{
  return false;
}
