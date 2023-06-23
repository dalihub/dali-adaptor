/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
 */

#import <Cocoa/Cocoa.h>

#include "extern-definitions.h"

// CLASS HEADER
#include <dali/internal/adaptor/macos/framework-mac.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

/// Application Status Enum
enum
{
  APP_CREATE,
  APP_TERMINATE,
  APP_PAUSE,
  APP_RESUME,
  APP_RESET,
  APP_LANGUAGE_CHANGE,
};

} // Unnamed namespace

FrameworkMac::FrameworkMac(Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: Framework(observer, taskObserver, argc, argv, type, useUiThread)
{
  // ensures the NSApp global object is initialized
  [NSApplication sharedApplication];

  // this is needed for applications without a bundle
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  // make sure we can become the key window
  [NSApp activateIgnoringOtherApps:YES];
}

FrameworkMac::~FrameworkMac()
{
  if (mRunning)
  {
    Quit();
  }
}

void FrameworkMac::Run()
{
  mRunning = true;
  AppStatusHandler(APP_CREATE, nullptr);
  [NSApp run];
  mRunning = false;
}

void FrameworkMac::Quit()
{
  AppStatusHandler(APP_TERMINATE, nullptr);
}

bool FrameworkMac::AppStatusHandler(int type, void *)
{
  switch (type)
  {
    case APP_CREATE:
    {
      mObserver.OnInit();
      break;
    }

    case APP_RESET:
      mObserver.OnReset();
      break;

    case APP_RESUME:
      mObserver.OnResume();
      break;

    case APP_TERMINATE:
      mObserver.OnTerminate();
      break;

    case APP_PAUSE:
      mObserver.OnPause();
      break;

    case APP_LANGUAGE_CHANGE:
      mObserver.OnLanguageChanged();
      break;

    default:
      break;
  }

  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
