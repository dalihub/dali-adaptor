#ifndef __DALI_SLP_PLATFORM_RESOURCE_LOADER_H__
#define __DALI_SLP_PLATFORM_RESOURCE_LOADER_H__

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

#include <dali/integration-api/platform-abstraction.h>
#include <dali/integration-api/glyph-set.h>
#include <dali/integration-api/resource-cache.h>
#include <dali/public-api/common/dali-vector.h>

#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace Dali
{

namespace Integration
{
namespace Log
{
class Filter;
}
}

namespace Platform
{
class FontController;
}

namespace SlpPlatform
{

/**
 * Contains information about a successfully loaded resource
 */
struct LoadedResource
{
  /**
   * Constructor
   * @param[in] loadedId        The ID of the resource
   * @param[in] loadedType      The resource type
   * @param[in] loadedResource  A pointer to the loaded resource data
   */
  LoadedResource(Integration::ResourceId      loadedId,
                 Integration::ResourceTypeId  loadedType,
                 Integration::ResourcePointer loadedResource)
  : id(loadedId),
    type(loadedType),
    resource(loadedResource)
  {
  }

  /// Copy constructor
  LoadedResource(const LoadedResource& loaded)
  : id(loaded.id),
    type(loaded.type),
    resource(loaded.resource)
  {
  }

  /// Assignment operator
  LoadedResource& operator=(const LoadedResource& rhs)
  {
    if( this != &rhs )
    {
      id = rhs.id;
      type = rhs.type;
      resource = rhs.resource;
    }
    return *this;
  }

  Integration::ResourceId      id;         ///< Integer ID
  Integration::ResourceTypeId  type;       ///< Type enum (bitmap, shader, ...)
  Integration::ResourcePointer resource;   ///< Reference counting pointer to the loaded / decoded representation  of the resource.
};

/**
 * Contains information about a successfully saved resource
 */
struct SavedResource
{
  /**
   * Constructor
   * @param[in] savedId   The ID of the resource
   * @param[in] savedType The resource type
   */
  SavedResource(Integration::ResourceId     savedId,
                Integration::ResourceTypeId savedType)
  : id(savedId),
    type(savedType)
  {
  }

  /// Copy constructor
  SavedResource(const LoadedResource& loaded)
  : id(loaded.id),
    type(loaded.type)
  {
  }

  /// Assignment operator
  SavedResource& operator=(const SavedResource& rhs)
  {
    if( this != &rhs )
    {
      id = rhs.id;
      type = rhs.type;
    }
    return *this;
  }

  Integration::ResourceId     id;
  Integration::ResourceTypeId type;
};

/**
 * Contains information about a failed resource load/save request
 */
struct FailedResource
{
  FailedResource(Integration::ResourceId resourceId, Integration::ResourceFailure failure):
    id(resourceId),
    failureType(failure)
  {
  }

  /// Copy constructor
  FailedResource(const FailedResource& failed)
  : id(failed.id),
    failureType(failed.failureType)
  {
  }

  /// Assignment operator
  FailedResource& operator=(const FailedResource& rhs)
  {
    if( this != &rhs )
    {
      id = rhs.id;
      failureType = rhs.failureType;
    }
    return *this;
  }

  Integration::ResourceId      id;
  Integration::ResourceFailure failureType;
};

/**
 * This implements the resource loading part of the PlatformAbstraction API.
 * The requests for a specific resource type are farmed-out to a resource
 * requester for that type which handles them in their own dedicated loading
 * threads.
 */
class ResourceLoader
{
public:

  /**
   * Create a resource loader.
   * There should exactly one of these objects per Dali Core.
   */
  ResourceLoader();

  /**
   * Non-virtual destructor.
   * ResourceLoader is NOT intended as a base class.
   */
  ~ResourceLoader();

  /**
   * Pause processing of already-queued resource requests.
   */
  void Pause();

  /**
   * Continue processing resource requests.
   */
  void Resume();

  /**
   * Check if the ResourceLoader is terminating
   * @return true if terminating else false
   */
  bool IsTerminating();

  /**
   * Add a partially loaded resource to the PartiallyLoadedResource queue
   * @param[in] resource The resource's information and data
   */
  void AddPartiallyLoadedResource(LoadedResource& resource);

  /**
   * Add a completely loaded resource to the LoadedResource queue
   * @param[in] resource The resource's information and data
   */
  void AddLoadedResource(LoadedResource& resource);

  /**
   * Add a successfully saved resource to the SavedResource queue
   * @param[in] resource The resource's information
   */
  void AddSavedResource(SavedResource& resource);

  /**
   * Add information about a failed resource load to the FailedLoads queue
   * @param[in] resource The failed resource's information
   */
  void AddFailedLoad(FailedResource& resource);

  /**
   * Add information about a failed resource save to the FailedSaves queue
   * @param[in] resource The failed resource's information
   */
  void AddFailedSave(FailedResource& resource);


  // From PlatformAbstraction

  /**
   * @copydoc PlatformAbstraction::LoadResource()
   */
  void LoadResource(const Integration::ResourceRequest& request);

  /**
   * @copydoc PlatformAbstraction::LoadResourceSynchronously()
   */
  Integration::ResourcePointer LoadResourceSynchronously( const Integration::ResourceType& resourceType, const std::string& resourcePath);

  /**
   * @copydoc PlatformAbstraction::SaveResource()
   */
  void SaveResource(const Integration::ResourceRequest& request);

  /**
   * @copydoc PlatformAbstraction::CancelLoad()
   */
  void CancelLoad(Integration::ResourceId id, Integration::ResourceTypeId typeId);

  /**
   * @copydoc PlatformAbstraction::IsLoading()
   */
  bool IsLoading();

  /**
   * @copydoc PlatformAbstraction::GetResources()
   */
  void GetResources(Integration::ResourceCache& cache);

  /**
   * @copydoc PlatformAbstraction::GetClosestImageSize()
   */
  void GetClosestImageSize( const std::string& filename,
                            const ImageAttributes& attributes,
                            Vector2& closestSize );

  /**
   * @copydoc PlatformAbstraction::GetClosestImageSize()
   */
  void GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                            const ImageAttributes& attributes,
                            Vector2& closestSize );

  /**
   * Called by Font objects to synchronously query glyph data.
   * @param[in] textRequest     resource request
   * @param[in] freeType        handle to the FreeType library
   * @param[in] fontFamily      name of the font's family
   * @param[in] getBitmap       whether to load bitmaps for the symbols as well
   * @return A GlyphSet pointer with a list of the requested glyph metrics.
   */
  Integration::GlyphSet* GetGlyphData ( const Integration::TextResourceType& textRequest,
                                        FT_Library freeType,
                                        const std::string& fontFamily,
                                        bool getBitmap);

  /**
   * Called by Font objects to synchronously load glyph data.
   * @param[in] textRequest     resource request
   * @param[in] fontFamily      name of the font's family
   * @return A GlyphSet pointer containing the requested glyph bitmaps.
   */
  Integration::GlyphSet* GetCachedGlyphData( const Integration::TextResourceType& textRequest,
                                             const std::string& fontFamily );

  /**
   * Called by Font objects to synchronously query global font metrics.
   * @param[in] freeType       handle to the FreeType library
   * @param[in] fontFamily     name of the font's family
   * @param[in] fontStyle      name of the font's style
   * @param[out] globalMetrics font requested global metrics.
   */
  void GetGlobalMetrics( FT_Library freeType,
                         const std::string& fontFamily,
                         const std::string& fontStyle,
                         Integration::GlobalMetrics& globalMetrics );

  /**
   * @copydoc PlatformAbstraction::SetDpi()
   */
  void SetDpi(unsigned int dpiHor, unsigned int dpiVer);

  /**
   * @copydoc PlatformAbstraction::GetFontFamilyForChars()
   */
  const std::string& GetFontFamilyForChars(const TextArray& charsRequested);

  /**
   * @copydoc PlatformAbstraction::AllGlyphsSupported()
   */
  bool AllGlyphsSupported(const std::string& fontFamily, const std::string& fontStyle, const TextArray& charsRequested);

  /**
   * @copydoc PlatformAbstraction::ValidateFontFamilyName()
   */
  bool ValidateFontFamilyName( const std::string& fontFamily,
                               const std::string& fontStyle,
                               bool& isDefaultSystemFontFamily,
                               bool& isDefaultSystemFontStyle,
                               std::string& closestFontFamilyMatch,
                               std::string& closestFontStyleMatch );

  /**
   * @copydoc SlpPlatformAbstraction::GetFontLineHeightFromCapsHeight
   */
  PixelSize GetFontLineHeightFromCapsHeight(const std::string& fontFamily, const std::string& fontStyle, CapsHeight capsHeight, FT_Library freeType);

  /**
   * @copydoc SlpPlatformAbstraction::GetFontList
   */
  void GetFontList( Dali::Integration::PlatformAbstraction::FontListMode mode, std::vector<std::string>& fontList );

  /**
   * @copydoc SlpPlatformAbstraction::LoadFile()
   */
  bool LoadFile( const std::string& filename, std::vector< unsigned char >& buffer ) const;

  /**
   * @copydoc SlpPlatformAbstraction::LoadFile()
   */
  std::string LoadFile(const std::string& filename) const;

  /**
   * @copydoc SlpPlatformAbstraction::SaveFile()
   */
  static bool SaveFile(const std::string& filename, std::vector< unsigned char >& buffer);

  /**
   * Sets the default font family that should be used by the font resources.
   * @param[in] fontFamily The default font family.
   * @param[in] fontStyle The default font style.
   */
  void SetDefaultFontFamily( const std::string& fontFamily, const std::string& fontStyle );

  /**
   * Retrieves the glyp's image representing the given character.
   *
   * @param[in] freeType Handle to the FreeType library.
   * @param[in] fontFamily The font's family name.
   * @param[in] fontStyle The font's style.
   * @param[in] fontSize The font's size.
   * @param[in] character The given character.
   *
   * @return The bitmap image.
   */
  Integration::BitmapPtr GetGlyphImage( FT_Library freeType, const std::string& fontFamily, const std::string& fontStyle, float fontSize, uint32_t character );

private:

  /**
   * Find the path of a platform-specific font resource.
   * Multi-threading note: this method will be called from the main thread only i.e. not
   * from within the Core::Render() method.
   * @note   Italics and boldness are applied by transforming the outline.
   * @param[in] fontFamily The font family e.g. Arial, Courier.
   * @param[in] fontStyle The font's style.
   * @return The path to a font resource; the closest available match for the family parameter.
   */
  const std::string& GetFontPath(const std::string& fontFamily, const std::string& fontStyle); // not const because of mutex

private:
  struct ResourceLoaderImpl;
  ResourceLoaderImpl* mImpl;

  volatile int mTerminateThread;        ///< Set to <> 0 in destructor, signals threads to exit their controlling loops

};

} // namespace SlpPlatform

} // namespace Dali

#endif // __DALI_SLPPLATFORM_RESOURCE_LOADER_H_
