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

// CLASS HEADER
#include "metrics-cache.h"

// EXTERNAL HEADERS
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <boost/functional/hash.hpp>
#include <string.h>

// INTERNAL HEADERS
#include <dali/integration-api/glyph-set.h>


namespace Dali
{

namespace // Anonymous namespace
{

const std::size_t VERSION_SIZE( sizeof(__DATE__ "-" __TIME__) );  // (date-time)
const char FILE_VERSION[ VERSION_SIZE ] =  __DATE__ "-" __TIME__; // Updates with each build

const size_t CHECKSUM_SIZE( sizeof( size_t ) );
const size_t NUMBER_GLOBAL_METRICS( 9 ); // line height, ascender, unitsPerEM, underlinePosition, underlineThickness, maxWidth, maxHeight, padAdjustX, padAdjustY
const size_t GLOBAL_METRICS_SIZE( NUMBER_GLOBAL_METRICS * sizeof( float ) );
const size_t GLOBAL_METRIC_HEADER_SIZE( GLOBAL_METRICS_SIZE + CHECKSUM_SIZE + VERSION_SIZE );
const std::string DALI_DEFAULT_FONT_CACHE_PATH( DALI_USER_FONT_CACHE_DIR );
const std::string METRICS_EXTENSION( ".metrics" );


std::string CreateFileName(std::string fontFamily, std::string fontStyle)
{
  std::string cacheFileName(DALI_DEFAULT_FONT_CACHE_PATH + fontFamily + "-" + fontStyle + METRICS_EXTENSION);

  std::replace(cacheFileName.begin(), cacheFileName.end(), ' ', '-' );
  return cacheFileName;
}

std::size_t CalculateGlobalMetricCheckSum( const Integration::GlobalMetrics& globalMetrics )
{
  int checksum = static_cast<int>( globalMetrics.lineHeight ) +
                 static_cast<int>( globalMetrics.ascender )   +
                 static_cast<int>( globalMetrics.unitsPerEM ) +
                 static_cast<int>( globalMetrics.underlinePosition ) +
                 static_cast<int>( globalMetrics.underlineThickness ) +
                 static_cast<int>( globalMetrics.maxWidth ) +
                 static_cast<int>( globalMetrics.maxHeight ) +
                 static_cast<int>( globalMetrics.padAdjustX ) +
                 static_cast<int>( globalMetrics.padAdjustY );


  return boost::hash_value( checksum );

}

std::size_t CalculateGlyphCheckSum( const Integration::GlyphMetrics& metrics )
{
  int checksum =  static_cast<int>( metrics.code )    +
                  static_cast<int>( metrics.width )   +
                  static_cast<int>( metrics.height )  +
                  static_cast<int>( metrics.top )     +
                  static_cast<int>( metrics.left )    +
                  static_cast<int>( metrics.xAdvance);

  return boost::hash_value( checksum );
}

bool ReadGlyphMetrics( std::ifstream& file,  Integration::GlyphMetrics& metrics )
{
  std::size_t checkSumRead;
  uint32_t code;

  file.read( reinterpret_cast<char*>( &code ),         sizeof( uint32_t ) );
  file.read( reinterpret_cast<char*>( &metrics.width ),   sizeof( float ) );
  file.read( reinterpret_cast<char*>( &metrics.height ),  sizeof( float ) );
  file.read( reinterpret_cast<char*>( &metrics.top ),     sizeof( float ) );
  file.read( reinterpret_cast<char*>( &metrics.left ),    sizeof( float ) );
  file.read( reinterpret_cast<char*>( &metrics.xAdvance), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &checkSumRead ),    sizeof( std::size_t ) );

  metrics.code = code;  // code is now stored as a bit mask so we can't read directly in to it
  metrics.quality = Integration::GlyphMetrics::LOW_QUALITY;

  const std::size_t calcCheckSum = CalculateGlyphCheckSum( metrics );

  return ( calcCheckSum == checkSumRead );
}

void WriteGlyphMetrics( std::ofstream& file, const Integration::GlyphMetrics& metrics )
{
  uint32_t code = metrics.code; // code is stored as a bitmask so we can't write directly from it
  file.write( reinterpret_cast<const char*>( &code ),     sizeof( uint32_t ) );
  file.write( reinterpret_cast<const char*>( &metrics.width ),    sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &metrics.height ),   sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &metrics.top ),      sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &metrics.left ),     sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &metrics.xAdvance ), sizeof( float ) );

  const std::size_t calcCheckSum = CalculateGlyphCheckSum( metrics );

  file.write( reinterpret_cast<const char*>( &calcCheckSum ), sizeof( std::size_t ) );
}

