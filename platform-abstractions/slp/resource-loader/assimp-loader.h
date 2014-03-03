#ifndef __DALI_SLP_PLATFORM_ASSIMP_LOADER_H__
#define __DALI_SLP_PLATFORM_ASSIMP_LOADER_H__

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

#include <stdio.h>

#include <assimp/IOStream.h>
#include <assimp/IOSystem.h>


namespace Dali
{
namespace SlpPlatform
{

class AssetLoader;

/**
 * Adaptor class to allow assimp to use platform specific loader
 * on current slp platform it is a thin wrapper over the stdio functions
 */
class AssimpIOStream : public Assimp::IOStream
{
protected:
  /**
   * Constructor
   */
  AssimpIOStream();

public:
  /**
   * Constructor
   * @param[in] loader An AssetLoader
   */
  AssimpIOStream(FILE* loader);

  /**
   * Destructor.
   */
  virtual ~AssimpIOStream();

  /**
   * Read from the file.
   * @param pvBuffer  Destination buffer
   * @param pSize     Size of each element in bytes
   * @param pCount    Number of elements
   * @return          Number of element read.
   */
  size_t Read(void *pvBuffer, size_t pSize, size_t pCount);

  /**
   * Write to the file.
   * @param pvBuffer  Source buffer
   * @param pSize     Size of each element in bytes
   * @param pCount    Number of elements
   * @return          Number of element written.
   */
  size_t  Write(const void *pvBuffer, size_t pSize, size_t pCount);

  /**
   * Set the read/write cursor of the file.
   * @param pOffset The amount to move the cursor
   * @param pOrigin Specifies the origin, start of stream, current position or end of stream
   * @return 0 if successful.
   */
  aiReturn Seek(size_t pOffset, aiOrigin pOrigin);

  /**
   * Get the current position of the read/write cursor.
   * @return The current position of the read/write cursor.
   */
  size_t Tell() const;

  /**
   * Returns file size.
   * @return The file size.
   */
  size_t FileSize() const;

  /**
   * Flush the contents of the file buffer (for writers) See fflush() for more details.
   */
  void Flush();

private:
  FILE*  mLoader;

}; // class AssimpIOStream

/**
 * Interface to the file system.
 */
class AssimpIOSystem : public Assimp::IOSystem
{
public:
  /**
   * Default constructor.
   * Create an instance and assign it to on Assimp::Importer using
   * Assimp::Importer::SetIOHandler
   */
  AssimpIOSystem();

  /**
   *  Virtual destructor.
   */
  virtual ~AssimpIOSystem();
public:

  /**
   * Tests for the existence of a file at the given path.
   * @param pFile The file name and path.
   * @return true if the file exists.
   */
  bool Exists(const std::string& pFile) const;

  /**
   * @copydoc Exists(const std::string&)const
   */
  bool Exists(const char* pFile) const;

  /**
   * Returns the system specific directory separator
   * @return System specific directory separator
   */
  char getOsSeparator() const;

  /**
   * Open a file with a given path.
   * @param pFile Path to the file
   * @param pMode Desired file I/O mode.
   * @return new AssimpIOStream
   */
  Assimp::IOStream* Open(const char* pFile, const char* pMode = "rb");

  /**
   * @copydoc Open(const char*,const char*)
   */
  Assimp::IOStream* Open(const std::string& pFile, const std::string& pMode = std::string("rb"));

  /**
   * Closes the given file and releases all resources associated with it.
   * @param pFile The file instance previously created by Open().
   */
  void Close(Assimp::IOStream* pFile);
}; // class AssimpIOSystem

} // namespace SlpPlatform
} // namespace Dali

#endif // __DALI_SLP_PLATFORM_ASSIMP_LOADER_H__
