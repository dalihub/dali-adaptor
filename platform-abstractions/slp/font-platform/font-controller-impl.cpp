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

// CLASS HEADER
#include "font-controller-impl.h"

// INTERNAL HEADERS
#include <dali/public-api/common/dali-common.h>
#include <dali/integration-api/debug.h>

// EXTERNAL HEADERS
#include <fontconfig/fontconfig.h>


namespace Dali
{

namespace SlpPlatform
{

#if defined(DEBUG_ENABLED)
namespace
{
Dali::Integration::Log::Filter* gLogFilter = Dali::Integration::Log::Filter::New(Debug::Concise, false, "LOG_FONT_CONTROLLER");
} // unnamed namespace
#endif

namespace
{
const std::string SETTING_FONT_PRELOAD_FONT_PATH( FONT_PRELOADED_PATH );
const std::string SETTING_FONT_DOWNLOADED_FONT_PATH( FONT_DOWNLOADED_PATH );
const std::string SETTING_FONT_APP_FONT_PATH( FONT_APPLICATION_PATH );

const uint32_t UNICODE_CR_LF = 0x85;
const uint32_t UNICODE_CHAR_START = 0x20;       // start range of unicode characters (below are control chars)

/**
 * @param[in] pattern pointer to a font config pattern
 * @return font style name or an empty string if the font has no style
 */
std::string GetFontStyle( const FcPattern *pattern )
{
  std::string styleName;
  FcChar8 *style(NULL);
  FcResult retVal =  FcPatternGetString( pattern, FC_STYLE, 0, &style );

  if( FcResultMatch == retVal)
  {
    styleName =  reinterpret_cast<const char*>(style);
  }
  else
  {
    styleName = "";
  }
  return styleName;
}

std::string GetFontFamily( const FcPattern *pattern )
{
  std::string familyName;
  FcChar8 *family(NULL);
  FcResult retVal =  FcPatternGetString( pattern, FC_FAMILY, 0, &family );

  if( FcResultMatch == retVal)
  {
    familyName =  reinterpret_cast<const char*>(family);
  }
  else
  {
    familyName = "";
  }
  return familyName;
}

bool CheckFontInstallPath( FontController::FontListMode listMode, const std::string& fileName )
{
  switch( listMode )
  {
    case FontController::LIST_SYSTEM_FONTS:
    {
      const std::string& preloadPath( SETTING_FONT_PRELOAD_FONT_PATH );
      const std::string& downloadPath( SETTING_FONT_DOWNLOADED_FONT_PATH );
      const std::size_t preloadLength = preloadPath.length();
      const std::size_t downloadLength = downloadPath.length();

      if( 0 == preloadPath.compare(0, preloadLength, fileName, 0, preloadLength) ||
           0 == downloadPath.compare(0, downloadLength, fileName, 0, downloadLength))
      {
        return true;
      }
      return false;
    }
    case FontController::LIST_APPLICATION_FONTS:
    {
      const std::string& appPath( SETTING_FONT_APP_FONT_PATH );
      const std::size_t appLength = appPath.length();

      if( 0 == appPath.compare(0, appLength, fileName, 0, appLength) )
      {
        return true;
      }
      return false;
    }
    default:
    {
      DALI_ASSERT_DEBUG(0 && "unhandled FontListMode");
      return false;
    }
  }
}

} // unnamed namespace

FontController::FontController()
{
  FcInit();
  FcConfigEnableHome(true);
}

void FontController::CreatePreferedFontList( )
{
  StyledFontFamily tizenFont;
  tizenFont.first = "Tizen";
  tizenFont.second = "Regular";

  // clear the current list
  mPreferredFonts.clear();

  _FcPattern* searchPattern = CreateFontFamilyPattern( tizenFont );

  FcResult result(FcResultMatch);

  // Match the pattern.
  std::string previousFont;

  FcFontSet* fontSet = FcFontSort( NULL /* use default configure */, searchPattern, false /* don't trim */, NULL, &result );

  for( int i=0; i < fontSet->nfont; ++i)
  {
      // we have already filled in the first entry with the default font
    _FcPattern* pattern = fontSet->fonts[i];

    StyledFontFamily styledFont;

    styledFont.first = GetFontFamily( pattern );
    styledFont.second = GetFontStyle( pattern );

    if( styledFont.first != previousFont )
    {
      mPreferredFonts.push_back( styledFont );
    }
    if( i == 0 )
    {
      mDefaultStyledFont = styledFont;
    }
    previousFont = styledFont.first;
  }
  FcPatternDestroy( searchPattern );
  FcFontSetDestroy( fontSet );

}

FontController::~FontController()
{
  // clear the font family cache
  ClearFontFamilyCache();
}

std::string FontController::GetFontPath( const StyledFontFamily& styledFontFamily )
{
  StyledFontFamily closestMachedStyledFontFamily;
  bool isDefault( false );

  // if the font was not found, the path will be an empty string
  return GetFontFamilyPath( styledFontFamily, isDefault, closestMachedStyledFontFamily );
}

FontList FontController::GetFontList( FontListMode fontListMode )
{
  // protect the mFontList from access by multiple threads
  // this is locked for the entire function, because we don't want two functions
  // trying to fill the cache with duplicate data.
  boost::mutex::scoped_lock sharedDatalock( mFontListMutex );

  // if we have already scanned for fonts, return the cached values
  if ( !mFontSystemList.empty() )
  {
    return GetCachedFontList( fontListMode );
  }

  // font list needs to be cached

  // font config isn't thread safe
  boost::mutex::scoped_lock lock( mFontConfigMutex );

  // use font config to get the font set which contains a list of fonts
  FcFontSet* fontset = GetFontSet();

  DALI_LOG_INFO (gLogFilter, Debug::Verbose, "number of fonts found: %d\n", fontset->nfont);

  if( fontset )
  {
    std::string preload_path(SETTING_FONT_PRELOAD_FONT_PATH);
    std::string download_path(SETTING_FONT_DOWNLOADED_FONT_PATH);
    std::string application_path(SETTING_FONT_APP_FONT_PATH);
    FcChar8 *file(NULL);
    FcChar8 *family(NULL);
    FcResult retVal;

    for( int i = 0; i < fontset->nfont; ++i)
    {
      retVal = FcPatternGetString(fontset->fonts[i], FC_FILE, 0, &file);
      if( FcResultMatch != retVal)
      {
        continue;  // has no file name
      }

      retVal = FcPatternGetString(fontset->fonts[i], FC_FAMILY, 0, &family);
      if( FcResultMatch != retVal)
      {
        continue;  // has no font name
      }

      std::string fileName(reinterpret_cast<const char*>(file));

      // this is checking to make sure the font is in either the normal font path, or download path
      if( 0 == preload_path.compare(0, preload_path.length(), fileName, 0, preload_path.length()) ||
          0 == download_path.compare(0, download_path.length(), fileName, 0, download_path.length()) ||
          0 == application_path.compare(0, application_path.length(), fileName, 0, application_path.length()) )
      {
        const std::string fontFamily( reinterpret_cast<const char*>(family));
        const std::string fontStyle = GetFontStyle( fontset->fonts[i]);

        // Add the font to the either the system or application font list
        AddToFontList( fileName, std::make_pair( fontFamily, fontStyle ) );
      }
    }
     // delete the font set
    FcFontSetDestroy(fontset);
  }
  else
  {
     DALI_ASSERT_ALWAYS( 0 && "No valid fonts found on system." );
  }
  // return the font list for the specified mode
  return GetCachedFontList( fontListMode );
}

bool FontController::ValidateFontFamilyName( const StyledFontFamily& styledFontFamily, bool& isDefaultSystemFont, StyledFontFamily& closestStyledFontFamilyMatch )
{
  // get font family path also finds the closest match
  std::string path = GetFontFamilyPath( styledFontFamily, isDefaultSystemFont, closestStyledFontFamilyMatch );

  // it should always find a closest match
  DALI_ASSERT_ALWAYS( !closestStyledFontFamilyMatch.first.empty()  && "Cannot find closest match for font family" );

  if( path.empty() )
  {
    return false;
  }
  else
  {
    return true;
  }
}

FontController::StyledFontFamily FontController::GetFontFamilyForChars(const TextArray& charsRequested)
{
  if( mPreferredFonts.empty() )
  {
    CreatePreferedFontList();
  }

  // Cycle through the preferred list of fonts on the system for 'Tizen'.
  for( std::size_t n = 0; n < mPreferredFonts.size() ; n++ )
  {
    StyledFontFamily font = mPreferredFonts[n];

    // First make sure it is cached so we can access it's character set object
    std::string filePath = GetFontPath( font );
    if( filePath.empty())
    {
      filePath = GetFontPath( font );
    }

    if( filePath.empty())
    {
      continue;
    }
    bool matched = FontFamilySupportsText( font, charsRequested);
    if( matched )
    {
      return font;
    }
  }
  // return empty string
  return StyledFontFamily();
}

void FontController::CacheFontInfo(FcPattern* pattern, const StyledFontFamily& inputStyledFontFamily, StyledFontFamily& closestStyledFontFamilyMatch )
{
  FcChar8 *family, *file;
  FcCharSet *matchedCharSet;
  FcResult retVal;

  // Check we can get the following data from the pattern

  retVal = FcPatternGetString( pattern, FC_FAMILY, 0, &family );
  if( retVal != FcResultMatch )
  {
    DALI_LOG_INFO (gLogFilter, Debug::Verbose,"CacheFontInfo failed to get family information from pattern %s %s\n",inputStyledFontFamily.first.c_str(),inputStyledFontFamily.second.c_str());
    return;
  }
  retVal = FcPatternGetString( pattern, FC_FILE, 0, &file);
  if( retVal!= FcResultMatch)
  {
    DALI_LOG_INFO (gLogFilter, Debug::Verbose,"CacheFontInfo failed to get file information from pattern %s %s\n",inputStyledFontFamily.first.c_str(),inputStyledFontFamily.second.c_str());
    return;
  }
  retVal = FcPatternGetCharSet( pattern, FC_CHARSET, 0, &matchedCharSet);
  if( retVal!= FcResultMatch)
  {
    DALI_LOG_INFO (gLogFilter, Debug::Verbose,"CacheFontInfo failed to get character set from pattern %s %s\n",inputStyledFontFamily.first.c_str(),inputStyledFontFamily.second.c_str());
    return;
  }

  // have to use reinterpret_cast because FcChar8 is unsigned char *, not a char *
  std::string familyName = reinterpret_cast<const char*>(family);
  std::string fileName = reinterpret_cast<const char*>(file);
  std::string fontStyle = GetFontStyle( pattern );

  closestStyledFontFamilyMatch = std::make_pair( familyName, fontStyle );

  // Add the match to the font cache
  AddCachedFont( closestStyledFontFamilyMatch, fileName, matchedCharSet );


  if( ( !inputStyledFontFamily.first.empty() &&
        inputStyledFontFamily.first != familyName ) ||
      ( inputStyledFontFamily.second != fontStyle  ))
  {
    // if the font family used to create the pattern was not found in the match then
    // store it in the MissingFont container
    AddFontNotFound( inputStyledFontFamily, closestStyledFontFamilyMatch );
  }
}

bool FontController::AllGlyphsSupported(const StyledFontFamily& styledFontFamily, const TextArray& text)
{
  StyledFontFamily closestStyledFontFamilyMatch;
  bool isDefault( false );

  // make sure the font is cached first.
  std::string path = GetFontFamilyPath( styledFontFamily, isDefault, closestStyledFontFamilyMatch );

  if( path.empty() )
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "font family not found  \n");
    return false;
  }