bool ReadGlyphGlobalMetrics( std::ifstream& file, Integration::GlobalMetrics& globalMetrics )
{
  char version_string[ VERSION_SIZE ];

  // Check file version
  file.read( version_string, VERSION_SIZE );
  if( strncmp( version_string, FILE_VERSION, VERSION_SIZE ) != 0 )
  {
    return false;
  }

  std::size_t checkSumRead;

  file.read( reinterpret_cast<char*>( &globalMetrics.lineHeight ), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &globalMetrics.ascender ), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &globalMetrics.unitsPerEM ), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &globalMetrics.underlinePosition ), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &globalMetrics.underlineThickness), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &globalMetrics.maxWidth), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &globalMetrics.maxHeight), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &globalMetrics.padAdjustX), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &globalMetrics.padAdjustY), sizeof( float ) );
  file.read( reinterpret_cast<char*>( &checkSumRead ), sizeof( std::size_t ) );

  const std::size_t calcCheckSum = CalculateGlobalMetricCheckSum( globalMetrics );

  return (calcCheckSum == checkSumRead);
}

void WriteGlyphGlobalMetrics( std::ofstream &file, const Integration::GlobalMetrics& globalMetrics )
{
  file.write( FILE_VERSION, VERSION_SIZE );

  file.write( reinterpret_cast<const char*>( &globalMetrics.lineHeight ), sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &globalMetrics.ascender ), sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &globalMetrics.unitsPerEM ), sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &globalMetrics.underlinePosition ), sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &globalMetrics.underlineThickness), sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &globalMetrics.maxWidth), sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &globalMetrics.maxHeight), sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &globalMetrics.padAdjustX ), sizeof( float ) );
  file.write( reinterpret_cast<const char*>( &globalMetrics.padAdjustY ), sizeof( float ) );

  const std::size_t calcCheckSum = CalculateGlobalMetricCheckSum( globalMetrics );

  file.write( reinterpret_cast<const char*>( &calcCheckSum ), sizeof( std::size_t ) );
}

} // Anonymous namespace

namespace SlpPlatform
{
namespace MetricsCache
{

bool ReadGlobal( std::string fontFamily, std::string fontStyle, Integration::GlobalMetrics& globalMetrics)
{
  bool success = false;

  std::string fileName = CreateFileName( fontFamily, fontStyle );

  std::ifstream file( fileName.c_str(), std::ios::in | std::ios::binary );

  if( file.good() )
  {
    file.seekg( std::ios::beg );

    success = ReadGlyphGlobalMetrics( file, globalMetrics );

    // check the metrics had the correct size.
    // this is just incase the metrics file is the wrong size, but the checksum magically worked.
    if( file.eof() )
    {
      // file.eof is true only after an invalid read
      success = false;
    }

    file.close();
  }

  return success;
}

void WriteGlobal( std::string fontFamily, std::string fontStyle, const Integration::GlobalMetrics& globalMetrics)
{
  std::string fileName = CreateFileName( fontFamily, fontStyle );
  std::ios_base::openmode mode = ( std::ios::out | std::ios::binary | std::ios::trunc );

  std::ofstream file( fileName.c_str(), mode );

  if( file.good() )
  {
    WriteGlyphGlobalMetrics( file, globalMetrics );

    file.close();
  }
}

bool Read( std::string fontFamily, std::string fontStyle, std::vector<Integration::GlyphMetrics>& glyphMetricsContainer )
{
  std::string fileName = CreateFileName( fontFamily, fontStyle );
  bool success( false );

  // Read from default glyph cache only when there is any metric loaded.
  std::ifstream file( fileName.c_str(), std::ios::in | std::ios::binary );

  if( file.good() )
  {
    // skip over the gobal metrics
    file.seekg( GLOBAL_METRIC_HEADER_SIZE , std::ios::beg );

    bool checkSumPassed(true);

    // file.good() is set to false if end of file is reached, or an error occurred
    while( checkSumPassed && file.good() )
    {
      Integration::GlyphMetrics glyphMetrics;

      checkSumPassed = ReadGlyphMetrics( file, glyphMetrics );

      // eof only returns true after a failed read
      if( file.eof() )
      {
        // this will occur when we hit the end of the file.
        checkSumPassed = true;
        break;
      }

      if( checkSumPassed )
      {
        // fill the map with cached metrics.
        glyphMetricsContainer.push_back(glyphMetrics);
      }
      else
      {
        DALI_LOG_WARNING("check sum failed for glyph %d in file \n", glyphMetrics.code, fileName.c_str() );
      }
    }

    file.close();
    success = checkSumPassed;
  }

  return success;
}

void Write( std::string fontFamily, std::string fontStyle, const Integration::GlyphSet& glyphSet )
{
  // append the file.
  std::string fileName = CreateFileName( fontFamily, fontStyle );
  std::ofstream file( fileName.c_str(), std::ios::out | std::ios::binary | std::ios::app );

  if( file.good() )
  {
    const Integration::GlyphSet::CharacterList& characterList = glyphSet.GetCharacterList();

    for( Integration::GlyphSet::CharacterConstIter it = characterList.begin(), endIt = characterList.end(); it != endIt; ++it )
    {
      WriteGlyphMetrics( file, it->second );
    }

    file.close();
  }
  else
  {
    DALI_LOG_WARNING( "Failed to open metric to file %s", fileName.c_str() );
  }
}


} // MetricsCache
} // SlpPlatform
} // Dali
