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
#include <vector>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
struct Argument
{
  const char * const opt;
  const char * const optDescription;

  void Print()
  {
    std::cout << std::left << "  --";
    std::cout.width( 15 );
    std::cout << opt;
    std::cout << optDescription;
    std::cout << std::endl;
  }
};

Argument EXPECTED_ARGS[] =
{
  { "no-vsync", "Disable VSync on Render" },
  { "width",    "Stage Width"             },
  { "height",   "Stage Height"            },
  { "dpi",      "Emulated DPI"            },
  { "help",     "Help"                    },

  { NULL,       NULL                      }
};

enum Option
{
  OPTION_NO_VSYNC = 0,
  OPTION_STAGE_WIDTH,
  OPTION_STAGE_HEIGHT,
  OPTION_DPI,
  OPTION_HELP
};

typedef std::vector< int > UnhandledContainer;

} // unnamed namespace

CommandLineOptions::CommandLineOptions(int *argc, char **argv[])
: noVSyncOnRender(0),
  stageWidth(0), stageHeight(0)
{
  if ( *argc > 1 )
  {
    // We do not want to print out errors.
    int origOptErrValue( opterr );
    opterr = 0;

    int help( 0 );

    const struct option options[]=
    {
      { EXPECTED_ARGS[OPTION_NO_VSYNC].opt,     no_argument,       &noVSyncOnRender, 1   },  // "--no-vsync"
      { EXPECTED_ARGS[OPTION_STAGE_WIDTH].opt,  required_argument, NULL,             'w' },  // "--width"
      { EXPECTED_ARGS[OPTION_STAGE_HEIGHT].opt, required_argument, NULL,             'h' },  // "--height"
      { EXPECTED_ARGS[OPTION_DPI].opt,          required_argument, NULL,             'd' },  // "--dpi"
      { EXPECTED_ARGS[OPTION_HELP].opt,         no_argument,       &help,            1   },  // "--help"
      { 0, 0, 0, 0 } // end of options
    };

    int shortOption( 0 );
    int optionIndex( 0 );

    const char* optString = "-w:h:d:"; // The '-' ensures that argv is NOT permuted
    bool optionProcessed( false );

    UnhandledContainer unhandledOptions; // We store indices of options we do not handle here

    do
    {
      shortOption = getopt_long( *argc, *argv, optString, options, &optionIndex );

      switch ( shortOption )
      {
        case 0:
        {
          // Check if we want help
          if ( help )
          {
            std::cout << "Available options:" << std::endl;
            Argument* arg = EXPECTED_ARGS;
            while ( arg->opt )
            {
              arg->Print();
              ++arg;
            }
            optionProcessed = true;
          }
          break;
        }

        case 'w':
        {
          if ( optarg )
          {
            stageWidth = atoi( optarg );
            optionProcessed = true;
          }
          break;
        }

        case 'h':
        {
          if ( optarg )
          {
            stageHeight = atoi( optarg );
            optionProcessed = true;
          }
          break;
        }

        case 'd':
        {
          if ( optarg )
          {
            stageDPI.assign( optarg );
            optionProcessed = true;
          }
          break;
        }

        case -1:
        {
          // All command-line options have been parsed.
          break;
        }

        default:
        {
          unhandledOptions.push_back( optind - 1 );
          break;
        }
      }
    } while ( shortOption != -1 );

    // Take out the options we have processed
    if ( optionProcessed )
    {
      if ( !unhandledOptions.empty() )
      {
        int index( 1 );

        // Overwrite the argv with the values from the unhandled indices
        const UnhandledContainer::const_iterator endIter = unhandledOptions.end();
        for ( UnhandledContainer::iterator iter = unhandledOptions.begin(); iter != endIter; ++iter )
        {
          (*argv)[ index++ ] = (*argv)[ *iter ];
        }
        *argc = unhandledOptions.size() + 1; // +1 for the program name
      }
      else
      {
        // There are no unhandled options, so we should just have the program name
        *argc = 1;
      }

      optind = 1; // Reset to start
    }

    opterr = origOptErrValue; // Reset opterr value.
  }
}

CommandLineOptions::~CommandLineOptions()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