  return FontFamilySupportsText( styledFontFamily, text);

}

void FontController::SetDefaultFontFamily( const StyledFontFamily& styledFontFamily )
{
  // reload font configuration files
  bool ok =  FcInitReinitialize();
  DALI_ASSERT_ALWAYS( ok && "FcInitReinitialize failed");

  CreatePreferedFontList();
}

void FontController::AddCachedFont(const StyledFontFamily& styledFontFamily, const std::string& fontPath, _FcCharSet *characterSet)
{
  if( styledFontFamily.first.empty() )
  {
    return;
  }

  FontFamilyLookup::const_iterator iter = mFontFamilyCache.find( styledFontFamily );
  if( iter == mFontFamilyCache.end() )
  {
    // store the path and chacter set
    FontCacheItem item;
    item.FontFileName = fontPath;
    item.FcCharSet = FcCharSetCopy( characterSet );  // increase the ref count on the char set
    mFontFamilyCache[ styledFontFamily ] = item;

    DALI_LOG_INFO (gLogFilter, Debug::Verbose,"Caching font %s %s\n",styledFontFamily.first.c_str(), styledFontFamily.second.c_str());
  }
}

FontList FontController::GetCachedFontList(  FontListMode fontListMode  ) const
{
  // return a list of fonts, for the FontListMode
  switch( fontListMode )
  {
    case LIST_SYSTEM_FONTS:
    {
      return mFontSystemList;
    }
    case LIST_APPLICATION_FONTS:
    {
      return mFontApplicationList;
    }
    case LIST_ALL_FONTS:
    {
      // add both system and application fonts together
      FontList list( mFontSystemList );
      list.insert( list.end(), mFontApplicationList.begin(), mFontApplicationList.end() );
      return list;
    }
  }
  DALI_ASSERT_ALWAYS(!"GetCachedFontList called with invalid value.");
}
std::string FontController::GetCachedFontPath( const StyledFontFamily& styledFontFamily ) const
{
  if( !mFontFamilyCache.empty() )
  {
    FontFamilyLookup::const_iterator iter = mFontFamilyCache.find( styledFontFamily );
    if( iter != mFontFamilyCache.end() )
    {
      return (*iter).second.FontFileName;
    }
  }
  return "";
}

