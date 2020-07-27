/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/clipboard.h>
#include <dali/devel-api/adaptor-framework/accessibility-adaptor.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>
#include <dali/integration-api/adaptor-framework/native-render-surface-factory.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/layer.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

// INTERNAL INCLUDES
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

// CLASS HEADER
#include <dali/internal/offscreen/tizen/offscreen-window-impl-tizen.h>

namespace Dali
{

namespace Internal
{

OffscreenWindowTizen* OffscreenWindowTizen::New( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent )
{
  OffscreenWindowTizen* window = new OffscreenWindowTizen( width, height, surface, isTranslucent );
  return window;
}

OffscreenWindowTizen::OffscreenWindowTizen( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent )
: mRenderNotification(),
  mIsTranslucent( isTranslucent )
{
  // Create surface
  if ( surface.Empty() )
  {
    mSurface = std::unique_ptr< RenderSurfaceInterface >( CreateNativeSurface( SurfaceSize( width, height ), surface, isTranslucent ) );
  }
  else
  {
    width = static_cast<uint16_t>( tbm_surface_queue_get_width( AnyCast< tbm_surface_queue_h > ( surface ) ) );
    height = static_cast<uint16_t>( tbm_surface_queue_get_height( AnyCast< tbm_surface_queue_h > ( surface ) ) );
    mSurface = std::unique_ptr< RenderSurfaceInterface >( CreateNativeSurface( SurfaceSize( width, height ), surface, isTranslucent ) );
  }
}

void OffscreenWindowTizen::Initialize( bool isDefaultWindow )
{
  if( isDefaultWindow )
  {
    Initialize();
    return;
  }

  Dali::Integration::SceneHolder sceneHolderHandler = Dali::Integration::SceneHolder( this );
  Dali::Adaptor::Get().AddWindow( sceneHolderHandler );

  Initialize();
}

void OffscreenWindowTizen::Initialize()
{
  // Connect callback to be notified when the surface is rendered
  TriggerEventFactory triggerEventFactory;

  mRenderNotification = std::unique_ptr< TriggerEventInterface >( triggerEventFactory.CreateTriggerEvent( MakeCallback( this, &OffscreenWindowTizen::OnPostRender ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER ) );

  NativeRenderSurface* surface = GetNativeRenderSurface();

  if( !surface )
  {
    return;
  }

  surface->SetRenderNotification( mRenderNotification.get() );
}

OffscreenWindowTizen::~OffscreenWindowTizen()
{
  NativeRenderSurface* surface = GetNativeRenderSurface();

  if( surface )
  {
    // To prevent notification triggering in NativeRenderSurface::PostRender while deleting SceneHolder
    surface->SetRenderNotification( nullptr );
  }
}

uint32_t OffscreenWindowTizen::GetLayerCount() const
{
  return mScene.GetLayerCount();
}

Dali::Layer OffscreenWindowTizen::GetLayer( uint32_t depth ) const
{
  return mScene.GetLayer( depth );
}

OffscreenWindow::WindowSize OffscreenWindowTizen::GetSize() const
{
  Size size = mScene.GetSize();

  return OffscreenWindow::WindowSize( static_cast<uint16_t>( size.width ), static_cast<uint16_t>( size.height ) );
}

Dali::Any OffscreenWindowTizen::GetNativeHandle() const
{
  return GetNativeRenderSurface()->GetNativeHandle();
}

NativeRenderSurface* OffscreenWindowTizen::GetNativeRenderSurface() const
{
  return dynamic_cast< NativeRenderSurface* >( mSurface.get() );
}

void OffscreenWindowTizen::OnPostRender()
{
  mPostRenderSignal.Emit( Dali::OffscreenWindow( this ), nullptr );
}

OffscreenWindow::PostRenderSignalType& OffscreenWindowTizen::PostRenderSignal()
{
  return mPostRenderSignal;
}

} // namespace Internal

} // namespace Dali
