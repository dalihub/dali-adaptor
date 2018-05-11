#ifndef __DALI_TIZEN_PLATFORM_ABSTRACTION_H__
#define __DALI_TIZEN_PLATFORM_ABSTRACTION_H__

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

// EXTERNAL INCLUDES
#include <string>
#include <dali/integration-api/platform-abstraction.h>

namespace Dali
{

namespace TizenPlatform
{

class ResourceLoader;

/**
 * Concrete implementation of the platform abstraction class.
 */
class TizenPlatformAbstraction : public Integration::PlatformAbstraction
{

public: // Construction & Destruction

  /**
   * Constructor
   */
  TizenPlatformAbstraction();

  /**
   * Destructor
   */
  virtual ~TizenPlatformAbstraction();

public: // PlatformAbstraction overrides

  /**
   * @copydoc PlatformAbstraction::GetClosestImageSize()
   */
  virtual ImageDimensions GetClosestImageSize( const std::string& filename,
                                               ImageDimensions size,
                                               FittingMode::Type fittingMode,
                                               SamplingMode::Type samplingMode,
                                               bool orientationCorrection );

  /**
   * @copydoc PlatformAbstraction::GetClosestImageSize()
   */
  virtual ImageDimensions GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                                               ImageDimensions size,
                                               FittingMode::Type fittingMode,
                                               SamplingMode::Type samplingMode,
                                               bool orientationCorrection );

  /**
   * @copydoc PlatformAbstraction::LoadImageSynchronously()
   */
  virtual Integration::ResourcePointer LoadImageSynchronously(const Integration::BitmapResourceType& resource, const std::string& resourcePath);

  /**
   * @copydoc PlatformAbstraction::DecodeBuffer()
   */
  virtual Integration::BitmapPtr DecodeBuffer( const Integration::BitmapResourceType& resource, uint8_t * buffer, size_t size );

  /**
   * @copydoc PlatformAbstraction::LoadShaderBinaryFile()
   */
  virtual bool LoadShaderBinaryFile( const std::string& filename, Dali::Vector< unsigned char >& buffer ) const;

  /**
   * @copydoc PlatformAbstraction::SaveShaderBinaryFile()
   */
  virtual bool SaveShaderBinaryFile( const std::string& filename, const unsigned char * buffer, unsigned int numBytes ) const;

  /**
   * Sets path for data/resource storage.
   * @param[in] path data/resource storage path
   */
  void SetDataStoragePath( const std::string& path );

private:

  TizenPlatformAbstraction( const TizenPlatformAbstraction& ); ///< Undefined
  TizenPlatformAbstraction& operator=( const TizenPlatformAbstraction& ); ///< Undefined

  std::string mDataStoragePath;

};

/**
 * Construct a platform abstraction and return it.
 * @return TizenPlatformAbstraction instance
 */
TizenPlatformAbstraction* CreatePlatformAbstraction();

/**
 * Save a file to disk
 * @param filename to create
 * @param buffer to store
 * @param numBytes to store
 * @return true if successful, false otherwise
 */
bool SaveFile( const std::string& filename, const unsigned char * buffer, unsigned int numBytes );

}  // namespace TizenPlatform

}  // namespace Dali

#endif // __DALI_TIZEN_PLATFORM_ABSTRACTION_H__
