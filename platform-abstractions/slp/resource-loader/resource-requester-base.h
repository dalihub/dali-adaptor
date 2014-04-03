#ifndef __DALI_SLP_PLATFORM_RESOURCE_REQUESTER_BASE_H__
#define __DALI_SLP_PLATFORM_RESOURCE_REQUESTER_BASE_H__

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

#include <platform-abstractions/slp/resource-loader/resource-loader.h>
#include <dali/integration-api/resource-request.h>
#include <dali/integration-api/resource-cache.h>

namespace Dali
{
namespace Integration
{
class ResourceRequest;
}

namespace SlpPlatform
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

  /**
   * Pause starting new work on background threads, but keep that work queued.
   */
  virtual void Pause() = 0;

  /**
   * Resume processing tasks on background threads.
   */
  virtual void Resume() = 0;

  /**
   * Load a resource.
   * @param[in] request The resource request
   */
  virtual void LoadResource( Integration::ResourceRequest& request ) = 0;

  /**
   * Load a resource synchronously.
   * @param[in] type The type of resource
   * @param[in] path The path to the resource
   * @return A pointer to the resource
   */
  virtual Integration::ResourcePointer LoadResourceSynchronously( const Integration::ResourceType& type, const std::string& path ) = 0;

  /**
   * Load more resources (for partial loading)
   * @param[in] request The initial load request
   * @param[in] partialResource The resources loaded by the last request
   * @return LOADING or PARTIALLY_LOADED if more resources to come, COMPLETELY_LOADED if complete
   */
  virtual Integration::LoadStatus LoadFurtherResources( Integration::ResourceRequest& request, LoadedResource partialResource ) = 0;

  /**
   * Save a resource
   * @param[in] request The initial save request
   */
  virtual void SaveResource(const Integration::ResourceRequest& request ) = 0;

  /**
   * Cancal load requests
   * @param[in] id The request id of the loading request
   * @param[in] typeId The resource type id of the loading request
   */
  virtual void CancelLoad(Integration::ResourceId id, Integration::ResourceTypeId typeId) = 0;

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

} // SlpPlatform
} // Dali

#endif // __DALI_SLP_PLATFORM_RESOURCE_REQUESTER_BASE_H__
