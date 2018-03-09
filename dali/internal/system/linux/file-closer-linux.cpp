#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/internal/system/common/file-closer.h>

#include <string>

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
  return ::fmemopen( __s, __len, __modes );
}

size_t fread( void*  _Buffer, size_t _ElementSize, size_t _ElementCount, FILE*  _Stream )
{
  return ::fread( _Buffer, _ElementSize, _ElementCount, _Stream );
}

int fclose( FILE *__stream )
{
  return ::fclose( __stream );
}

void fwrite( void *buf, int size, int count, FILE *fp )
{
  ::fwrite( buf, size, count, fp );
}

int fseek( FILE *fp, int offset, int origin )
{
  return ::fseek( fp, offset, origin );
}

int ftell( FILE *fp )
{
  return ::ftell( fp );
}

bool feof( FILE *fp )
{
  return ::feof( fp );
}
} //InternalFile
} //Platform
} //Internal
} //Dali
