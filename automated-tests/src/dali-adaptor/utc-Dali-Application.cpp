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

#include <dali/dali.h>
#include <dali-test-suite-utils.h>

using namespace Dali;

void utc_dali_application_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_application_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{

struct MyTestApp : public ConnectionTracker
{
  MyTestApp( Application& app)
  : initCalled( false ),
    application( app )
  {
    application.InitSignal().Connect( this, &MyTestApp::Create );
  }

  void Create(Application& app)
  {
    initCalled = true;
  }

  void Quit()
  {
    application.Quit();
  }

  // Data
  bool initCalled;
  Application& application;
};

} // unnamed namespace
int UtcDaliApplicationNew(void)
{
  Application application = Application::New();

  MyTestApp testApp( application );

  DALI_TEST_CHECK( application );

  END_TEST;
}
