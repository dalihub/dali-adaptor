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

#include <dali/public-api/dali-core.h>

#include <native-buffer-plugin.h>
#include <adaptor.h>
#include <render-surface.h>
#include <orientation.h>
#include <timer.h>

using namespace Dali;

/*****************************************************************************
 * Test to see if the dali-adaptor is linking correctly.
 */

class LinkerApp : public ConnectionTracker
{
public:
  LinkerApp(NativeBufferPlugin &app)
  {
    app.InitSignal().Connect(this, &LinkerApp::Create);
  }

public:

  void Create(NativeBufferPlugin& app)
  {
  }
};

/*****************************************************************************/

int
main(int argc, char **argv)
{
  Any nullPtr;

  NativeBufferPlugin* plugin = new NativeBufferPlugin(400, 400, false, 2, RenderSurface::RENDER_30FPS);

  LinkerApp linkerApp (*plugin);

  plugin->Run();

  delete plugin;
  return 0;
}
