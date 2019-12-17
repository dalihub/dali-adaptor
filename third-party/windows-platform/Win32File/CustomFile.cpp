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

namespace std
{
int GetRealFileMode(const char *path, int _Mode)
{
  std::string strPath = path;

  if ((std::ios::in | std::ios::ate == _Mode) && strPath.find(".json") != std::string::npos)
  {
    return std::ios::in | std::ios::binary | std::ios::ate;
  }
  else
  {
    return _Mode;
  }
}

extern const char* GetRealName(const char *name);
}

namespace CustomFile
{
FILE* FOpen( const char *name, const char *mode )
{
  if( NULL != name && '*' == name[0] )
  {
    std::string realName = std::GetRealName( name );
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
