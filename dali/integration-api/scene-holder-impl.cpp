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
#include <dali/integration-api/scene-holder-impl.h>

// EXTERNAL HEADERS
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>

// INTERNAL HEADERS
#include <dali/internal/adaptor/common/lifecycle-observer.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

uint32_t SceneHolder::mSceneHolderCounter = 0;

class SceneHolder::SceneHolderLifeCycleObserver : public LifeCycleObserver
{
public:

  SceneHolderLifeCycleObserver(Adaptor*& adaptor)
  : mAdaptor( adaptor )
  {
  };

private: // Adaptor::LifeCycleObserver interface

  void OnStart() override {};
  void OnPause() override {};
  void OnResume() override {};
  void OnStop() override {};
  void OnDestroy() override
  {
    mAdaptor = nullptr;
  };

private:
  Adaptor*& mAdaptor;
};


SceneHolder::SceneHolder()
: mLifeCycleObserver( new SceneHolderLifeCycleObserver( mAdaptor ) ),
  mId( mSceneHolderCounter++ ),
  mSurface( nullptr ),
  mAdaptor( nullptr ),
  mAdaptorStarted( false ),
  mVisible( true )
{
}

SceneHolder::~SceneHolder()
{
  if ( mAdaptor )
  {
    mAdaptor->RemoveObserver( *mLifeCycleObserver.get() );
    mAdaptor->RemoveWindow( this );
    mAdaptor = nullptr;
  }
}

void SceneHolder::Add( Dali::Actor actor )
{
  if ( mScene )
  {
    mScene.Add( actor );
  }
}

void SceneHolder::Remove( Dali::Actor actor )
{
  if ( mScene )
  {
    mScene.Remove( actor );
  }
}

Dali::Layer SceneHolder::GetRootLayer() const
{
  return mScene ? mScene.GetRootLayer() : Dali::Layer();
}

uint32_t SceneHolder::GetId() const
{
  return mId;
}

std::string SceneHolder::GetName() const
{
  return mName;
}

bool SceneHolder::IsVisible() const
{
  return mVisible;
}

Dali::Integration::Scene SceneHolder::GetScene()
{
  return mScene;
}

void SceneHolder::SetSurface(Dali::RenderSurfaceInterface* surface)
{
  mSurface.reset( surface );

  mScene.SetSurface( *mSurface.get() );

  unsigned int dpiHorizontal, dpiVertical;
  dpiHorizontal = dpiVertical = 0;

  mSurface->GetDpi( dpiHorizontal, dpiVertical );
  mScene.SetDpi( Vector2( static_cast<float>( dpiHorizontal ), static_cast<float>( dpiVertical ) ) );

  mSurface->SetAdaptor( *mAdaptor );

  OnSurfaceSet( surface );
}

Dali::RenderSurfaceInterface* SceneHolder::GetSurface() const
{
  return mSurface.get();
}

void SceneHolder::SetBackgroundColor( Vector4 color )
{
  if ( mSurface )
  {
    mSurface->SetBackgroundColor( color );
  }
}

Vector4 SceneHolder::GetBackgroundColor() const
{
  return mSurface ? mSurface->GetBackgroundColor() : Vector4();
}

void SceneHolder::SetAdaptor(Dali::Adaptor& adaptor)
{
  // Avoid doing this more than once
  if( mAdaptorStarted )
  {
    return;
  }

  mAdaptorStarted = true;

  // Create the scene
  PositionSize positionSize = mSurface->GetPositionSize();
  mScene = Dali::Integration::Scene::New( Vector2( positionSize.width, positionSize.height ) );
  mScene.SetSurface( *mSurface.get() );

  Internal::Adaptor::Adaptor& adaptorImpl = Internal::Adaptor::Adaptor::GetImplementation( adaptor );
  mAdaptor = &adaptorImpl;

  // Create an observer for the adaptor lifecycle
  mAdaptor->AddObserver( *mLifeCycleObserver );

  if ( mSurface )
  {
    unsigned int dpiHorizontal, dpiVertical;
    dpiHorizontal = dpiVertical = 0;

    mSurface->GetDpi( dpiHorizontal, dpiVertical );
    mScene.SetDpi( Vector2( static_cast<float>( dpiHorizontal ), static_cast<float>( dpiVertical ) ) );

    mSurface->SetAdaptor( *mAdaptor );
  }

  OnAdaptorSet( adaptor );
}

void SceneHolder::Pause()
{
  OnPause();
}

void SceneHolder::Resume()
{
  OnResume();
}

}// Adaptor

}// Internal

} // Dali
