#ifndef __DALI_SLP_PLATFORM_RESOURCE_THREAD_IMAGE_H__
#define __DALI_SLP_PLATFORM_RESOURCE_THREAD_IMAGE_H__

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

#include <dali/integration-api/resource-cache.h>
#include <dali/integration-api/resource-types.h>
#include "resource-thread-base.h"

namespace Dali
{

namespace Integration
{
namespace Log
{
class Filter;
}
}

namespace SlpPlatform
{

class ResourceThreadImage : public ResourceThreadBase
{
public:
  /**
   * Constructor
   * @param[in] resourceLoader A reference to the ResourceLoader
   */
  ResourceThreadImage(ResourceLoader& resourceLoader);

  /**
   * Destructor
   */
  virtual ~ResourceThreadImage();

  /**
   * @copydoc ResourceLoader::LoadResourceSynchronously()
   * Note, this is not threaded, but is called synchronously.
   */
  Integration::ResourcePointer LoadResourceSynchronously( const Integration::ResourceType& resourceType, const std::string& resourcePath );

  /**
   * @copydoc ResourceLoader::GetClosestImageSize()
   * Note, this is not threaded, but is called synchronously.
   */
  void GetClosestImageSize( const std::string& filename,
                            const ImageAttributes& attributes,
                            Vector2 &closestSize );

  /**
   * @copydoc ResourceLoader::GetClosestImageSize()
   * Note, this is not threaded, but is called synchronously.
   */
  void GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                            const ImageAttributes& attributes,
                            Vector2 &closestSize );

private:
  /**
   * @copydoc ResourceThreadBase::Load
   */
  virtual void Load(const Integration::ResourceRequest& request);

  /**
   * @copydoc ResourceThreadBase::Decode
   */
  virtual void Decode(const Integration::ResourceRequest& request);

  /**
   *@copydoc ResourceThreadBase::Save
   */
  virtual void Save(const Integration::ResourceRequest& request);

  /**
   * Convert the file stream into in-memory image pixel data and metadata.
   * @param[in] resourceType The type of resource to convert.
   * @param[in] path The path to the resource.
   * @param[in] fp File Pointer. Closed on exit.
   * @param[out] ptr Pointer to write image data to
   * @return true on success, false on failure
   */
  bool ConvertStreamToBitmap( const Integration::ResourceType& resourceType,
                              std::string path,
                              FILE * const fp,
                              Integration::ImageDataPtr& ptr );
  /**
   * @brief Common code after a load or decode.
   *
   * @param[in] request   The request being attempted.
   * @param[in] result    Whether the conversion succeeded.
   * @param[in] imageData The pointer to receive the created image object.
   * @param[in] msg       Warning message to show on failure.
   */
  void HandleConversionResult( const Integration::ResourceRequest& request, bool result, Integration::ImageDataPtr imageData, const char * const msg );

}; // class ResourceThreadImage

} // namespace SlpPlatform

} // namespace Dali

#endif // __DALI_SLP_PLATFORM_RESOURCE_THREAD_IMAGE_H__
