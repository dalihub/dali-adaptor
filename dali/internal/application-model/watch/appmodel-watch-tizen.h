#ifndef APPMODEL_WATCH_H
#define APPMODEL_WATCH_H
/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class AppModelWatch
{
public: // Construction & Destruction
  /**
   * Constructor
   */
   AppModelWatch();

  /**
   * Destructor
   */
   ~AppModelWatch();

public:
    int AppMain(void* data);
    void AppExit();

private: // impl members
  struct Impl;
  Impl* mImpl;
};

}
}
}

#endif // APPMODEL_WATCH_H

