#ifndef __DALI_SLP_PLATFORM_RESOURCE_BITMAP_REQUESTER_H__
#define __DALI_SLP_PLATFORM_RESOURCE_BITMAP_REQUESTER_H__

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
#include "resource-thread-image.h"
#include "resource-thread-distance-field.h"

namespace Dali
{
namespace SlpPlatform
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

  /**
   * @copydoc ResourceLoader::LoadResourceSynchronously()
   */
  Integration::ResourcePointer LoadResourceSynchronously( const Integration::ResourceType& resourceType, const std::string& resourcePath );

  /**
   * @copydoc PlatformAbstraction::GetClosestImageSize()
   */
  void GetClosestImageSize( const std::string& filename,
                            const ImageAttributes& attributes,
                            Vector2 &closestSize );

  /**
   * @copydoc PlatformAbstraction::GetClosestImageSize()
   */
  void GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                            const ImageAttributes& attributes,
                            Vector2 &closestSize );


protected:
  ResourceThreadImage*          mThreadImage;           ///< Image loader thread object
  ResourceThreadDistanceField*  mThreadDistanceField;   ///< Distance field generator thread.
};

} // SlpPlatform
} // Dali

#endif // __DALI_SLP_PLATFORM_RESOURCE_BITMAP_REQUESTER_H__
