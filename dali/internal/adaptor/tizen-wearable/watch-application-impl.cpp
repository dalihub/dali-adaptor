/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <dali/internal/adaptor/tizen-wearable/watch-application-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

WatchApplicationPtr WatchApplication::New(
  int* argc,
  char **argv[],
  const std::string& stylesheet,
  Dali::WatchApplication::WINDOW_MODE windowMode)
{
  WatchApplicationPtr watch ( new WatchApplication (argc, argv, stylesheet, windowMode ) );
  return watch;
}

WatchApplication::WatchApplication( int* argc, char** argv[], const std::string& stylesheet, Dali::Application::WINDOW_MODE windowMode )
: Application(argc, argv, stylesheet, windowMode, PositionSize(), Framework::WATCH),
  mState( UNINITIALIZED )
{
}

WatchApplication::~WatchApplication()
{
}

void WatchApplication::OnInit()
{
  Application::OnInit();

  Dali::Adaptor::Get().SetRenderRefreshRate( 2 ); // make 30 fps for watch applications

  mState = INITIALIZED;
}

void WatchApplication::OnTerminate()
{
  Application::OnTerminate();

  mState = TERMINATED;
}

void WatchApplication::OnResume()
{
  Application::OnResume();

  mState = RESUMED;
}

void WatchApplication::OnPause()
{
  Application::OnPause();

  mState = PAUSED;
}

void WatchApplication::OnTimeTick(WatchTime& time)
{
  Dali::WatchApplication watch(this);
  mTickSignal.Emit( watch, time );

  if(mState == PAUSED)
  {
    // This is a pre-resume scenario. All rendering engine of tizen SHOULD forcely update once at this time.
    Internal::Adaptor::Adaptor::GetImplementation( GetAdaptor() ).RequestUpdateOnce();
  }

  // A watch application will queue messages to update the UI in the signal emitted above
  // Process these immediately to avoid a blinking issue where the old time is briefly visible
  CoreEventInterface& eventInterface = Internal::Adaptor::Adaptor::GetImplementation( GetAdaptor() );
  eventInterface.ProcessCoreEvents();
}

void WatchApplication::OnAmbientTick(WatchTime& time)
{
  Dali::WatchApplication watch(this);
  mAmbientTickSignal.Emit( watch, time );

  // A watch application will queue messages to update the UI in the signal emitted above
  // Process these immediately to avoid a blinking issue where the old time is briefly visible
  CoreEventInterface& eventInterface = Internal::Adaptor::Adaptor::GetImplementation( GetAdaptor() );
  eventInterface.ProcessCoreEvents();
}

void WatchApplication::OnAmbientChanged(bool ambient)
{
  Dali::WatchApplication watch(this);
  mAmbientChangeSignal.Emit( watch, ambient );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
