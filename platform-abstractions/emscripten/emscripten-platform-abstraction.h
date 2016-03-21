#ifndef __DALI_EMSCRIPTEN_PLATFORM_ABSTRACTION_H__
#define __DALI_EMSCRIPTEN_PLATFORM_ABSTRACTION_H__

/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <set>
#include <stdint.h>
#include <cstring>
#include <queue>

// INTERNAL INCLUDES
#include <dali/integration-api/platform-abstraction.h>
#include <dali/integration-api/bitmap.h>

namespace Dali
{

/**
 * An Dali Platform abstraction using libSDL for Emscripten.
 *
 * Emscripten already wraps the SDL API so we can use this API to create windows/canvas in the browser
 *
 */
class DALI_IMPORT_API EmscriptenPlatformAbstraction : public Dali::Integration::PlatformAbstraction
{

public:

  struct Resources
  {
    bool                         loaded;
    Integration::ResourceId      loadedId;
    Integration::ResourceTypeId  loadedType;
    Integration::ResourcePointer loadedResource;

    bool                         loadFailed;
    Integration::ResourceId      loadFailedId;
    Integration::ResourceFailure loadFailure;

    bool                         saved;
    Integration::ResourceId      savedId;
    Integration::ResourceTypeId  savedType;

    bool                         saveFailed;
    Integration::ResourceId      saveFailedId;
    Integration::ResourceFailure saveFailure;
  };

  struct LoadFileResult
  {
    LoadFileResult()
    : loadResult(false)
    {

    }

    bool loadResult;
    std::vector< unsigned char> buffer;
  };


  /**
   * Constructor
   */
  EmscriptenPlatformAbstraction();


  /**
   * Destructor
   */
  virtual ~EmscriptenPlatformAbstraction();

  /**
   * @copydoc PlatformAbstraction::GetTimeMicroseconds()
   */
  virtual void GetTimeMicroseconds(unsigned int &seconds, unsigned int &microSeconds);

  void IncrementGetTimeResult(size_t milliseconds);

  /**
   * @copydoc PlatformAbstraction::Suspend()
   */
  virtual void Suspend();

  /**
   * @copydoc PlatformAbstraction::Resume()
   */
  virtual void Resume();

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
   * @copydoc PlatformAbstraction::LoadResource()
   */
  virtual void LoadResource(const Integration::ResourceRequest& request);

  virtual Integration::ResourcePointer LoadResourceSynchronously( const Integration::ResourceType& resourceType, const std::string& resourcePath );

  /**
   * @copydoc PlatformAbstraction::SaveResource()
   */
  virtual void SaveResource(const Integration::ResourceRequest& request);

  /**
   * @copydoc PlatformAbstraction::DecodeBuffer()
   */
  virtual Integration::BitmapPtr DecodeBuffer( const Integration::ResourceType& resourceType, uint8_t * buffer, size_t bufferSize );

  /**
   * @copydoc PlatformAbstraction::CancelLoad()
   */
  virtual void CancelLoad(Integration::ResourceId id, Integration::ResourceTypeId typeId);

  /**
   * @copydoc PlatformAbstraction::GetResources()
   */
  virtual void GetResources(Integration::ResourceCache& cache);

  /**
   * @copydoc PlatformAbstraction::IsLoading()
   */
  virtual bool IsLoading();

  /**
   * @copydoc PlatformAbstraction::GetDefaultFontFamily()
   */
  virtual const std::string& GetDefaultFontFamily() const;

  /**
   * @copydoc PlatformAbstraction::GetDefaultFontSize()
   */
  virtual int GetDefaultFontSize() const;

  /**
   * Sets horizontal and vertical pixels per inch value that is used by the display
   * @param[in] dpiHorizontal horizontal dpi value
   * @param[in] dpiVertical   vertical dpi value
   */
  virtual void SetDpi (unsigned int dpiHorizontal, unsigned int dpiVertical);

  /**
   * @copydoc PlatformAbstraction::LoadFile()
   */
  virtual bool LoadFile( const std::string& filename, Dali::Vector< unsigned char >& buffer ) const;

  /**
   * @copydoc PlatformAbstraction::SaveFile()
   */
  virtual bool SaveFile( const std::string& filename, const unsigned char * buffer, unsigned int numBytes ) const;

  /**
   * @copydoc PlatformAbstraction::JoinLoaderThreads()
   */
  virtual void JoinLoaderThreads();

  /**
   * @copydoc PlatformAbstraction::LoadShaderBinaryFile()
   */
  virtual bool LoadShaderBinaryFile( const std::string& filename, Dali::Vector< unsigned char >& buffer ) const;

  /**
   * @copydoc PlatformAbstraction::SaveShaderBinaryFile()
   */
  virtual bool SaveShaderBinaryFile( const std::string& filename, const unsigned char * buffer, unsigned int numBytes ) const;

  /**
   * @copydoc PlatformAbstraction::UpdateDefaultsFromDevice()
   */
  virtual void UpdateDefaultsFromDevice();

private:
  std::string                   mGetDefaultFontFamilyResult;
  Resources                     mResources;
  Vector2                       mSize;

  LoadFileResult                mLoadFileResult;

  typedef std::pair< Integration::ResourceId, Integration::BitmapPtr > ResourceIdBitmapPair;
  std::queue<ResourceIdBitmapPair> mResourceQueue;

};

} // Dali

#endif /* __DALI_TET_PLATFORM_ABSTRACTION_H__ */
