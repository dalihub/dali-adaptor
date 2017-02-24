#ifndef __DALI_TIZEN_PLATFORM_RESOURCE_BITMAP_REQUESTER_H__
#define __DALI_TIZEN_PLATFORM_RESOURCE_BITMAP_REQUESTER_H__

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

#include "resource-requester-base.h"

namespace Dali
{
namespace TizenPlatform
{

/**
 * Class to own request threads and manage resource requests for bitmaps
 */
class ResourceBitmapRequester : public ResourceRequesterBase
{
public:
  /**
   * Constructor
   * @param[in] resourceLoader The resource loader with which to communicate results
   */
  ResourceBitmapRequester( ResourceLoader& resourceLoader );

  /**
   * Destructor
   */
  virtual ~ResourceBitmapRequester();

private:
};

} // TizenPlatform
} // Dali

#endif // __DALI_TIZEN_PLATFORM_RESOURCE_BITMAP_REQUESTER_H__
