#ifndef __DALI_TIZEN_PLATFORM_RESOURCE_REQUESTER_BASE_H__
#define __DALI_TIZEN_PLATFORM_RESOURCE_REQUESTER_BASE_H__

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

#include <platform-abstractions/tizen/resource-loader/resource-loader.h>

namespace Dali
{
namespace TizenPlatform
{

/**
 * Base class to handle resource requests
 */
class ResourceRequesterBase
{
public:
  /**
   * Constructor
   * @param[in] resourceLoader The resource loader with which to communicate results
   */
  ResourceRequesterBase( ResourceLoader& resourceLoader );

  /**
   * Destructor
   */
  virtual ~ResourceRequesterBase();


protected:
  ResourceLoader& mResourceLoader; ///< The resource loader to which to send results

private:
  /**
   * Undefined default constructor
   */
  ResourceRequesterBase();

  /**
   * Undefined Copy constructor
   */
  ResourceRequesterBase(const ResourceRequesterBase& rhs );

  /**
   * Undefined Assignment operator
   */
  ResourceRequesterBase& operator=(const ResourceRequesterBase& rhs );
};

} // TizenPlatform
} // Dali

#endif // __DALI_TIZEN_PLATFORM_RESOURCE_REQUESTER_BASE_H__