FcCharSet* FontController::GetCachedFontCharacterSet( const StyledFontFamily& styledFontFamily ) const
{
  if( !mFontFamilyCache.empty() )
  {
    FontFamilyLookup::const_iterator iter = mFontFamilyCache.find( styledFontFamily );
    if( iter != mFontFamilyCache.end() )
    {
      return (*iter).second.FcCharSet;
    }
  }
  return NULL;
}

std::string FontController::GetFontFamilyPath( const StyledFontFamily& styledFontFamily, bool& isDefaultSystemFont, StyledFontFamily& closestStyledFontFamilyMatch )
{
  // Lets suppose the given font family is ok.
  isDefaultSystemFont = false;

  // default the closest Match to empty
  closestStyledFontFamilyMatch = std::make_pair("","");

  std::string font( styledFontFamily.first );
  std::string style( styledFontFamily.second );

  // lock the mFontFamilyCacheMutex and don't release it until the function finishes.
  // If we release it then another thread may try to create the same duplicate data.
  boost::mutex::scoped_lock lock( mFontFamilyCacheMutex );

  // if the font is blank, then use the default font if it has been cached
  if( font.empty() &&  ( !mDefaultStyledFont.first.empty() ) )
  {
    font = mDefaultStyledFont.first;
    style = mDefaultStyledFont.second;

    // No font family is given, default system font is used.
    isDefaultSystemFont = true;
  }

  StyledFontFamily styledFontFamilyToCheck = std::make_pair( font, style );

  // first check to see if the font has been marked as not found
  closestStyledFontFamilyMatch = GetFontNotFound( styledFontFamilyToCheck );

  if( !closestStyledFontFamilyMatch.first.empty() )
  {
    // the font wasn't found, so return an empty path
    return "";
  }

  // check the cache
  std::string fontFileName = GetCachedFontPath( styledFontFamilyToCheck );

  if( !fontFileName.empty() )
  {
    closestStyledFontFamilyMatch = std::make_pair( font, style );

    return fontFileName;
  }

  DALI_LOG_INFO( gLogFilter, Debug::Verbose,"Failed to find %s %s in cache, querying FontConfig for a match\n", styledFontFamily.first.c_str(), styledFontFamily.second.c_str());

  // it's not in the cache, find a match using font config and add it to the cache
  boost::mutex::scoped_lock fcLock( mFontConfigMutex );

  // create the pattern
  _FcPattern *fontFamilyPattern = CreateFontFamilyPattern( std::make_pair( font, style ) );

  FcResult result(FcResultMatch);

  // match the pattern
  _FcPattern *match = FcFontMatch( NULL /* use default configure */, fontFamilyPattern, &result );

  if( match )
  {
    CacheFontInfo(match, std::make_pair( font, style ), closestStyledFontFamilyMatch);

    fontFileName = GetCachedFontPath(closestStyledFontFamilyMatch);

    // destroyed the matched pattern
    FcPatternDestroy( match );
  }
  else
  {
    DALI_LOG_ERROR("FcFontMatch failed for font %s %s\n",font.c_str(),style.c_str());
  }

  // destroy the pattern
  FcPatternDestroy( fontFamilyPattern );

  if( closestStyledFontFamilyMatch != styledFontFamily  )
  {
    // the font wasn't font, so return an empty path
    return "";
  }
  else
  {
    return fontFileName;
  }
}

