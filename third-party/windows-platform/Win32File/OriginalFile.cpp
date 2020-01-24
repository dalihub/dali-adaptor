#include "stdio.h"

void* OriginalFOpen( const char *name, const char *mode )
{
  return fopen( name, mode );
}

int OriginalFClose( const void *fp )
{
  return fclose( (FILE*)fp );
}

int OriginalFRead( void* buf, int eleSize, int count, const void *fp )
{
  return fread( buf, eleSize, count, (FILE*)fp );
}

void OriginalFWrite( void *buf, int size, const void *fp )
{
  fwrite( buf, size, 1, (FILE*)fp );
}

void OriginalFWrite( void *buf, int eleSize, int count, const void *fp )
{
  fwrite( buf, eleSize, count, (FILE*)fp );
}

int OriginalFSeek( const void *fp, int offset, int origin )
{
  return fseek( (FILE*)fp, offset, origin );
}

int OriginalFTell( const void *fp )
{
  return ftell( (FILE*)fp );
}

bool OriginalFEof( const void *fp )
{
  return (bool)feof( (FILE*)fp );
}