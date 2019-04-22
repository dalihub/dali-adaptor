/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/vector-animation-renderer.h>

// INTERNAL INCLUDES
#include <dali/internal/vector-animation/common/vector-animation-renderer-impl.h>

namespace Dali
{

VectorAnimationRenderer VectorAnimationRenderer::New( const std::string& url )
{
  Internal::Adaptor::VectorAnimationRendererPtr animationRenderer = Internal::Adaptor::VectorAnimationRenderer::New();
  if( animationRenderer )
  {
    animationRenderer->Initialize( url );
  }

  return VectorAnimationRenderer( animationRenderer.Get() );
}

VectorAnimationRenderer::VectorAnimationRenderer()
{
}

VectorAnimationRenderer::~VectorAnimationRenderer()
{
}

VectorAnimationRenderer::VectorAnimationRenderer( Internal::Adaptor::VectorAnimationRenderer* internal )
: BaseHandle( internal )
{
}

VectorAnimationRenderer::VectorAnimationRenderer( const VectorAnimationRenderer& handle )
: BaseHandle( handle )
{
}

VectorAnimationRenderer& VectorAnimationRenderer::operator=( const VectorAnimationRenderer& rhs )
{
  BaseHandle::operator=( rhs );
  return *this;
}

void VectorAnimationRenderer::SetRenderer( Renderer renderer )
{
  GetImplementation( *this ).SetRenderer( renderer );
}

void VectorAnimationRenderer::SetSize( uint32_t width, uint32_t height )
{
  GetImplementation( *this ).SetSize( width, height );
}

void VectorAnimationRenderer::StopRender()
{
  GetImplementation( *this ).StopRender();
}

void VectorAnimationRenderer::Render( uint32_t frameNumber )
{
  GetImplementation( *this ).Render( frameNumber );
}

uint32_t VectorAnimationRenderer::GetTotalFrameNumber() const
{
  return GetImplementation( *this ).GetTotalFrameNumber();
}

float VectorAnimationRenderer::GetFrameRate() const
{
  return GetImplementation( *this ).GetFrameRate();
}

void VectorAnimationRenderer::GetDefaultSize( uint32_t& width, uint32_t& height ) const
{
  GetImplementation( *this ).GetDefaultSize( width, height );
}

} // namespace Dali
