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
#include "data-cache-io.h"

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// EXTERNAL INCLUDES
#include <stdio.h>
#include <string.h>
#include <sys/file.h>   // flock()
#include <unistd.h>     // ftruncate()
#include <errno.h>


namespace Dali
{

namespace TizenPlatform
{

namespace DataCacheIo
{

namespace // unnamed name space
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_DATA_CACHE_IO");
#endif

const std::size_t DESCRIPTION_SIZE( 16 );                         ///< Description size
const std::size_t VERSION_SIZE( sizeof(__DATE__ "-" __TIME__) );  ///< (date-time)
const std::size_t COMPRESSION_MODE_SIZE( 10 );                    ///< off | RLE
const std::size_t NUMBER_ENTRIES_SIZE( sizeof(unsigned int) );    ///< number of entries in index file
const std::size_t OFFSET_SIZE( sizeof(unsigned int) );            ///< size of the offset field

const char INDEX_FILE_DESCRPITION[ DESCRIPTION_SIZE+1 ] = "Index File";   ///< String description added to the header
const char DATA_FILE_DESCRPITION[ DESCRIPTION_SIZE+1 ] =  "Data File";    ///< String description added to the header

const char FILE_VERSION[ VERSION_SIZE+1 ] =  __DATE__ "-" __TIME__;       ///< Updates with each build

typedef Dali::Platform::DataCache::DataKey DataKey;
typedef Dali::Platform::DataCache::Data Data;
typedef Dali::Platform::DataCache::CompressionMode CompressionMode;
typedef unsigned int DataOffset;
typedef unsigned int DataLength;

/**
 * File header, for both index and data files.
 */
struct FileHeader
{
  char description[ DESCRIPTION_SIZE+1 ];
  char version[ VERSION_SIZE+1 ];
  char compressionMode[ COMPRESSION_MODE_SIZE+1 ];
};

/**
 * Data meta information
 */
struct DataMeta
{
  DataMeta() :mKey(0), mLength(0)
  {}
  DataMeta(DataKey key, DataLength length)
  :mKey(key), mLength(length)
  {}
  DataKey mKey;
  DataLength mLength;
};

const std::size_t FILE_HEADER_SIZE( sizeof (FileHeader ));  ///< header size
const std::size_t KEY_META_SIZE( sizeof (KeyMeta ));        ///< key meta size
const std::size_t DATA_META_SIZE( sizeof (DataMeta ));      ///< Data meta size

/**
 * Helper to return the string for a compression mode.
 */
const char *GetCompressString( CompressionMode mode)
{
  return (mode == Dali::Platform::DataCache::COMPRESSION_OFF) ? "comp off" : "RLE on";
}

/**
 * Helper to read a file header
 */
bool ReadHeader( FILE* file,
                  const char *fileDescription,
                  CompressionMode compressionMode)
{
  FileHeader header;

  if( fseek( file, 0, SEEK_SET ) )
  {
    DALI_LOG_ERROR("Error seeking to start of file\n");
    return false;
  }

  std::size_t read = fread( &header , FILE_HEADER_SIZE, 1, file );
  if( read == 0 )
  {
    // This will happen if the file has just been created.
    return false;
  }

  // make sure description, version and compression mode all match
  if( ( strncmp( header.description, fileDescription, DESCRIPTION_SIZE ) == 0) &&
      ( strncmp( header.version,     FILE_VERSION, VERSION_SIZE ) == 0) &&
      ( strncmp( header.compressionMode, GetCompressString( compressionMode ), COMPRESSION_MODE_SIZE )) == 0 )
  {
    return true;
  }
  // this will happen if the version changes
  DALI_LOG_WARNING("Header miss-match %s\n",fileDescription);
  return false;

}

/**
 * Helper to write a file header
 */
void WriteHeader( FILE* file,
                  const char *fileDescription,
                  CompressionMode compressionMode)
{
  // Seek to beginning of file
  rewind( file );

  FileHeader header;
  memset( &header, 0, FILE_HEADER_SIZE );

  strncpy( header.description, fileDescription, DESCRIPTION_SIZE );
  strncpy( header.version,     FILE_VERSION,    VERSION_SIZE );
  strncpy( header.compressionMode, GetCompressString( compressionMode ), COMPRESSION_MODE_SIZE );

  std::size_t NUMBER_OF_HEADERS( 1 );
  std::size_t writeCount = fwrite( &header, FILE_HEADER_SIZE, NUMBER_OF_HEADERS, file );
  if( writeCount != NUMBER_OF_HEADERS )
  {
    DALI_LOG_ERROR( "Error '%s' writing file header\n", strerror( errno ) );
  }
}

/**
 * Helper to write data meta information to the data file (key, length).
 * Data file format is
 * | KEY | DATA LENGTH | DATA |
 */
void WriteDataMeta( FILE* dataFile, DataKey key, const Data& data)
{
  DataMeta meta( key, data.length );

  DALI_ASSERT_ALWAYS( data.length != 0 );

  std::size_t NUMBER_OF_DATA_METAS( 1 );
  // write the key and data length
  std::size_t writeCount = fwrite( &meta, DATA_META_SIZE, NUMBER_OF_DATA_METAS, dataFile );
  if( writeCount != NUMBER_OF_DATA_METAS )
  {
    DALI_LOG_ERROR( "Error '%s' writing data meta\n", strerror( errno ) );
  }
}

/**
 * Helper to read data meta information
 */
bool ReadDataMeta(FILE* dataFile, DataKey& key, Data& data)
{
  DataMeta meta;

  // read the key and data length
  std::size_t read = fread( &meta, DATA_META_SIZE, 1, dataFile );
  if( read == 0 )
  {
    return false;
  }

  key = meta.mKey;
  data.length = meta.mLength;

  return true;
}

/**
 *  Helper to check a data section is valid
 */
bool CheckDataIsValid(FILE* dataFile,
              unsigned int offset,
              DataKey key,
              Data& data,
              unsigned int bufferSize,
              bool checkDataExists)
{
  DataKey loadedKey(0);

  // move to the file pointer to where the data is held
  int ret = fseek( dataFile , offset, SEEK_SET);
  if( ret != 0 )
  {
    // something has been corrupted.
    DALI_LOG_ERROR("data offset is invalid (corrupt?)\n");
    return false;
  }

  // read the key, and data size at the current file position to make sure it's valid
  bool ok = ReadDataMeta( dataFile, loadedKey, data );
  if( !ok )
  {
    DALI_LOG_ERROR("data meta corrupt\n");
    return false;
  }

  // check the key matches
  if( loadedKey != key )
  {
    DALI_LOG_ERROR("Key miss-match in data file\n");
    return false;
  }

  // make sure the size  is valid
  if( data.length > bufferSize )
  {
    DALI_LOG_ERROR("Data size is corrupt in data file %d data size, buffer size %d \n",data.length, bufferSize);
    return false;
  }

  if( checkDataExists )
  {
    // check if the file is big enough to support the data
    ret = fseek( dataFile , data.length, SEEK_CUR);
    if( ret != 0 )
    {
      // something has been corrupted.
      DALI_LOG_ERROR("data field invalid or file truncated\n");
      return false;
    }
  }
  return true;
}

/**
 *  Helper to check the index file body
 */
bool CheckIndexFileBody( FILE* indexFile,
                          DataKey &lastKey,
                          DataOffset &lastOffset,
                          unsigned int& numberEntries,
                          unsigned int maxNummberEntries )
{

  bool ok = ReadNumberEntries( indexFile, numberEntries );
  if( !ok )
  {
    return false;
  }
  if( numberEntries == 0 )
  {
    return true;
  }
  if( numberEntries >= maxNummberEntries )
  {
    DALI_LOG_ERROR("number entries > maxNummberEntries in index file, entries: %d \n", numberEntries);
    return false;
  }

  // seek to offset where the last index is held
  unsigned int lastKeyMetaOffset( FILE_HEADER_SIZE + NUMBER_ENTRIES_SIZE );
  lastKeyMetaOffset+= (numberEntries - 1) * KEY_META_SIZE;

  int ret = fseek( indexFile, lastKeyMetaOffset, SEEK_SET );
  if( ret != 0 )
  {
    DALI_LOG_ERROR("index file corrupt, failed to seek to last index\n");
    return false;
  }

  // read the key + offset
  ok = ReadKey( indexFile, lastKey, lastOffset );
  if( !ok )
  {
    return false;
  }

  if( fgetc( indexFile ) != EOF)
  {
    // data index is corrupt, it contains more entries than are recorded in numberEntries field
    DALI_LOG_ERROR("index has  more entries than recorded\n");
    return false;
  }

  return true;

}

/**
 * Helper to check the files are valid
 */
bool CheckFilesAreValid( FILE* indexFile,
                         FILE* dataFile,
                         Dali::Platform::DataCache::CompressionMode compressionMode,
                         unsigned int maxDataSize,
                         unsigned int maxNummberEntries)
{

  bool ok = ReadHeader( indexFile, INDEX_FILE_DESCRPITION, compressionMode );
  if(!ok)
  {
    return false;
  }
  ok = ReadHeader( dataFile, DATA_FILE_DESCRPITION, compressionMode );
  if(!ok)
  {
    return false;
  }

  DataKey key(0);
  Data data;
  DataOffset offset(0);
  unsigned int numEntries(0);

  // check the index file is ok, and find out the last entry in it
  ok = CheckIndexFileBody( indexFile, key, offset, numEntries, maxNummberEntries );
  if( !ok)
  {
    return false;
  }

  // if number entries = 0, then check the data file has no data after it's header
  if( numEntries == 0)
  {
   if( fgetc( dataFile ) != EOF)
   {
     // data file is corrupt, it contains data thas has no entries
     return false;
   }
   else
   {
     // no further checks needed
     return true;
   }
  }

  // check if the data is valid, this doesn't read the data, just checks the file
  // has the correct length
  ok = CheckDataIsValid( dataFile, offset, key, data , maxDataSize, true);
  if( !ok )
  {
    return false;
  }
  // check we're at the end of the data file
  if( fgetc( dataFile ) != EOF)
  {
    return false;
  }
  return true;
}

/**
 *  Helper to lock a file
 */
bool LockFile(FILE* file)
{
  // Place an exclusive lock. Only one process may hold an
  // exclusive lock for a given file at a given time.
  // It places an advisory locks only; given suitable permissions on a file,
  // a process is free to ignore the use of flock() and perform I/O on the file.
  int ret = flock( fileno(file), LOCK_EX);
  if( 0 != ret )
  {
    DALI_LOG_ERROR("Error '%s' locking cache file\n", strerror( errno ) );
    return false;
  }

  return true;
}
}; // unnamed namespace
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void CheckAndRepairFiles( const std::string& indexFileName,
    const std::string& dataFileName,
    Dali::Platform::DataCache::CompressionMode compressionMode,
    unsigned int maxDataSize,
    unsigned int maxNummberEntries )
{
  // It's possible if the power is lost or the has been process killed while writing to the index / data files
  // they could be corrupt. The following quickly tests to make sure the headers
  // and the last entry in the index / data file is valid.

  FILE* dataFile = OpenFile( dataFileName, DATA_FILE, LOCK_FILE, READ_WRITE, CREATE_IF_MISSING );
  if( dataFile != NULL )
  {
    FILE* indexFile = OpenFile( indexFileName, INDEX_FILE, NO_LOCK, READ_WRITE, CREATE_IF_MISSING );
    if( indexFile != NULL )
    {
      if( ! CheckFilesAreValid( indexFile, dataFile, compressionMode, maxDataSize, maxNummberEntries) )
      {
        ReCreateFiles( indexFile, dataFile, compressionMode);
      }
      fclose( indexFile);
    }
    fclose( dataFile);
  }
}

FILE* OpenFile(const std::string& fileName,
              FileType fileType,
              LockMode lockMode,
              FileOpenMode fileMode,
              FileCreationMode creationMode )
{
  // open and the file, for reading(r)  binary (b) (+) update
  const char* openMode( (fileMode == READ_WRITE) ? "r+b" : "rb" );
  FILE* file = fopen( fileName.c_str(), openMode );
  if( file == NULL )
  {
    if((fileMode == READ_WRITE) && (creationMode == CREATE_IF_MISSING) )
    {
      // Attempt to create a new file for reading / writing
      DALI_LOG_INFO( gLogFilter , Debug::Concise, "Creating new file: %s\n",fileName.c_str());
      file = fopen( fileName.c_str(), "w+b");
      if( file == NULL )
      {
        DALI_LOG_ERROR( "Failed to create file %s, with error '%s'\n", fileName.c_str(), strerror( errno ) );
      }
    }
  }

  if( file != NULL )                              // open file?
  {
    if( lockMode == LOCK_FILE )                   // and file requires lock?
    {
      bool fileLockSucceeded = LockFile( file );  // lock file
      if( ! fileLockSucceeded )
      {
        DALI_LOG_ERROR("Failed to lock file %s\n", fileName.c_str() );
        fclose( file );
        file = NULL;
      }
    }
  }

  return file;
}

void ReCreateFiles(FILE* indexFile, FILE* dataFile, CompressionMode compressionMode )
{
  int resultStatus(0);

  // Truncate to zero and then write new headers.

  // index file
  resultStatus = ftruncate( fileno( indexFile ), 0);
  if( resultStatus )
  {
    DALI_LOG_ERROR( "Error '%s' truncating index file\n", strerror( errno ) );
  }
  else
  {
    WriteHeader( indexFile, INDEX_FILE_DESCRPITION, compressionMode);
    WriteNumberEntries( indexFile, 0 );
  }

  // data file
  resultStatus = ftruncate( fileno( dataFile ), 0);
  if( resultStatus )
  {
    DALI_LOG_ERROR( "Error '%s' truncating data file\n", strerror( errno ) );
  }
  else
  {
    WriteHeader( dataFile, DATA_FILE_DESCRPITION, compressionMode);
  }
}

void PrepareForWritingData( FILE* dataFile, FILE* indexFile )
{
  if( fseek(dataFile, 0, SEEK_END) )
  {
    DALI_LOG_ERROR("Error seeking to start of dataFile\n");
  }

  if( fseek(indexFile, 0, SEEK_END) )
  {
    DALI_LOG_ERROR("Error seeking to start of indexFile\n");
  }
}

unsigned int WriteData( FILE* dataFile, DataKey key, const Data& data )
{
  // get the current file offset
  unsigned int offset( 0u );
  long positionIndicator = ftell( dataFile );
  if( positionIndicator > -1L )
  {
    offset = static_cast<unsigned int>(positionIndicator);
  }
  else
  {
    DALI_LOG_ERROR("Error finding the current file offset\n");
  }

  // write the key first and data size
  WriteDataMeta( dataFile, key, data);

  // write the data
  fwrite( data.data, data.length, 1, dataFile );

  return offset;
}

bool ReadData( FILE* dataFile,
              unsigned int offset,
              DataKey key,
              Data& data,
              unsigned char* dataBuffer,
              unsigned int bufferSize)
{

  // read the data meta information first, ensure it's valid and will
  // fit in to the dataBuffer
  bool ok = CheckDataIsValid( dataFile, offset, key, data, bufferSize, false);
  if( !ok )
  {
    // CheckDataIsValid will log an error
    return false;
  }

  // read the actual data
  std::size_t read = fread( dataBuffer , data.length, 1, dataFile  );
  if( read == 0 )
  {
    DALI_LOG_ERROR("Note enough data in data file (corrupt)\n");
    return false;
  }
  return true;
}

bool ReadEntries(FILE *indexFile, KeyMeta* meta, unsigned int startIndex, unsigned int count)
{
  // seek past the header and existing entries to startIndex
  unsigned int seekOffset = ( FILE_HEADER_SIZE + NUMBER_ENTRIES_SIZE )+ startIndex* KEY_META_SIZE;

  int ret = fseek( indexFile , seekOffset , SEEK_SET);
  if( ret != 0 )
  {
    DALI_LOG_ERROR("index file corrupt, fseek failed\n");
    return false;
  }
  std::size_t readCount = fread( meta, KEY_META_SIZE, count, indexFile);
  if( readCount != count )
  {
    DALI_LOG_ERROR("index file missing data\n");
    return false;
  }
  return true;
}

bool ReadNumberEntries( FILE* indexFile, unsigned int &numEntries )
{
  int ret = fseek( indexFile , FILE_HEADER_SIZE, SEEK_SET);
  if( ret != 0 )
  {
    DALI_LOG_ERROR("index file corrupt, fseek failed\n");
    return false;
  }

  std::size_t read = fread( &numEntries, NUMBER_ENTRIES_SIZE , 1, indexFile);
  if( read == 0)
  {
    DALI_LOG_ERROR("index file corrupt, fread failed\n");
    return false;
  }
  return true;
}

void WriteNumberEntries( FILE* indexFile, unsigned int numEntries )
{
  // entries field is first bytes after header
  if ( fseek( indexFile , FILE_HEADER_SIZE, SEEK_SET) )
  {
    DALI_LOG_ERROR("index file corrupt, fseek failed\n");
  }

  fwrite( &numEntries, NUMBER_ENTRIES_SIZE , 1, indexFile);
}

bool ReadKey( FILE* indexFile, DataKey &key, unsigned int& offset )
{
  KeyMeta meta;

  std::size_t read = fread( &meta, KEY_META_SIZE, 1, indexFile );
  if( read == 0 )
  {
    DALI_LOG_ERROR("index file corrupt, failed to read key meta\n");
    return false;
  }
  key = meta.mKey;
  offset = meta.mOffset;
  return true;
}

void WriteKey(FILE* indexFile, DataKey key, unsigned int offset)
{
  KeyMeta meta( key, offset );

  // write the key first
  fwrite( &meta, KEY_META_SIZE, 1, indexFile );
}

std::size_t GetHeaderSize()
{
  return FILE_HEADER_SIZE;
}
} // namespace DataCacheIO

} // namespace TizenPlatform

} // namespace Dali
