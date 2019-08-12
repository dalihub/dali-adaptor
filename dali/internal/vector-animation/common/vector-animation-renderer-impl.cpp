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
#include <dali/internal/vector-animation/common/vector-animation-renderer-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace // unnamed namespace
{

// Type Registration
Dali::BaseHandle Create()
{
  return Dali::BaseHandle();
}

Dali::TypeRegistration type( typeid( Dali::VectorAnimationRenderer ), typeid( Dali::BaseHandle ), Create );

} // unnamed namespace

VectorAnimationRendererPtr VectorAnimationRenderer::New()
{
  VectorAnimationRendererPtr renderer = new VectorAnimationRenderer();
  return renderer;
}

VectorAnimationRenderer::VectorAnimationRenderer()
: mPlugin( std::string() )
{
}

VectorAnimationRenderer::~VectorAnimationRenderer()
{
}

void VectorAnimationRenderer::Initialize( const std::string& url )
{
  mPlugin.Initialize( url );
}

void VectorAnimationRenderer::SetRenderer( Dali::Renderer renderer )
{
  mPlugin.SetRenderer( renderer );
}

void VectorAnimationRenderer::SetSize( uint32_t width, uint32_t height )
{
  mPlugin.SetSize( width, height );
}

void VectorAnimationRenderer::StopRender()
{
  mPlugin.StopRender();
}

bool VectorAnimationRenderer::Render( uint32_t frameNumber )
{
  return mPlugin.Render( frameNumber );
}

uint32_t VectorAnimationRenderer::GetTotalFrameNumber() const
{
  return mPlugin.GetTotalFrameNumber();
}

float VectorAnimationRenderer::GetFrameRate() const
{
  return mPlugin.GetFrameRate();
}

void VectorAnimationRenderer::GetDefaultSize( uint32_t& width, uint32_t& height ) const
{
  mPlugin.GetDefaultSize( width, height );
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