_FcPattern* FontController::CreateFontFamilyPattern(const StyledFontFamily& styledFontFamily)
{
  // create the cached font family lookup pattern
  // a pattern holds a set of names, each name refers to a property of the font
  _FcPattern *fontFamilyPattern = FcPatternCreate();

  // add a property to the pattern for the font family
  FcPatternAddString( fontFamilyPattern, FC_FAMILY, (FcChar8 *)(styledFontFamily.first.c_str()));

  // add a property to the pattern for the font family
  FcPatternAddString( fontFamilyPattern, FC_STYLE, (FcChar8 *)(styledFontFamily.second.c_str()));

  // Add a property of the pattern, to say we want to match TrueType fonts
  FcPatternAddString( fontFamilyPattern , FC_FONTFORMAT, (FcChar8 *)"TrueType");

  // modify the config, with the mFontFamilyPatterm
  FcConfigSubstitute( NULL /* use default configure */, fontFamilyPattern, FcMatchPattern );

  // provide default values for unspecified properties in the font pattern
  // e.g. patterns without a specified style or weight are set to Medium
  FcDefaultSubstitute( fontFamilyPattern );

  return fontFamilyPattern;
}

bool FontController::IsAControlCharacter(uint32_t character) const
{
  // UNICODE_CHAR_START is the space character
  // below it are the control characters that we want to ignore.

  if( character < UNICODE_CHAR_START  ||
      character == UNICODE_CR_LF )
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool FontController::FontFamilySupportsText(const StyledFontFamily& styledFontFamily, const TextArray& text )
{
  _FcCharSet* charSet = GetCachedFontCharacterSet( styledFontFamily );

  DALI_ASSERT_ALWAYS( charSet && "No cached character set for font family" );

  size_t textLength = text.size();

  // quick early exit before accessing font config for text arrays which are just a single control character
  if( textLength == 1 )
  {
    if( IsAControlCharacter( text[0] ) )
    {
      return true;
    }
  }

  // protect font config
  boost::mutex::scoped_lock fcLock( mFontConfigMutex );

  for( TextArray::const_iterator iter = text.begin(), endIter = text.end(); iter != endIter; ++iter)
  {
    const uint32_t character = (*iter);

    // if it's a control character then don't test it
    if( IsAControlCharacter( character) )
    {
      continue;
    }

    // test to see if the character set supports the character
    FcBool hasChar = FcCharSetHasChar(charSet, character );
    if( !hasChar )
    {
      return false;
    }
  }
  return true;
}

void FontController::ClearFontFamilyCache()
{
  // should be called by the destructor only

  for( FontFamilyLookup::iterator iter = mFontFamilyCache.begin(), enditer = mFontFamilyCache.end(); iter != enditer; ++iter)
  {
    FontCacheItem& cacheItem = (*iter).second;

    // this reduces the character sets ref count by 1.
    FcCharSetDestroy( cacheItem.FcCharSet );
  }

  mFontFamilyCache.clear();
}

void FontController::AddToFontList(const std::string& fileName, const StyledFontFamily& styledFontFamily)
{
  bool systemFont = CheckFontInstallPath(LIST_SYSTEM_FONTS, fileName);

  FontList* fontList(NULL);

  if( systemFont )
  {
    fontList = &mFontSystemList;
  }
  else
  {
    fontList = &mFontApplicationList;
  }

  // check the font family doesn't already exist in the vector, then add it
  if( fontList->end() == std::find( fontList->begin(), fontList->end(), styledFontFamily ) )
  {
    fontList->push_back( styledFontFamily );
  }
}

_FcFontSet* FontController::GetFontSet() const
{
  // create a new pattern.
  // a pattern holds a set of names, each name refers to a property of the font
  FcPattern *pattern = FcPatternCreate();

  // create an object set used to define which properties are to be returned in the patterns from FcFontList.
  FcObjectSet *objectSet = FcObjectSetCreate();

  // build an object set from a list of property names
  FcObjectSetAdd( objectSet, FC_FAMILY);
  FcObjectSetAdd( objectSet, FC_STYLE);
  FcObjectSetAdd( objectSet, FC_FILE);

  // get a list of fonts
  // creates patterns from those fonts containing only the objects in objectSet and returns the set of unique such patterns
  FcFontSet *fontset = FcFontList( NULL /* the default configuration is checked to be up to date, and used */, pattern, objectSet);

  // clear up the object set
  if( objectSet )
  {
    FcObjectSetDestroy( objectSet );
  }
  // clear up the pattern
  if( pattern )
  {
    FcPatternDestroy( pattern );
  }
  return fontset;
}

_FcCharSet* FontController::CreateCharacterSet(const TextArray& charsRequested)
{
  // create the character set object
  FcCharSet* charSet = FcCharSetCreate();

  bool validCharAdded(false);

  // add valid characters to the character set.
  for (TextArray::const_iterator iter = charsRequested.begin(), endIter = charsRequested.end(); iter != endIter; ++iter)
  {
    const uint32_t character = (*iter);

    // if it's not a control character then add it
    if( !IsAControlCharacter( character ) )
    {
      FcBool ok = FcCharSetAddChar(charSet, character );
      if( ok )
      {
        validCharAdded = true;
      }
    }
  }
  // if no characters have been added to the character set, then return null
  if( !validCharAdded )
  {
    FcCharSetDestroy(charSet);
    return NULL;
  }
  return charSet;
}

void FontController::AddFontNotFound( const StyledFontFamily& missingStyledFontFamily, StyledFontFamily& closestStyledFontFamilyMatch )
{

  // we store the missing font, and the name of the font that font config found as the closet match
  mFontsNotFound[ missingStyledFontFamily ] = closestStyledFontFamilyMatch;
}

FontController::StyledFontFamily FontController::GetFontNotFound(const StyledFontFamily& styledFontFamily) const
{
  if( mFontsNotFound.empty() )
  {
    return std::make_pair("","");
  }

  FontsNotFound::const_iterator iter = mFontsNotFound.find( styledFontFamily );
  if( iter != mFontsNotFound.end() )
  {
    return (*iter).second;
  }
  return std::make_pair("","");
}

} // namespace SlpPlatform

} // namespace Dali

// Implementation of Dali::Platform::FontController::New()
Dali::Platform::FontController* Dali::Platform::FontController::New()
{
  return new Dali::SlpPlatform::FontController();
}
