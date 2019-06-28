#ifndef DALI_INTERNAL_COMMAND_LINE_OPTIONS_H
#define DALI_INTERNAL_COMMAND_LINE_OPTIONS_H

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

// EXTERNAL INCLUDES
#include <string>
#include <cstdint> // int32_t

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Parses the passed command line arguments and sets the values stored within this
 * class appropriately.
 *
 * The following options are supported:
 *
 * @code
 *  -w|--width          Stage Width
 *  -h|--height         Stage Height
 *  -d|--dpi            Emulated DPI
 *     --help           Help
 * @endcode
 *
 * When the above options are found, they are stripped from argv, and argc is updated appropriately.
 */
struct CommandLineOptions
{
public:

  /**
   * Constructor
   * @param[in,out]  argc  The number of arguments
   * @param[in,out]  argv  The argument list
   * @note Supported options are stripped from argv, and argc is updated appropriately.
   */
  CommandLineOptions( int32_t *argc, char **argv[] );

  /**
   * Destructor
   */
  ~CommandLineOptions();

public: // Command line parsed values

  int32_t stageWidth;      ///< The width of the stage required.  0 if not set.
  int32_t stageHeight;     ///< The height of the stage required.   0 if not set.
  std::string stageDPI;    ///< DPI stored as hxv, where h is horizontal DPI and v is vertical DPI
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_COMMAND_LINE_OPTIONS_H
