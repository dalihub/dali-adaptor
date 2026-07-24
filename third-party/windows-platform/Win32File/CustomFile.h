#ifndef DALI_ADAPTOR_WINDOWS_CUSTOM_FILE_H
#define DALI_ADAPTOR_WINDOWS_CUSTOM_FILE_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <cstddef>
#include <cstdio>

namespace CustomFile
{
FILE* FOpen(const char* name, const char* mode);

int FClose(const void* fp);

FILE* FMemopen(void* buffer, size_t dataSize, const char* mode);

size_t FRead(void* buffer, size_t elementSize, size_t elementCount, const void* fp);

void FWrite(void* buffer, size_t size, const void* fp);

size_t FWrite(const void* buffer, size_t elementSize, size_t elementCount, void* fp);

int FSeek(const void* fp, long offset, int origin);

long FTell(const void* fp);

int FEof(const void* fp);
} // namespace CustomFile

#define fmemopen CustomFile::FMemopen

#endif // DALI_ADAPTOR_WINDOWS_CUSTOM_FILE_H
