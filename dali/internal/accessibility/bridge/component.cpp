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

// CLASS HEADER

// EXTERNAL INCLUDES
#include <iostream>

// INTERNAL INCLUDES
#include <dali/internal/accessibility/bridge/accessibility-common.h>

using namespace Dali::Accessibility;

bool Component::Contains( Point p, CoordType ctype )
{
  auto extents = GetExtents( ctype );
  return p.x >= extents.x && p.y >= extents.y && p.x <= extents.x + extents.width && p.y <= extents.y + extents.height;
}

Accessible* Component::GetAccessibleAtPoint( Point p, CoordType ctype )
{
  auto children = GetChildren();
  for( auto childIt = children.rbegin(); childIt != children.rend(); childIt++ )
  {
    auto component = dynamic_cast< Component* >( *childIt );
    if( component && component->Contains( p, ctype ) )
    {
      return component;
    }
  }
  return nullptr;
}

bool Component::IsScrollable()
{
  return false;
}
