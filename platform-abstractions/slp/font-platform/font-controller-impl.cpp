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
const std::string FONT_FORMAT( "TrueType" );
const std::string DEFAULT_FONT_FAMILY_NAME( "Tizen" );
const std::string DEFAULT_FONT_STYLE( "Regular" );

const std::string NULL_FONT_FAMILY_NAME( "" );
const FontController::StyledFontFamily NULL_STYLED_FONT_FAMILY( std::make_pair( NULL_FONT_FAMILY_NAME, std::string( "" ) ) );

/**
 * @param[in] pattern pointer to a font config pattern
 * @param[out] familyName font family name or an empty string if the font is not found.
 * @return Whether a font is found.
 */
bool GetFontFamily( const FcPattern* pattern, std::string& familyName )
{
  FcChar8* family = NULL;
  const FcResult retVal = FcPatternGetString( pattern, FC_FAMILY, 0u, &family );

  if( FcResultMatch != retVal )
  {
    familyName.clear();
    return false;
  }

  // Have to use reinterpret_cast because FcChar8 is unsigned char*, not a const char*.
  familyName.assign( reinterpret_cast<const char*>( family ) );

  return true;
}

/**
 * @param[in] pattern pointer to a font config pattern
 * @param[out] fontStyle font style name or an empty string if the font has no style.
 * @return Whether a font style is found.
 */
bool GetFontStyle( const FcPattern* pattern, std::string& fontStyle )
{
  FcChar8* style = NULL;
  const FcResult retVal = FcPatternGetString( pattern, FC_STYLE, 0u, &style );

  if( FcResultMatch != retVal)
  {
    // Has no style.
    fontStyle.clear();
    return false;
  }

  // Have to use reinterpret_cast because FcChar8 is unsigned char*, not a const char*.
  fontStyle.assign( reinterpret_cast<const char*>( style ) );

  return true;
}

/**
 * @param[in] pattern pointer to a font config pattern
 * @param[out] fileName font file name or an empty string if the font is not found.
 * @return Whether a font is found.
 */
bool GetFileName( const FcPattern* pattern, std::string& fileName )
{
  FcChar8* file = NULL;
  const FcResult retVal = FcPatternGetString( pattern, FC_FILE, 0u, &file );

  if( FcResultMatch != retVal )
  {
    // Has no file name.
    fileName.clear();
    return false;
  }

  // Have to use reinterpret_cast because FcChar8 is unsigned char*, not a const char*.
  fileName.assign( reinterpret_cast<const char*>( file ) );

  return true;
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

      if( ( 0u == preloadPath.compare( 0u, preloadLength, fileName, 0u, preloadLength ) ) ||
          ( 0u == downloadPath.compare( 0u, downloadLength, fileName, 0u, downloadLength ) ) )
      {
        return true;
      }
      return false;
    }
    case FontController::LIST_APPLICATION_FONTS:
    {
      const std::string& appPath( SETTING_FONT_APP_FONT_PATH );
      const std::size_t appLength = appPath.length();

      if( 0u == appPath.compare( 0u, appLength, fileName, 0u, appLength ) )
      {
        return true;
      }
      return false;
    }
    default:
    {
      DALI_ASSERT_DEBUG( false && "unhandled FontListMode" );
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

FontController::~FontController()
{
  // clear the font family cache
  ClearFontFamilyCache();

  // Clear the preferred font list.
  ClearPreferredFontList();
}

const std::string& FontController::GetFontPath( const StyledFontFamily& styledFontFamily )
{
  DALI_ASSERT_DEBUG( !styledFontFamily.first.empty() && !styledFontFamily.second.empty() && "FontController::GetFontPath(): The font name or the font style is empty. Probably they have not been validated." );

  // lock the mFontFamilyCacheMutex and don't release it until the function finishes.
  // If we release it then another thread may try to create the same duplicate data.
  boost::mutex::scoped_lock lock( mFontFamilyCacheMutex );

  StyledFontFamily closestStyledFontFamilyMatch;

  // first check to see if the font has been matched before.
  closestStyledFontFamilyMatch = GetMatchedFont( styledFontFamily );

  if( closestStyledFontFamilyMatch.first.empty() )
  {
    // The font is not in the matches font cache. Use the given one.
    closestStyledFontFamilyMatch = styledFontFamily;
  }

  return GetCachedFontPath( closestStyledFontFamilyMatch );
}

void FontController::GetFontList( FontListMode fontListMode, FontList& fontList )
{
  // protect the mFontList from access by multiple threads
  // this is locked for the entire function, because we don't want two functions
  // trying to fill the cache with duplicate data.
  boost::mutex::scoped_lock sharedDatalock( mFontListMutex );

  // if we have already scanned for fonts, return the cached values
  if ( !mFontSystemList.empty() )
  {
    GetCachedFontList( fontListMode, fontList );

    return;
  }

  // font list needs to be cached

  // font config isn't thread safe
  boost::mutex::scoped_lock lock( mFontConfigMutex );

  // use font config to get the font set which contains a list of fonts
  FcFontSet* fontSet = GetFontSet();

  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "number of fonts found: %d\n", fontSet->nfont );

  if( fontSet )
  {
    std::string preload_path(SETTING_FONT_PRELOAD_FONT_PATH);
    std::string download_path(SETTING_FONT_DOWNLOADED_FONT_PATH);
    std::string application_path(SETTING_FONT_APP_FONT_PATH);

    for( int i = 0u; i < fontSet->nfont; ++i )
    {
      FcPattern* fontPattern = fontSet->fonts[i];
      std::string fileName;

      if( !GetFileName( fontPattern, fileName ) )
      {
        continue;  // Has no file name. Jump to the next iteration.
      }

      // this is checking to make sure the font is in either the normal font path, or download path
      if( 0u == preload_path.compare( 0u, preload_path.length(), fileName, 0u, preload_path.length() ) ||
          0u == download_path.compare( 0u, download_path.length(), fileName, 0u, download_path.length() ) ||
          0u == application_path.compare( 0u, application_path.length(), fileName, 0u, application_path.length() ) )
      {
        StyledFontFamily styledFontFamily;

        if( !GetFontFamily( fontPattern, styledFontFamily.first ) )
        {
          continue;  // Has no font name. Jump to the next iteration.
        }

        GetFontStyle( fontPattern, styledFontFamily.second );

        // Add the font to the either the system or application font list
        AddToFontList( fileName, styledFontFamily );
      }
    }
     // delete the font set
    FcFontSetDestroy( fontSet );
  }
  else
  {
     DALI_ASSERT_ALWAYS( false && "No valid fonts found on system." );
  }

  // return the font list for the specified mode
  GetCachedFontList( fontListMode, fontList );
}

