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
#include "data-cache-impl.h"

// INTERNAL INCLUDES
#include "data-cache-io.h"
#include "data-compression.h"
#include <dali/integration-api/debug.h>


// EXTERNAL INCLUDES
#include <boost/functional/hash.hpp>
#include <boost/thread.hpp>

namespace Dali
{

namespace SlpPlatform
{

namespace
{

boost::mutex mFileCheckMutex; ///< used to ensure only one thread at a time is allowed to check a file for corruption
std::vector<  std::size_t /* hash value*/>  fileCheckList;  ///< List of files hashes that have been checked

bool FileCheckedForCorruption( std::size_t hashValue )
{
  return std::find( fileCheckList.begin(), fileCheckList.end(), hashValue ) != fileCheckList.end();
}

void SetExistsFlag( Dali::Platform::DataCache::DataVector& dataVector, bool exists)
{
  for( unsigned int i = 0, size = dataVector.size(); i < size; ++i)
  {
    dataVector[i].exists = exists;
  }
}
#ifdef DEBUG_ENABLED
void CheckForDuplicateKeys(const Dali::Platform::DataCache::KeyVector& keyVector )
{
  std::map< Dali::Platform::DataCache::DataKey, bool> keyLookup;
  for(unsigned int i = 0, size = keyVector.size(); i<size; ++i)
  {
    Dali::Platform::DataCache::DataKey key = keyVector[i];
    DALI_ASSERT_ALWAYS( keyLookup.find( key ) == keyLookup.end() && "DataCache::Add() called with duplicate keys");
    keyLookup[ key ] = 1;
  }
}
#else
void CheckForDuplicateKeys(const Dali::Platform::DataCache::KeyVector& keyVector )
{
  // don't check for duplicate keys in release mode
}
#endif

} // unnamed namespace

DataCache::DataCache( Dali::Platform::DataCache::ReadWriteMode mode,
                      Dali::Platform::DataCache::CompressionMode compressionMode,
                      const std::string& fileName,
                      unsigned int dataSize,
                      unsigned int maxNumberEntries )
:mIndexFile( fileName +".index"),
 mDataFile( fileName +".data"),
 mNumberEntries( 0 ),
 mMaxNumberEntries( maxNumberEntries ),
 mMaxDataSize( dataSize ),
 mEncodeBuffer( NULL ),
 mDecodeBuffer( NULL ),
 mEncodeBufferSize(0),
 mMode( mode ),
 mCompressionMode( compressionMode )
{
  mEncodeBufferSize = GetMaxEncodedDataSize();
  mEncodeBuffer = new unsigned char [ mEncodeBufferSize ];

  if( compressionMode != COMPRESSION_OFF )
  {
    mDecodeBuffer = new unsigned char[ mMaxDataSize ];
  }


    // check for corruption and repair if required
  CorruptedFileCheck();
}

DataCache::~DataCache()
{
  delete []mDecodeBuffer;
  delete []mEncodeBuffer;
}

void DataCache::Find( const KeyVector& keyVector, DataVector& dataVector )
{
  // create the data objects, and set the data.exists flag to false
  dataVector.resize( keyVector.size() );
  SetExistsFlag( dataVector, false );

  // the aim is to avoid loading the index file, unless the key is not found in our memory lookup
  bool indexFileLoaded( false );

  // if the memory lookup is empty, this will try and load the index file
  InitialiseLookup( indexFileLoaded );

  if( mLookup.empty() )
  {
    // the index file has not been created
    return;
  }

  // open the data file for read binary
  FILE* dataFile = DataCacheIo::OpenFile( mDataFile.c_str(), DataCacheIo::DATA_FILE, DataCacheIo::NO_LOCK, DataCacheIo::READ_ONLY );
  if( NULL == dataFile )
  {
    DALI_LOG_ERROR("Failed to open data file '%s' for reading\n", mDataFile.c_str() );
    return;
  }

  // read the data for each key, if it exists
  ReadFromDataFile( dataFile, indexFileLoaded, keyVector, dataVector );

  // close the data file
  fclose( dataFile );
}

// Add()
//
// Stage 1.
// - Lock the data file
// - Read the latest index file to check for any updates (from other threads/processes)
//
// Stage 2.
// - Insert data to the end of the file
// - insert the new key / data offset to index file
// - Increase the number of entries
//
// Stage 3.
// - Close the index file
// - Close and unlock the data file.
//
// The order ensures that:
// We never write duplicate data.
// Every entry in the index file, always has valid data.
// The last chunk of data written to disk is the number of entries in the index file.
// The write operations are atomic. If many threads are reading from the index file
// while the write is in progress, depending on timing they will either read
// n number of entries, or n+1 number of entries. In both cases the data will be valid.
//
// If two processes try to save at the same time, one is blocked due to the lock
// on the data file.
//
// If the Process dies during this operation, the file is closed automatically
// and the lock released
//
void DataCache::Add( const KeyVector& keyVector, const DataVector& dataVector )
{
  DALI_ASSERT_DEBUG( keyVector.size() == dataVector.size() && !keyVector.empty() );

  CheckForDuplicateKeys( keyVector );

  // copy the vector, so we can modify the exists flag
  DataVector tempDataVector( dataVector );
  SetExistsFlag( tempDataVector, false );

  FILE* dataFile( NULL );
  FILE* indexFile( NULL );

  // open both data and index files, the dataFile will be locked..
  // ..on failure both dataFile and indexFile will be NULL
  OpenIndexAndDataForWriting( &dataFile, &indexFile );

  if( dataFile != NULL && indexFile != NULL )
  {
    // update our lookup table with the one on disk
    if( ReLoadIndexFile( indexFile ) )
    {
      // Ensure we don't write the same key/data twice if another process has already done it.
      // if the key/data already exists, the data exists flag will be set to true
      FindExistingData( keyVector, tempDataVector );

      // write the data, and update the index
      WriteDataAndUpdateIndex( keyVector, tempDataVector, dataFile, indexFile );

      // close the index file
      fclose( indexFile );

      // closing the data file will release the lock
      fclose( dataFile );
    }
    else // probable corruption detected
    {
      CloseAndReinitializeFiles( &indexFile, &dataFile );
    }
  }
}

void DataCache::CorruptedFileCheck( ) const
{
  // only allow one thread at a time to perform the corruption check
  mFileCheckMutex.lock();

  boost::hash<const std::string>  hasher;
  std::size_t hashValue = hasher( mDataFile );

  // If the file has been checked for corruption already, do not check it again.
  if( ! FileCheckedForCorruption( hashValue ))
  {
    // make sure the files are the correct version and haven't been corrupted
    DataCacheIo::CheckAndRepairFiles(  mIndexFile, mDataFile, mCompressionMode, mEncodeBufferSize, mMaxNumberEntries);

    // add to the list of checked files
    fileCheckList.push_back( hashValue );
  }

  mFileCheckMutex.unlock();
}

void DataCache::InitialiseLookup( bool &indexFileLoaded )
{
  // ensure the index file has been loaded at least once.
  indexFileLoaded = false;

  if( mLookup.empty() )
  {
    // if the memory lookup is empty, try loading the index from file
    LoadIndexFile();
    indexFileLoaded = true;
  }
}

void DataCache::LoadIndexFile()
{
  FILE* indexFile = DataCacheIo::OpenFile( mIndexFile,
                                           DataCacheIo::INDEX_FILE,
                                           DataCacheIo::NO_LOCK,
                                           DataCacheIo::READ_ONLY );

  // the index file may not exist yet
  if( !indexFile )
  {
    return;
  }

  // re-load it to check for new entries
  if( ReLoadIndexFile( indexFile ) )
  {
    fclose( indexFile );
  }
  else
  {
    CloseAndReinitializeFiles( &indexFile, NULL );
  }
}

bool DataCache::ReadFromDataFile( FILE* dataFile,
                                  bool indexFileLoaded,
                                  const KeyVector& keyVector,
                                  DataVector& dataVector
                                  )
{
  // for each key, try and find it in  memory lookup.
  // if a key is missing then see if the index file has been updated by another thread / process

  for( unsigned int index = 0, vectorSize = keyVector.size(); index < vectorSize; ++index )
  {
    DataKey key = keyVector[index];

    KeyLookup::const_iterator keyIter =  mLookup.find( key);

    // if a key isn't found in the lookup and we haven't tried loading the index file yet, do it now.
    if( ( keyIter == mLookup.end() ) && ( !indexFileLoaded ) )
    {
      LoadIndexFile();
      indexFileLoaded = true;
      keyIter = mLookup.find( key );
    }

    // if we have found key, fill in the data
    if( keyIter != mLookup.end() )
    {
      // key.second holds the offset in the data file of the binary data
      unsigned int offset = (*keyIter).second;
      Data& data(  dataVector[ index]  );

      DALI_ASSERT_DEBUG( data.exists == false);

      bool ok = ReadData( dataFile, offset , key, data);
      if( !ok)
      {
        return false;
      }
      data.exists = true;
    }
  }
  return true;
}

void DataCache::FindExistingData( const KeyVector& keyVector, DataVector& dataVector) const
{
  if( mLookup.empty() )
  {
    return;
  }
  // for each key, try and find it in  memory lookup.
  // at the same time check for duplicate keys in the key vector
  for( unsigned int index = 0, vectorSize = keyVector.size(); index < vectorSize; ++index )
  {
    if( mLookup.find( keyVector[index] ) == mLookup.end() )
    {
      dataVector[index].exists = false;
    }
    else
    {
      dataVector[index].exists = true;
    }
  }
}

void DataCache::OpenIndexAndDataForWriting(FILE** dataFile, FILE** indexFile )
{
  // open and lock  the data file
  *dataFile = DataCacheIo::OpenFile( mDataFile, DataCacheIo::DATA_FILE, DataCacheIo::LOCK_FILE, DataCacheIo::READ_WRITE );
  if( *dataFile == NULL )
  {
    DALI_LOG_ERROR( "Failed to open and lock %s\n", mDataFile.c_str() );
  }
  else
  {
    // open the index file, there is no need in locking it as well
    // as the data file lock is used as a global lock
    *indexFile = DataCacheIo::OpenFile( mIndexFile, DataCacheIo::INDEX_FILE, DataCacheIo::NO_LOCK, DataCacheIo::READ_WRITE );
    if( *indexFile == NULL )
    {
      DALI_LOG_ERROR( "Failed to open %s\n", mIndexFile.c_str() );

      fclose( *dataFile );
      *dataFile = NULL;
    }
  }
}

void DataCache::WriteDataAndUpdateIndex( const KeyVector& keyVector,
                                         const DataVector& dataVector,
                                         FILE* dataFile,
                                         FILE* indexFile
                                         )
{
  // seek to the end of both files for adding new data
  DataCacheIo::PrepareForWritingData( dataFile, indexFile );

  unsigned int newEntries( 0 );

  for ( unsigned int index = 0, arraySize = keyVector.size(); index < arraySize; ++index )
  {
    const Data& data( dataVector[index] );
    const DataKey& key( keyVector[index] );

    // write the data, if the data doesn't already exist
    if( data.exists == false )
    {
      newEntries++;

      // write the data
      unsigned int offset = WriteData( dataFile, key, data );

      // update the index file with the new key / offset.
      DataCacheIo::WriteKey( indexFile, key, offset );

      // write to our memory lookup
      mLookup[ key ] = offset;

    }
  }
  if( newEntries )
  {
    mNumberEntries+= newEntries;

    // assert if max entries is exceeded, the cache files will be deleted on restart
    DALI_ASSERT_ALWAYS( mNumberEntries <= mMaxNumberEntries);

    // write the number of entries
    DataCacheIo::WriteNumberEntries( indexFile, mNumberEntries );

  }
}

bool DataCache::ReadData( FILE* dataFile,
                unsigned int offset,
                DataCache::DataKey key,
                DataCache::Data& data) const
{
  unsigned char *dataBuffer( mEncodeBuffer );

  // load the data in to the encode buffer
  bool ok = DataCacheIo::ReadData( dataFile, offset, key, data, mEncodeBuffer, mEncodeBufferSize );
  if( !ok )
  {
    DALI_LOG_ERROR("data file corrupt \n");
    return false;
  }

  if( mCompressionMode == RUN_LENGTH_ENCODING )
  {
    std::size_t bytesDecoded(0);
    ok = DataCompression::DecodeRle( mEncodeBuffer, data.length, mDecodeBuffer, mMaxDataSize, bytesDecoded);
    if( !ok )
    {
      DALI_LOG_ERROR("data file corrupt \n");
      return false;
    }
    // set the dataBuffer to point to the decoded data
    dataBuffer = mDecodeBuffer;

    // set the data.length to be the decoded length
    data.length = bytesDecoded;
  }

  // allocate and copy the data in the data structure
  data.data = new unsigned char[ data.length ];
  memcpy( data.data, dataBuffer, data.length );

  return true;
}

unsigned int DataCache::WriteData( FILE *dataFile, DataKey key, const Data &data) const
{
  unsigned int offset;

  // check data is not too large
  DALI_ASSERT_DEBUG( data.length <= mMaxDataSize );

  if( mCompressionMode == COMPRESSION_OFF )
  {
    // write the data
    offset = DataCacheIo::WriteData( dataFile, key, data );
  }
  else
  {
    // compress, then write the data
    std::size_t compressedLength( 0 );
    DataCompression::EncodeRle( data.data, data.length, mEncodeBuffer, mEncodeBufferSize, compressedLength);
    Data compressedData;
    compressedData.SetData( mEncodeBuffer, compressedLength );
    offset = DataCacheIo::WriteData( dataFile, key, compressedData );
  }
  return offset;
 }

bool DataCache::ReLoadIndexFile( FILE *indexFile )
{
  // First check if our memory cache is out of sync with what is in the file
  // another process may have added some new entries
  unsigned int numberEntries(0) ;

  bool ok = DataCacheIo::ReadNumberEntries( indexFile, numberEntries);
  if( !ok )
  {
    return false;
  }
  if( numberEntries == mNumberEntries )
  {
    // if there is no new entries return
    return true;
  }

  if( numberEntries < mNumberEntries )
  {
    // this should not happen, but if it does delete the cache files and assert
    DALI_LOG_ERROR("numberEntries too small \n");
    return false;
  }
   ok = ReadNewEntries( indexFile, (  numberEntries - mNumberEntries) );
   if( !ok )
   {
     return false;
   }
   return true;
}

bool DataCache::ReadNewEntries( FILE* indexFile, unsigned int newEntries)
{
  // instead of performing an I/O operation per entry, we read all new entries
  // in a single read.
  DataCacheIo::KeyMeta* keyMeta = new DataCacheIo::KeyMeta[ newEntries ];

  // read newEntries worth of data from start index mNumberEntries.
  bool ok = DataCacheIo::ReadEntries(indexFile, keyMeta, mNumberEntries, newEntries);
  if( ok )
  {
    for( unsigned int i = 0; i< newEntries; ++i )
    {
      DataKey key( keyMeta[i].mKey );
      unsigned int offset (keyMeta[i].mOffset );

      // check the key hasn't been stored before
      DALI_ASSERT_DEBUG( mLookup.end() == mLookup.find( key) );

      // create a new key with the offset
      mLookup[ key ] = offset;
      mNumberEntries++;
    }
  }
  else
  {
    DALI_LOG_ERROR("read new entries failed \n");
  }

  delete []keyMeta;
  return ok;
}

//
//  If file corruption is detected while the thread is running
//  and after CorruptedFileCheck() has been performed,
//  then delete the files.
//  This should only occur if:
//  - 2 or more Dali Applications are running
//  - One of them is killed half way through a DataCache::Add operation,
//  corrupting the files.
//  - The other app then tries to use the corrupt files, and detects an error.
//   When either app restarts the cache will be recreated.
//
void DataCache::CloseAndReinitializeFiles( FILE** indexFile, FILE** dataFile ) const
{
  DALI_LOG_ERROR( "corrupt data file detected, re-created" );

  // close existing files
  if( indexFile && *indexFile )
  {
    fclose( *indexFile );
    *indexFile = NULL;
  }

  if( dataFile && *dataFile )
  {
    fclose( *dataFile );
    *dataFile = NULL;
  }

  // reopen files writable
  FILE* newDataFile = DataCacheIo::OpenFile( mDataFile, DataCacheIo::DATA_FILE, DataCacheIo::LOCK_FILE, DataCacheIo::READ_WRITE );
  if( newDataFile != NULL )
  {
    FILE* newIndexFile = DataCacheIo::OpenFile( mIndexFile, DataCacheIo::INDEX_FILE, DataCacheIo::NO_LOCK, DataCacheIo::READ_WRITE );
    if( newIndexFile != NULL )
    {
      // re-create the files with zero entries
      DataCacheIo::ReCreateFiles( newIndexFile, newDataFile, mCompressionMode );
      fclose( newIndexFile );
    }
    fclose( newDataFile );
  }
}

unsigned int DataCache::GetMaxEncodedDataSize() const
{
  if( mCompressionMode == COMPRESSION_OFF )
  {
    return mMaxDataSize;
  }
  else
  {
    // RLE can double the data size in worst case scenario
    return DataCompression::GetMaximumRleCompressedSize( mMaxDataSize );
  }
}

} // namespace SlpPlatform

} // namespace Dali

// Implementation of Dali::Platform::DataCache::New()
Dali::Platform::DataCache* Dali::Platform::DataCache::New(
    Dali::Platform::DataCache::ReadWriteMode mode,
    Dali::Platform::DataCache::CompressionMode compressionMode,
    const std::string& fileName,
    unsigned int maxDataSize,
    unsigned int maxNumberEntries)
{
  return new Dali::SlpPlatform::DataCache( mode, compressionMode, fileName, maxDataSize, maxNumberEntries );
}
