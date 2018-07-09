/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/internal/system/common/file-closer.h>

#include <CustomFile.h>

namespace Dali
{
namespace Internal
{
namespace Platform
{
namespace InternalFile
{
FILE *fmemopen( void *__s, size_t __len, const char *__modes )
{
  FILE *ret = (FILE*)CustomFile::fmemopen( __s, __len, __modes );
  return ret;
}

size_t fread( void*  _Buffer, size_t _ElementSize, size_t _ElementCount, FILE*  _Stream )
{
  return CustomFile::fread( _Buffer, _ElementSize, _ElementCount, _Stream );
}

int fclose( FILE *__stream )
{
  return CustomFile::fclose( __stream );
}

void fwrite( void *buf, int size, int count, FILE *fp )
{
  CustomFile::fwrite( buf, size * count, fp );
}

int fseek( FILE *fp, int offset, int origin )
{
  return CustomFile::fseek( fp, offset, origin );
}

int ftell( FILE *fp )
{
  return CustomFile::ftell( fp );
}

bool feof( FILE *fp )
{
  return CustomFile::feof( fp );
}
} //InternalFile
} //Platform
} //Internal
} //Dali
