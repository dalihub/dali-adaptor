#ifndef DALI_FILE_LOADER_IMPL_GENERIC_H
#define DALI_FILE_LOADER_IMPL_GENERIC_H

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
#include <dali/devel-api/adaptor-framework/file-loader.h>

// INTERNAL INCLUDES

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

int ReadFile(const std::string& filename, Dali::Vector<char>& memblock, Dali::FileLoader::FileType fileType = Dali::FileLoader::BINARY);

int ReadFile(const std::string& filename, std::streampos& fileSize, Dali::Vector<char>& memblock, Dali::FileLoader::FileType fileType = Dali::FileLoader::BINARY);

std::streampos GetFileSize(const std::string& filename);

} // Adaptor

} // Internal

} // Dali
#endif // DALI_FILE_LOADER_IMPL_GENERIC_H
