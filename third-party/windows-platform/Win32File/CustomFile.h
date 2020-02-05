#ifndef _CUSTOMFILE_INCLUDE_
#define _CUSTOMFILE_INCLUDE_

#include <stdint.h>
#include <string>

namespace CustomFile
{
FILE* FOpen( const char *name, const char *mode );

int FClose( const void* fp );

FILE* FMemopen( void* buffer, size_t dataSize, const char * mode );

int FRead( void* buf, int eleSize, int count, const void *fp );

void FWrite( void *buf, int size, const void *fp );

unsigned int FWrite( const char *buf, unsigned int eleSize, unsigned int count, void *fp );

int FSeek( const void *fp, int offset, int origin );

int FTell( const void *fp );

bool FEof( const void *fp );
}

#endif
