#ifndef __DALI_SLP_PLATFORM_RESOURCE_TEXT_REQUESTER_H__
#define __DALI_SLP_PLATFORM_RESOURCE_TEXT_REQUESTER_H__

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

#include <platform-abstractions/slp/resource-loader/resource-requester-base.h>
#include "resource-thread-text.h"

namespace Dali
{
namespace Integration
{
class ResourceRequest;
}

namespace SlpPlatform
{

class ResourceTextRequester : public ResourceRequesterBase
{
public:
  /**
   * Constructor
   * @param[in] resourceLoader The resource loader with which to communicate results
   */
  ResourceTextRequester( ResourceLoader& resourceLoader );

  /**
   * Destructor
   */
  virtual ~ResourceTextRequester();

  /**
   * @copydoc ResourceRequester::Pause()
   */
  virtual void Pause();

  /**
   * @copydoc ResourceRequester::Resume()
   */
  virtual void Resume();

  /**
   * @copydoc ResourceRequester::LoadResource()
   */
  virtual void LoadResource( Integration::ResourceRequest& request );

  /**
   * @copydoc ResourceRequester::LoadResourceSynchronously()
   */
  virtual Integration::ResourcePointer LoadResourceSynchronously( const Integration::ResourceType& type, const std::string& path );

  /**
   * @copydoc ResourceRequester::LoadFurtherResources()
   */
  virtual Integration::LoadStatus LoadFurtherResources( Integration::ResourceRequest& request, LoadedResource partialResource );

  /**
   * @copydoc ResourceRequester::SaveResource()
   */
  virtual void SaveResource(const Integration::ResourceRequest& request );

  /**
   * @copydoc ResourceRequester::CancelLoad()
   */
  virtual void CancelLoad(Integration::ResourceId id, Integration::ResourceTypeId typeId);

private:

  /**
   * Create thread if it doesn't exist
   */
  void CreateThread();

protected:
  ResourceThreadText* mThreadText; ///< Loader thread
};

} // SlpPlatform
} // Dali

#endif // __DALI_SLP_PLATFORM_RESOURCE_TEXT_REQUESTER_H__