bool FontController::ValidateFontFamilyName( const StyledFontFamily& styledFontFamily,
                                             bool& isDefaultSystemFontFamily,
                                             bool& isDefaultSystemFontStyle,
                                             StyledFontFamily& closestStyledFontFamilyMatch )
{
  // Initialize the defaults to false as the validation process supposes the given font is correct.
  isDefaultSystemFontFamily = false;
  isDefaultSystemFontStyle = false;

  // default the closest Match to empty
  closestStyledFontFamilyMatch.first.clear();
  closestStyledFontFamilyMatch.second.clear();

  // lock the mFontFamilyCacheMutex and don't release it until the function finishes.
  // If we release it then another thread may try to create the same duplicate data.
  boost::mutex::scoped_lock lock( mFontFamilyCacheMutex );

  StyledFontFamily styledFontFamilyToCheck = styledFontFamily;

  // if the font is blank, then use the default font if it has been cached
  if( styledFontFamilyToCheck.first.empty() && ( !mDefaultStyledFont.first.empty() ) )
  {
    styledFontFamilyToCheck.first = mDefaultStyledFont.first;

    // No font family is given, default system font is used.
    isDefaultSystemFontFamily = true;
  }

  if( styledFontFamilyToCheck.second.empty() &&  ( !mDefaultStyledFont.second.empty() ) )
  {
    styledFontFamilyToCheck.second = mDefaultStyledFont.second;

    // No font style is given, default system font is used.
    isDefaultSystemFontStyle = true;
  }

  // first check to see if the font has been matched before.
  closestStyledFontFamilyMatch = GetMatchedFont( styledFontFamilyToCheck );

  if( !closestStyledFontFamilyMatch.first.empty() )
  {
    // The font has been cached before.
    return true;
  }

  // check the cache
  const std::string& fontFileName = GetCachedFontPath( styledFontFamilyToCheck );

  if( !fontFileName.empty() )
  {
    // The font has been cached before.

    closestStyledFontFamilyMatch = styledFontFamilyToCheck;

    return true;
  }

  DALI_LOG_INFO( gLogFilter, Debug::Verbose,"Failed to find %s %s in cache, querying FontConfig for a match\n", styledFontFamily.first.c_str(), styledFontFamily.second.c_str() );

  // it's not in the cache, find a match using font config and add it to the cache
  boost::mutex::scoped_lock fcLock( mFontConfigMutex );

  // create the pattern
  FcPattern* fontFamilyPattern = CreateFontFamilyPattern( styledFontFamilyToCheck );

  FcResult result(FcResultMatch);

  // match the pattern
  FcPattern* match = FcFontMatch( NULL /* use default configure */, fontFamilyPattern, &result );

  bool validFont = false;

  if( match )
  {
    validFont = true;

    CacheFontInfo( match, styledFontFamilyToCheck, closestStyledFontFamilyMatch );

    // destroyed the matched pattern
    FcPatternDestroy( match );
  }
  else
  {
    DALI_LOG_ERROR( "FcFontMatch failed for font %s %s\n", styledFontFamilyToCheck.first.c_str(), styledFontFamilyToCheck.second.c_str() );
  }

  // destroy the pattern
  FcPatternDestroy( fontFamilyPattern );

  return validFont;
}

