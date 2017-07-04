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
#include "watch-application-impl.h"

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
: Application(argc, argv, stylesheet, windowMode, PositionSize(), Framework::WATCH)
{
}

WatchApplication::~WatchApplication()
{
}

void WatchApplication::OnTimeTick(WatchTime& time)
{
  Dali::WatchApplication watch(this);
  mTickSignal.Emit( watch, time );
}

void WatchApplication::OnAmbientTick(WatchTime& time)
{
  Dali::WatchApplication watch(this);
  mAmbientTickSignal.Emit( watch, time );
}

void WatchApplication::OnAmbientChanged(bool ambient)
{
  Dali::WatchApplication watch(this);
  mAmbientChangeSignal.Emit( watch, ambient );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
