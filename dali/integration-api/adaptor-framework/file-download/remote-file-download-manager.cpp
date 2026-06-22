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

// CLASS HEADER
#include <dali/integration-api/adaptor-framework/file-download/remote-file-download-manager.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/remote-file-download-manager-impl.h>

// EXTERNAL INCLUDES
#include <utility>

namespace Dali
{

RemoteFileDownloadManager::RemoteFileDownloadManager()
: BaseHandle()
{
}

RemoteFileDownloadManager::RemoteFileDownloadManager(Internal::Adaptor::RemoteFileDownloadManager* impl)
: BaseHandle(impl)
{
}

RemoteFileDownloadManager RemoteFileDownloadManager::Get()
{
  return Internal::Adaptor::RemoteFileDownloadManager::Get();
}

bool RemoteFileDownloadManager::IsAvailable()
{
  return Internal::Adaptor::RemoteFileDownloadManager::IsAvailable();
}

RemoteFileDownloadManager::RequestId RemoteFileDownloadManager::StartDownload(const std::string& url,
                                                                              size_t             maxSize,
                                                                              CompletionCallback callback)
{
  if(DALI_LIKELY(*this))
  {
    return GetImplementation(*this).StartDownload(url, maxSize, std::move(callback));
  }
  return INVALID_REQUEST_ID;
}

void RemoteFileDownloadManager::CancelDownload(RequestId requestId)
{
  if(DALI_LIKELY(*this))
  {
    GetImplementation(*this).CancelDownload(requestId);
  }
}

} // namespace Dali
