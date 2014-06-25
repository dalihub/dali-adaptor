/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <render-surface.h>

// INTERNAL INCLUDES
#include <render-surface-impl.h>
#include <pixmap-render-surface.h>
#include <window-render-surface.h>

namespace Dali
{

RenderSurface::RenderSurface()
{
}

RenderSurface::~RenderSurface()
{
}

RenderSurface* CreateDefaultSurface( RenderSurface::SurfaceType type, PositionSize positionSize, const std::string& name )
{
  // create a Ecore window by default
  Any surface;
  Any display;

  Internal::Adaptor::ECore::RenderSurface* renderSurface(NULL);
  if( RenderSurface::WINDOW == type )
  {
    renderSurface = new Internal::Adaptor::ECore::WindowRenderSurface( positionSize, surface, display, name );
  }
  else
  {
    renderSurface = new Internal::Adaptor::ECore::PixmapRenderSurface( positionSize, surface, display, name );
  }

  return renderSurface;
}

} // namespace Dali