const FontController::StyledFontFamily& FontController::GetFontFamilyForChars( const Integration::TextArray& charsRequested )
{
  if( 0u == mPreferredFonts.Count() )
  {
    StyledFontFamily tizenFont( DEFAULT_FONT_FAMILY_NAME, DEFAULT_FONT_STYLE );
    CreatePreferedFontList( tizenFont );
  }

  // Cycle through the preferred list of fonts on the system for 'Tizen'.
  for( std::size_t n = 0u; n < mPreferredFonts.Count(); ++n )
  {
    const StyledFontFamily& font = *mPreferredFonts[n];

    if( !mPreferredFontsValidated[n] )
    {
      // First make sure it is validated and cached so we can access it's character set object
      bool isDefaultSystemFontFamily = false;
      bool isDefaultSystemFontStyle = false;
      StyledFontFamily closestStyledFontFamilyMatch;
      ValidateFontFamilyName( font,
                              isDefaultSystemFontFamily,
                              isDefaultSystemFontStyle,
                              closestStyledFontFamilyMatch );

      mPreferredFontsValidated[n] = true;
    }

    const std::string& filePath = GetFontPath( font );

    if( filePath.empty() )
    {
      continue;
    }

    const bool matched = FontFamilySupportsText( font, charsRequested );
    if( matched )
    {
      return font;
    }
  }

  // return empty string
  return NULL_STYLED_FONT_FAMILY;
}

void FontController::CacheFontInfo( FcPattern* pattern, const StyledFontFamily& inputStyledFontFamily, StyledFontFamily& closestStyledFontFamilyMatch )
{
  // Check we can get the following data from the pattern

  if( !GetFontFamily( pattern, closestStyledFontFamilyMatch.first ) )
  {
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "CacheFontInfo failed to get family information from pattern %s %s\n", inputStyledFontFamily.first.c_str(), inputStyledFontFamily.second.c_str() );
    return;
  }

  std::string fileName;
  if( !GetFileName( pattern, fileName ) )
  {
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "CacheFontInfo failed to get file information from pattern %s %s\n", inputStyledFontFamily.first.c_str(), inputStyledFontFamily.second.c_str() );
    return;
  }

  FcCharSet* matchedCharSet = NULL;
  const FcResult retVal = FcPatternGetCharSet( pattern, FC_CHARSET, 0u, &matchedCharSet );
  if( retVal != FcResultMatch )
  {
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "CacheFontInfo failed to get character set from pattern %s %s\n", inputStyledFontFamily.first.c_str(), inputStyledFontFamily.second.c_str() );
    return;
  }

  GetFontStyle( pattern, closestStyledFontFamilyMatch.second );

  // Add the match to the font cache
  AddCachedFont( closestStyledFontFamilyMatch, fileName, matchedCharSet );

  if( ( !inputStyledFontFamily.first.empty() &&
        ( inputStyledFontFamily.first != closestStyledFontFamilyMatch.first ) ) &&
      ( !inputStyledFontFamily.second.empty() &&
        ( inputStyledFontFamily.second != closestStyledFontFamilyMatch.second ) ) )
  {
    // if the font family used to create the pattern was not found in the match then
    // store it in the MissingFont container
    AddMatchedFont( inputStyledFontFamily, closestStyledFontFamilyMatch );
  }
}

bool FontController::AllGlyphsSupported( const StyledFontFamily& styledFontFamily, const Integration::TextArray& text )
{
  // The font has already been validated by the font implementation.

  return FontFamilySupportsText( styledFontFamily, text );
}

void FontController::SetDefaultFontFamily( const StyledFontFamily& styledFontFamily )
{
  CreatePreferedFontList(styledFontFamily);
}

