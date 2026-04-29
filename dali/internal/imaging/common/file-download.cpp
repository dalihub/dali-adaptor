/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// HEADER
#include <dali/internal/imaging/common/file-download.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/file-download/file-download-plugin-proxy.h>

namespace Dali
{
namespace TizenPlatform
{
namespace Network
{

bool DownloadRemoteFileIntoMemory(const std::string&     url,
                                  Dali::Vector<uint8_t>& dataBuffer,
                                  size_t&                dataSize,
                                  size_t                 maximumAllowedSizeBytes)
{
  return Dali::FileDownloadPluginProxy::DownloadRemoteFileIntoMemory(url, dataBuffer, dataSize, maximumAllowedSizeBytes);
}

} // namespace Network

} // namespace TizenPlatform

} // namespace Dali
