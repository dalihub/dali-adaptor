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

// Internal
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/internal/system/common/file-closer.h>

// External
#include <CustomFile.h>
#include <string>

namespace Dali
{

namespace Internal
{

namespace Platform
{

namespace InternalFile
{

std::string GetRealName( const char *name )
{
  if( NULL != name && '*' == name[0] )
  {
    std::string envName;

    const char *p = name + 1;
    
    while( 0 != *p && '*' != *p )
    {
      envName.push_back( *p );
      p++;
    }

    p++;

    char *envValue = std::getenv( envName.c_str() );

    std::string realName = envValue;
    realName += p;

    return realName;
  }
  else
  {
    return "";
  }
}

FILE *FOpen( const char *name, const char *mode )
{
  if( NULL != name && '*' == name[0] )
  {
    std::string realName = GetRealName( name );
    return (FILE*)CustomFile::FOpen( realName.c_str(), mode );
  }
  else
  {
    return (FILE*)CustomFile::FOpen( name, mode );
  }
}

FILE *FMemopen( void *__s, size_t __len, const char *__modes )
{
  FILE *ret = (FILE*)CustomFile::FMemopen( __s, __len, __modes );
  return ret;
}

size_t FRead( void*  _Buffer, size_t _ElementSize, size_t _ElementCount, FILE*  _Stream )
{
  return CustomFile::FRead( _Buffer, _ElementSize, _ElementCount, _Stream );
}

int FClose( FILE *__stream )
{
  return CustomFile::FClose( __stream );
}

void FWrite( void *buf, int size, int count, FILE *fp )
{
  CustomFile::FWrite( buf, size * count, fp );
}

int FSeek( FILE *fp, int offset, int origin )
{
  return CustomFile::FSeek( fp, offset, origin );
}

int FTell( FILE *fp )
{
  return CustomFile::FTell( fp );
}

bool FEof( FILE *fp )
{
  return CustomFile::FEof( fp );
}
} //InternalFile
} //Platform
} //Internal
} //Dali