void FontController::AddCachedFont( const StyledFontFamily& styledFontFamily, const std::string& fontPath, _FcCharSet *characterSet )
{
  FontFamilyLookup::const_iterator iter = mFontFamilyCache.find( styledFontFamily );
  if( iter == mFontFamilyCache.end() )
  {
    // store the path and chacter set
    FontCacheItem item;
    item.FontFileName = fontPath;
    item.FcCharSet = FcCharSetCopy( characterSet );  // increase the ref count on the char set
    mFontFamilyCache[ styledFontFamily ] = item;

    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "Caching font %s %s\n", styledFontFamily.first.c_str(), styledFontFamily.second.c_str() );
  }
}

void FontController::GetCachedFontList( FontListMode fontListMode, FontList& fontList ) const
{
  // return a list of fonts, for the FontListMode
  switch( fontListMode )
  {
    case LIST_SYSTEM_FONTS:
    {
      fontList.insert( fontList.end(), mFontSystemList.begin(), mFontSystemList.end() );
      return;
    }
    case LIST_APPLICATION_FONTS:
    {
      fontList.insert( fontList.end(), mFontApplicationList.begin(), mFontApplicationList.end() );
      return;
    }
    case LIST_ALL_FONTS:
    {
      // add both system and application fonts together
      fontList.insert( fontList.end(), mFontSystemList.begin(), mFontSystemList.end() );
      fontList.insert( fontList.end(), mFontApplicationList.begin(), mFontApplicationList.end() );
      return;
    }
  }
  DALI_ASSERT_ALWAYS( false && "GetCachedFontList called with invalid value." );
}

