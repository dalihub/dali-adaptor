#ifndef DALI_IMAGE_LOADING_PLUGIN_H
#define DALI_IMAGE_LOADING_PLUGIN_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/image-loader-input.h>

namespace Dali
{

/**
 * @brief
 */
class ImageLoaderPlugin
{
public:

  /**
   * @brief Constructor.
   */
  ImageLoaderPlugin(){}

  /**
   * @brief Destructor.
   */
  virtual ~ImageLoaderPlugin(){}

  /**
   * @brief Get the image decorder
   * @param[in] filename The path to the resource.
   * @return BitmapLoader
   */
  virtual const ImageLoader::BitmapLoader* BitmapLoaderLookup( const std::string& filename ) const = 0;

  /**
   * @brief Function pointer called in adaptor to create a image loading plugin instance.
   * @return Pointer to the newly created plugin object
   */
  typedef ImageLoaderPlugin* CreateImageLoaderPlugin( void );

  /**
   * @brief Function pointer called in adaptor to destory a image loading plugin instance.
   */
  typedef void DestroyImageLoaderPlugin( ImageLoaderPlugin* plugin );

};

} // namespace Dali;

#endif
