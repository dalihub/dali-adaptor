#include "CustomFile.h"

extern void* MemFOpen( uint8_t* buffer, size_t dataSize, const char * const mode );
extern void MemFClose( const void *fp );
extern int MemFRead( void* buf, int eleSize, int count, const void *fp );
extern void MemFWrite( void *buf, int size, const void *fp );
extern int MemFSeek( const void *fp, int offset, int origin );
extern int MemFTell( const void *fp );
extern bool MemFEof( const void *fp );

extern void* OriginalFOpen( const char *name, const char *mode );
extern int OriginalFClose( const void *fp );
extern int OriginalFRead( void* buf, int eleSize, int count, const void *fp );
extern void OriginalFWrite( void *buf, int size, const void *fp );
extern void OriginalFWrite( void *buf, int eleSize, int count, const void *fp );
extern int OriginalFSeek( const void *fp, int offset, int origin );
extern int OriginalFTell( const void *fp );
extern bool OriginalFEof( const void *fp );

namespace
{
std::string GetRealName(const char* name)
{
  if (nullptr != name && '*' == name[0])
  {
    std::string envName;

    const char *p = name + 1;

    while (0 != *p && '*' != *p)
    {
      envName.push_back(*p);
      p++;
    }

    p++;

    char *envValue = std::getenv(envName.c_str());

    std::string realName;
    realName = "";
    realName += envValue;
    realName += p;

    return realName;
  }
  else
  {
    return std::string(name);
  }
}
} // namespace

namespace CustomFile
{
FILE* FOpen( const char *name, const char *mode )
{
  if( NULL != name && '*' == name[0] )
  {
    std::string realName = GetRealName( name );
    FILE* ret = (FILE*)OriginalFOpen( realName.c_str(), mode );
    if (NULL == ret)
    {
      int temp = 0;
    }
    return ret;
  }
  else
  {
    return (FILE*)OriginalFOpen( name, mode );
  }
}

int FClose( const void* fp )
{
  if( -1 == *( (char*)fp + 4 ) )
  {
    MemFClose( fp );
    return 0;
  }
  else
  {
    return OriginalFClose( fp );
  }
}

FILE* FMemopen( void* buffer, size_t dataSize, const char * mode )
{
  return (FILE*)MemFOpen( ( uint8_t*)buffer, dataSize, mode );
}

int FRead( void* buf, int eleSize, int count, const void *fp )
{
  if( -1 == *( (char*)fp + 4 ) )
  {
    return MemFRead( buf, eleSize, count, fp );
  }
  else
  {
    return OriginalFRead( buf, eleSize, count, fp );
  }
}

void FWrite( void *buf, int size, const void *fp )
{
  if( -1 == *( (char*)fp + 4 ) )
  {
    MemFWrite( buf, size, fp );
  }
  else
  {
    OriginalFWrite( buf, size, fp );
  }
}

unsigned int FWrite( const char  *buf, unsigned int eleSize, unsigned int count, void *fp )
{
  if( -1 == *( (char*)fp + 4 ) )
  {
    MemFWrite( (void*)buf, eleSize * count, fp );
  }
  else
  {
    OriginalFWrite((void*)buf, eleSize, count, fp );
  }

  return eleSize * count;
}

int FSeek( const void *fp, int offset, int origin )
{
  if( -1 == *( (char*)fp + 4 ) )
  {
    return MemFSeek( fp, offset, origin );
  }
  else
  {
    return OriginalFSeek( fp, offset, origin );
  }
}

int FTell( const void *fp )
{
  if( -1 == *( (char*)fp + 4 ) )
  {
    return MemFTell( fp );
  }
  else
  {
    return OriginalFTell( fp );
  }
}

bool FEof( const void *fp )
{
  if( -1 == *( (char*)fp + 4 ) )
  {
    return MemFEof( fp );
  }
  else
  {
    return OriginalFEof( fp );
  }
}
}

extern "C"
{
size_t __cdecl fread_for_c( void*  _Buffer, size_t _ElementSize, size_t _ElementCount, void*  _Stream )
{
  return CustomFile::FRead( _Buffer, _ElementSize, _ElementCount, _Stream );
}

void __cdecl fwrite_for_c( void *buf, int size, const void *fp )
{
  CustomFile::FWrite( buf, size, fp );
}
}