const std::string& FontController::GetCachedFontPath( const StyledFontFamily& styledFontFamily ) const
{
  if( !mFontFamilyCache.empty() )
  {
    FontFamilyLookup::const_iterator iter = mFontFamilyCache.find( styledFontFamily );
    if( iter != mFontFamilyCache.end() )
    {
      return (*iter).second.FontFileName;
    }
  }

  return NULL_FONT_FAMILY_NAME;
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

_FcPattern* FontController::CreateFontFamilyPattern( const StyledFontFamily& styledFontFamily )
{
  // create the cached font family lookup pattern
  // a pattern holds a set of names, each name refers to a property of the font
  FcPattern* fontFamilyPattern = FcPatternCreate();

  // add a property to the pattern for the font family
  FcPatternAddString( fontFamilyPattern, FC_FAMILY, reinterpret_cast<const FcChar8*>( styledFontFamily.first.c_str() ) );

  // add a property to the pattern for the font family
  FcPatternAddString( fontFamilyPattern, FC_STYLE, reinterpret_cast<const FcChar8*>( styledFontFamily.second.c_str() ) );

  // Add a property of the pattern, to say we want to match TrueType fonts
  FcPatternAddString( fontFamilyPattern , FC_FONTFORMAT, reinterpret_cast<const FcChar8*>( FONT_FORMAT.c_str() ) );

  // modify the config, with the mFontFamilyPatterm
  FcConfigSubstitute( NULL /* use default configure */, fontFamilyPattern, FcMatchPattern );

  // provide default values for unspecified properties in the font pattern
  // e.g. patterns without a specified style or weight are set to Medium
  FcDefaultSubstitute( fontFamilyPattern );

  return fontFamilyPattern;
}

bool FontController::IsAControlCharacter( uint32_t character ) const
{
  // UNICODE_CHAR_START is the space character
  // below it are the control characters that we want to ignore.

  return( ( character < UNICODE_CHAR_START ) ||
          ( character == UNICODE_CR_LF ) );
}

bool FontController::FontFamilySupportsText( const StyledFontFamily& styledFontFamily, const Integration::TextArray& text )
{
  FcCharSet* charSet = GetCachedFontCharacterSet( styledFontFamily );

  DALI_ASSERT_ALWAYS( charSet && "No cached character set for font family" );

  const size_t textLength = text.Count();

  // quick early exit before accessing font config for text arrays which are just a single control character
  if( textLength == 1u )
  {
    if( IsAControlCharacter( *text.Begin() ) )
    {
      return true;
    }
  }

  // protect font config
  boost::mutex::scoped_lock fcLock( mFontConfigMutex );

  for( Integration::TextArray::ConstIterator iter = text.Begin(), endIter = text.End(); iter != endIter; ++iter )
  {
    const uint32_t character = (*iter);

    // if it's a control character then don't test it
    if( IsAControlCharacter( character ) )
    {
      continue;
    }

    // test to see if the character set supports the character
    if( !FcCharSetHasChar( charSet, character ) )
    {
      return false;
    }
  }
  return true;
}

void FontController::ClearFontFamilyCache()
{
  // should be called by the destructor only

  for( FontFamilyLookup::iterator iter = mFontFamilyCache.begin(), enditer = mFontFamilyCache.end(); iter != enditer; ++iter )
  {
    FontCacheItem& cacheItem = (*iter).second;

    // this reduces the character sets ref count by 1.
    FcCharSetDestroy( cacheItem.FcCharSet );
  }

  mFontFamilyCache.clear();
}

void FontController::AddToFontList( const std::string& fileName, const StyledFontFamily& styledFontFamily )
{
  const bool systemFont = CheckFontInstallPath( LIST_SYSTEM_FONTS, fileName );

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
  FcPattern* pattern = FcPatternCreate();

  // create an object set used to define which properties are to be returned in the patterns from FcFontList.
  FcObjectSet* objectSet = FcObjectSetCreate();

  // build an object set from a list of property names
  FcObjectSetAdd( objectSet, FC_FAMILY );
  FcObjectSetAdd( objectSet, FC_STYLE );
  FcObjectSetAdd( objectSet, FC_FILE );

  // get a list of fonts
  // creates patterns from those fonts containing only the objects in objectSet and returns the set of unique such patterns
  FcFontSet* fontset = FcFontList( NULL /* the default configuration is checked to be up to date, and used */, pattern, objectSet );

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

_FcCharSet* FontController::CreateCharacterSet( const Integration::TextArray& charsRequested )
{
  // create the character set object
  FcCharSet* charSet = FcCharSetCreate();

  bool validCharAdded(false);

  // add valid characters to the character set.
  for( Integration::TextArray::ConstIterator iter = charsRequested.Begin(), endIter = charsRequested.End(); iter != endIter; ++iter )
  {
    const uint32_t character = (*iter);

    // if it's not a control character then add it
    if( !IsAControlCharacter( character ) )
    {
      FcBool ok = FcCharSetAddChar( charSet, character );
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

void FontController::AddMatchedFont( const StyledFontFamily& missingStyledFontFamily, StyledFontFamily& closestStyledFontFamilyMatch )
{
  // we store the missing font, and the name of the font that font config found as the closet match
  mMatchedFontsFound[ missingStyledFontFamily ] = closestStyledFontFamilyMatch;
}

const FontController::StyledFontFamily& FontController::GetMatchedFont( const StyledFontFamily& styledFontFamily ) const
{
  if( mMatchedFontsFound.empty() )
  {
    return NULL_STYLED_FONT_FAMILY;
  }

  MatchedFontLookup::const_iterator iter = mMatchedFontsFound.find( styledFontFamily );
  if( iter != mMatchedFontsFound.end() )
  {
    return iter->second;
  }

  return NULL_STYLED_FONT_FAMILY;
}

void FontController::CreatePreferedFontList( const StyledFontFamily& styledFontFamily )
{
  // clear the current list
  ClearPreferredFontList();

  FcPattern* searchPattern = CreateFontFamilyPattern( styledFontFamily );

  FcResult result(FcResultMatch);

  // Match the pattern.
  StyledFontFamily previousFont;

  FcFontSet* fontSet = FcFontSort( NULL /* use default configure */, searchPattern, false /* don't trim */, NULL, &result );

  for( int i = 0u; i < fontSet->nfont; ++i )
  {
      // we have already filled in the first entry with the default font
    FcPattern* pattern = fontSet->fonts[i];

    StyledFontFamily* styledFont = new StyledFontFamily();

    GetFontFamily( pattern, styledFont->first );
    GetFontStyle( pattern, styledFont->second );

    bool releaseMemory = true;
    if( *styledFont != previousFont )
    {
      mPreferredFonts.PushBack( styledFont );
      releaseMemory = false;
    }
    if( i == 0u )
    {
      mDefaultStyledFont = *styledFont;
    }
    previousFont = *styledFont;

    if( releaseMemory )
    {
       delete styledFont;
    }
  }

  // Set all fonts to non validated.
  mPreferredFontsValidated.Resize( fontSet->nfont, false );

  FcPatternDestroy( searchPattern );
  FcFontSetDestroy( fontSet );
}

void FontController::ClearPreferredFontList()
{
  for( Vector<StyledFontFamily*>::Iterator it = mPreferredFonts.Begin(), endIt = mPreferredFonts.End(); it != endIt; ++it )
  {
    delete *it;
  }
  mPreferredFonts.Clear();
}

} // namespace SlpPlatform

} // namespace Dali

// Implementation of Dali::Platform::FontController::New()
Dali::Platform::FontController* Dali::Platform::FontController::New()
{
  return new Dali::SlpPlatform::FontController();
}
