//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include "command-line-options.h"

// EXTERNAL INCLUDES
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
  const char * RENDER_NO_VSYNC_OPTION = "no-vsync";
  const char * RENDER_NO_VSYNC_DETAIL = "Disable VSync on Render";

  const char * RENDER_WIDTH_OPTION = "width";
  const char * RENDER_WIDTH_DETAIL = "Stage Width";

  const char * RENDER_HEIGHT_OPTION = "height";
  const char * RENDER_HEIGHT_DETAIL = "Stage Height";

  const char * RENDER_DPI_OPTION = "dpi";
  const char * RENDER_DPI_DETAIL = "Emulated DPI";

  const char * USAGE = "help";

  struct Arguments
  {
    const char * opt;
    const char * optDescription;

    void Print()
    {
      std::cout << "--" << opt << " " << optDescription;
    }
  };

  Arguments arguments[] =
  {
    { RENDER_NO_VSYNC_OPTION, RENDER_NO_VSYNC_DETAIL },
    { RENDER_WIDTH_OPTION,    RENDER_WIDTH_DETAIL    },
    { RENDER_HEIGHT_OPTION,   RENDER_HEIGHT_DETAIL   },
    { RENDER_DPI_OPTION,      RENDER_DPI_DETAIL      },
    { NULL, NULL }
  };

} // unnamed namespace


CommandLineOptions::CommandLineOptions(int argc, char *argv[])
: noVSyncOnRender(0),
  stageWidth(0), stageHeight(0)
{
  const struct option options[]=
  {
    { arguments[0].opt, no_argument, &noVSyncOnRender, 1 },  // "--no-vsync"
    { arguments[1].opt, required_argument, NULL,     'w' },  // "--width"
    { arguments[2].opt, required_argument, NULL,     'h' },  // "--height"
    { arguments[3].opt, required_argument, NULL,     'd' },  // "--dpi"
    { USAGE, no_argument, NULL, 'u'},
    { 0, 0, 0, 0 } // end of options
  };


  int opt(0);
  int optIndex(0);

  const char* optString = "w:h:d:?";

  do
  {
    opt = getopt_long(argc, argv, optString, options, &optIndex);

    switch (opt)
    {
      case 0:
        // if setting vsync getopt set flag already
        break;
      case 'w':
        stageWidth = atoi(optarg);
        break;
      case 'h':
        stageHeight = atoi(optarg);
        break;
      case 'd':
        stageDPI.assign(optarg);
        break;
      case 'u':
        // show usage
        std::cout << "Available options:" << std::endl;
        for ( int i = 0; arguments[i].opt != NULL; ++i)
        {
          arguments[i].Print();
          std::cout << std::endl;
        }
        break;
      default:
        // -1 will exit here (no more options to parse)
        break;
    }
  }
  while (opt != -1);

  // Ignores any unknown options
}

CommandLineOptions::~CommandLineOptions()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
