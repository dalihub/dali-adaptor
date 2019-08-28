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
 */

// CLASS HEADER
#include <dali/internal/adaptor-framework/common/file-stream-impl.h>

// EXTERNAL INCLUDES
#include <string>
#include <fstream>

#include <dali/integration-api/debug.h>

namespace Dali
{

FileStream::Impl::Impl(const std::string& filename, uint8_t mode)
: mFileName( filename ),
  mMode( mode ),
  mBuffer( nullptr ),
  mDataSize( 0 ),
  mFile( nullptr )
{
  DALI_ASSERT_DEBUG( !filename.empty() && "Can't open a empty filename." );
  DALI_ASSERT_DEBUG( mode != 0 && "No mode is undefined behaviour" );
}

FileStream::Impl::Impl(uint8_t* buffer, size_t dataSize, uint8_t mode)
: mMode( mode ),
  mBuffer( buffer ),
  mDataSize( dataSize ),
  mFile( nullptr )
{
  DALI_ASSERT_DEBUG( buffer != 0 && "Can't open file on null buffer." );
  DALI_ASSERT_DEBUG( dataSize > 0 && "Pointless to open file on empty buffer." );
  DALI_ASSERT_DEBUG( mode != 0 && "No mode is undefined behaviour." );
}

FileStream::Impl::Impl(Dali::Vector<uint8_t>& vector, size_t dataSize, uint8_t mode)
: mMode( mode ),
  mBuffer( nullptr ),
  mDataSize( dataSize ),
  mFile( nullptr )
{
  // Resize the buffer to ensure any null that gets written by
  // fmemopen is written past the end of any data that is written to the buffer.
  // (Workaround for a bug in Ubuntu that overwrites null to the last byte of the
  // data block regardless of whether binary mode was specified. Tizen doesn't write
  // null if binary mode is specified).

  ++mDataSize;
  vector.Resize( mDataSize );
  mBuffer = &vector[0];

  DALI_ASSERT_DEBUG( mBuffer != nullptr && "Can't open file on null buffer." );
  DALI_ASSERT_DEBUG( dataSize > 0 && "Pointless to open file on empty buffer." );
  DALI_ASSERT_DEBUG( mode != 0 && "No mode is undefined behaviour." );
}

FileStream::Impl::~Impl()
{
  if( mFile )
  {
    const int closeFailed = fclose( mFile );
    if( closeFailed )
    {
      DALI_LOG_WARNING( "File close failed for FILE: \"%p\".\n", static_cast<void*>( mFile ) );
    }

    mFile = nullptr;
  }

  if( mFileStream.is_open() )
  {
    mFileStream.close();
  }
}

std::iostream& FileStream::Impl::GetStream()
{
  if( mFile )
  {
    // return empty stream if FILE stream is open to avoid simultaneous access to the same file
    return mFileStream;
  }

  if( mFileStream.is_open() )
  {
    return mFileStream;
  }

  if( mBufferStream.rdbuf()->in_avail() )
  {
    return mBufferStream;
  }

  std::ios_base::openmode openMode;
  if( mMode & Dali::FileStream::WRITE )
  {
    openMode = ( std::ios::out | std::ios::ate );
  }
  else
  {
    openMode = std::ios::in;
  }

  if( mMode & Dali::FileStream::BINARY )
  {
    openMode |= std::ios::binary;
  }

  if( !mFileName.empty() )
  {
    mFileStream.open( mFileName, openMode );
    if( !mFileStream.is_open() )
    {
      DALI_LOG_WARNING( "stream open failed for: \"%s\", in mode: \"%d\".\n", mFileName, static_cast<int>( openMode ) );
    }
    return mFileStream;
  }
  else if( mBuffer )
  {
    mBufferStream.rdbuf()->pubsetbuf( reinterpret_cast<char*>( mBuffer ), mDataSize );
    if( !mBufferStream.rdbuf()->in_avail() )
    {
      DALI_LOG_WARNING( "File open failed for memory buffer at location: \"%p\", of size: \"%u\", in mode: \"%d\".\n",
          static_cast<void*>( mBuffer ), static_cast<unsigned>( mDataSize ), static_cast<int>( openMode ) );
    }
  }

  return mBufferStream;
}

FILE* FileStream::Impl::GetFile()
{
  if( mFileStream.is_open() || mBufferStream.rdbuf()->in_avail() )
  {
    // return empty FILE stream if the stream is open to avoid simultaneous access to the same file
    return nullptr;
  }

  if( mFile )
  {
    return mFile;
  }

  char openMode[16] = { 0 };
  int i = 0;

  if( mMode & Dali::FileStream::WRITE )
  {
    openMode[i++] = 'w';
  }
  else
  {
    openMode[i++] = 'r';
  }

  if( mMode & Dali::FileStream::BINARY )
  {
    openMode[i++] = 'b';
  }

  openMode[i++] = 0;

  if( !mFileName.empty() )
  {
    mFile = fopen( mFileName.c_str(), openMode );
    if( !mFile )
    {
      DALI_LOG_WARNING( "file open failed for: \"%s\", in mode: \"%s\".\n", mFileName, openMode );
    }
  }
  else if( mBuffer )
  {
    mFile = fmemopen( mBuffer, mDataSize, openMode );
    if( !mFile )
    {
      DALI_LOG_WARNING( "File open failed for memory buffer at location: \"%p\", of size: \"%u\", in mode: \"%s\".\n",
          static_cast<void*>( mBuffer ), static_cast<unsigned>( mDataSize ), openMode );
    }
  }

  return mFile;
}

} // Dali
