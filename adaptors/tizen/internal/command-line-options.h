#ifndef __DALI_INTERNAL_COMMAND_LINE_OPTIONS_H__
#define __DALI_INTERNAL_COMMAND_LINE_OPTIONS_H__

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

// EXTERNAL INCLUDES
#include <string>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Parses the passed command line arguments and sets the values stored within this
 * class appropriately.
 */
struct CommandLineOptions
{
public:

  /**
   * Constructor
   * @param[in]  argc  The number of arguments
   * @param[in]  argv  The argument list
   */
  CommandLineOptions(int argc, char *argv[]);

  /**
   * Destructor
   */
  ~CommandLineOptions();

public: // Command line parsed values

  int noVSyncOnRender; ///< If 1, then the user does not want VSync on Render
  int stageWidth; ///< The width of the stage required.  0 if not set.
  int stageHeight; ///< The height of the stage required.   0 if not set.
  std::string stageDPI; ///< DPI stored as hxv, where h is horizontal DPI and v is vertical DPI
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_COMMAND_LINE_OPTIONS_H__
