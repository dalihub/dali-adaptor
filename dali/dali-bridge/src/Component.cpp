#include "Common.hpp"
#include <iostream>

using namespace Dali::Accessibility;

bool Component::Contains( Point p, CoordType ctype )
{
  auto extents = GetExtents( ctype );
  return p.x >= extents.position.x && p.y >= extents.position.y && p.x <= extents.position.x + extents.size.width && p.y <= extents.position.y + extents.size.height;
}

Component* Component::GetAccessibleAtPoint( Point p, CoordType ctype )
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
