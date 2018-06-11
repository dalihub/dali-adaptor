#ifndef _CUSTOMFILE_INCLUDE_
#define _CUSTOMFILE_INCLUDE_

#include <stdint.h>

namespace CustomFile
{
//void* fopen( const char *name, const char *mode );

int fclose( const void* fp );

void* fmemopen( void* buffer, size_t dataSize, const char * const mode );

int fread( void* buf, int eleSize, int count, const void *fp );

void fwrite( void *buf, int size, const void *fp );

int fseek( const void *fp, int offset, int origin );

int ftell( const void *fp );

bool feof( const void *fp );
}

#endif
