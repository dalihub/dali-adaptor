#ifndef DALI_FILE_LOADER_H
#define DALI_FILE_LOADER_H

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

// EXTERNAL INCLUDES
#include <string>
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace FileLoader
{
/**
 * @brief File type formats
 * The default format is binary
 */
enum FileType           ///< FileType format
{
  BINARY,               ///< File will be loaded as a binary
  TEXT                  ///< File will be loaded as text
};

/**
 * @brief Load the file. It will load it either as a binary or as a text
 *
 * @param[in] filename  Filename of the file to load.
 * @param[in] memblock  Dali::Vector containing the buffer loaded
 * @param[in] fileType  How we want to load the file. Binary or Text. Binary default
 * @return error code. 0 - Error, 1 - Ok
 *
 *
 */
DALI_ADAPTOR_API int ReadFile(const std::string& filename, Dali::Vector<char> & memblock, FileLoader::FileType fileType = BINARY);

/**
 * @brief Load the file. It will load it either as a binary or as a text
 *
 * @param[in] filename  Filename of the file to load.
 * @param[in] fileSize  Size of the loaded file
 * @param[in] memblock  Dali::Vector containing the buffer loaded
 * @param[in] fileType  How we want to load the file. Binary or Text. Binary default
 * @return error code. 0 - Error, 1 - Ok
 *
 */
DALI_ADAPTOR_API int ReadFile(const std::string& filename, std::streampos& fileSize, Dali::Vector<char> & memblock, FileLoader::FileType fileType = BINARY);

/**
 * @brief Get the file size of a file
 *
 * @param[in] filename  Filename of the file to load.
 * @return the size of the file or 0 if file not found
 */
DALI_ADAPTOR_API std::streampos GetFileSize(const std::string& filename);
};

} // Dali
#endif // DALI_FILE_LOADER_H
