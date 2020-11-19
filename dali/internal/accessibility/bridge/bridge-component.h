#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COMPONENT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COMPONENT_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <array>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/accessibility/bridge/bridge-base.h>

class BridgeComponent : public virtual BridgeBase
{
protected:
  BridgeComponent();

  void RegisterInterfaces();

  Dali::Accessibility::Component* FindSelf() const;

public:
  DBus::ValueOrError< bool > Contains( int32_t x, int32_t y, uint32_t coordType );
  DBus::ValueOrError< Dali::Accessibility::Accessible* > GetAccessibleAtPoint( int32_t x, int32_t y, uint32_t coordType );
  DBus::ValueOrError< std::tuple< int32_t, int32_t, int32_t, int32_t > > GetExtents( uint32_t coordType );
  DBus::ValueOrError< int32_t, int32_t > GetPosition( uint32_t coordType );
  DBus::ValueOrError< int32_t, int32_t > GetSize( uint32_t coordType );
  DBus::ValueOrError< Dali::Accessibility::ComponentLayer > GetLayer();
  DBus::ValueOrError< double > GetAlpha();
  DBus::ValueOrError< bool > GrabFocus();
  DBus::ValueOrError< bool > GrabHighlight();
  DBus::ValueOrError< bool > ClearHighlight();
  DBus::ValueOrError< int16_t > GetMdiZOrder();
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_COMPONENT_H
