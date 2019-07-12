#ifndef DALI_INTERNAL_PLATFORM_FILECLOSER_H
#define DALI_INTERNAL_PLATFORM_FILECLOSER_H
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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-loader.h>

// EXTERNAL INCLUDES
#include <cstdio>

namespace Dali
{
namespace Internal
{
namespace Platform
{

/**
 * Opens files and closes them later even if an exception is thrown.
 */
class FileCloser
{
protected: // prevent this class being directly instantiated

  /**
   * @brief Construct a FileCloser guarding a new FILE* for accessing the path passed in.
   */
  FileCloser( const char* const filename, const char* const mode )
  {
    DALI_ASSERT_DEBUG( filename != 0 && "Can't open a null filename." );
    DALI_ASSERT_DEBUG( mode != 0 && "Null mode is undefined behaviour in spec." );

    Dali::FileLoader::FileType fileType = Dali::FileLoader::FileType::TEXT;

    const char* modeStr = mode;
    while( *modeStr )
    {
      switch ( *modeStr )
      {
      case 'r':
        break;
      case 'b':
        fileType = FileLoader::FileType::BINARY;
        break;
      // Still has to use fopen for append and write modes
      case 'a':
      case 'w':
      case '+':
        mFile = fopen( filename, mode );
        return;
      default:
        break;
      }

      ++modeStr;
    }

    std::streampos bufferSize = 0;
    if( !Dali::FileLoader::ReadFile( filename, bufferSize, mFileBuffer, fileType ) )
    {
      mFile = nullptr;
    }
    else
    {
      mFile = fmemopen( &mFileBuffer[0], bufferSize, mode );
    }
  }

  /**
   * @brief Construct a FileCloser guarding a FILE* for reading out of the memory buffer passed in.
   */
  FileCloser( uint8_t* buffer, size_t dataSize, const char* const mode )
  : mFile( fmemopen( buffer, dataSize, mode) )
  {
  }

  FileCloser( Dali::Vector<uint8_t>& vector, size_t dataSize, const char * const mode )
  {
    // Resize the buffer to ensure any null that gets written by
    // fmemopen is written past the end of any data that is written to the buffer.
    // (Workaround for a bug in Ubuntu that overwrites null to the last byte of the
    // data block regardless of whether binary mode was specified. Tizen doesn't write
    // null if binary mode is specified).
    size_t bufferSize = dataSize;
    ++bufferSize;
    vector.Resize( bufferSize );

    void * const buffer = &vector[0];
    mFile = fmemopen( buffer, bufferSize, mode );

    DALI_ASSERT_DEBUG( buffer != 0 && "Cant open file on null buffer." );
    DALI_ASSERT_DEBUG( dataSize > 0 && "Pointless to open file on empty buffer." );
    DALI_ASSERT_DEBUG( mode != 0 && "Null mode is undefined behaviour in spec." );

    if( mFile == 0 )
    {
      DALI_LOG_WARNING( "File open failed for memory buffer at location: \"%p\", of size: \"%u\", in mode: \"%s\".\n", static_cast<void*>(buffer), static_cast<unsigned>(dataSize), mode );
    }
  }

   /**
    * @brief Destroy the FileCloser and clean up its FILE*.
    */
  ~FileCloser()
  {
    if( mFile != 0 )
    {
      const int closeFailed = fclose( mFile );

      if ( closeFailed )
      {
        DALI_LOG_WARNING( "File close failed for FILE: \"%p\".\n", static_cast<void*>(mFile) );
      }
      mFile = 0;
    }
  }

public:
  /**
   * @return The FILE* guarded by this object.
   */
  FILE* GetFile()
  {
    return mFile;
  }

private:

  // Undefined
  FileCloser( const FileCloser& fileCloser );

  // Undefined
  FileCloser& operator=( const FileCloser& fileCloser );

private:
  FILE* mFile;
  Dali::Vector<char> mFileBuffer;
};

} // namespace Platform

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_PLATFORM_FILECLOSER_H
