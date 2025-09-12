#ifndef APPMODEL_WIDGET_TIZEN_H
#define APPMODEL_WIDGET_TIZEN_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
/**
 * Plays feedback effects for Dali-Toolkit UI Controls.
 */
class AppModelWidget
{
public: // Construction & Destruction
        /**
         * Constructor
         */
  AppModelWidget();

  /**
   * Destructor
   */
  ~AppModelWidget();

public:
  int  AppMain(void* data);
  void AppExit();

private: // impl members
  struct Impl;
  Impl* mImpl;
};
} //namespace Adaptor
} //namespace Internal
} //namespace Dali

#endif