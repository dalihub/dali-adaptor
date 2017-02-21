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

// CLASS HEADER
#include "resource-bitmap-requester.h"

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include "network/file-download.h"
#include "network/http-utils.h"

using namespace Dali::Integration;

namespace Dali
{
namespace TizenPlatform
{
namespace
{
enum ResourceScheme
{
  FILE_SYSTEM_RESOURCE,
  NETWORK_RESOURCE
};
}// unnamed namespace

ResourceBitmapRequester::ResourceBitmapRequester( ResourceLoader& resourceLoader )
: ResourceRequesterBase( resourceLoader )
{
}

ResourceBitmapRequester::~ResourceBitmapRequester()
{
}



} // TizenPlatform
} // Dali
