#include <string>

using namespace std;

typedef unsigned char BYTE;
typedef unsigned int UINT;

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

class CBufferFile
{
public:
  CBufferFile( BYTE* lpBuffer, UINT nBufferSize )
  {
    mFlag = 0xFF;
    mCurIndex = 0;
    mLength = nBufferSize;

    mBuffer = lpBuffer;
  }

  virtual ~CBufferFile()
  {
    Close();
  }

  int Read( void *buf, int size )
  {
    int realReadSize = mLength - mCurIndex;

    if( realReadSize > size )
    {
      realReadSize = size;
    }

    if( 0 < realReadSize )
    {
      memcpy( buf, mBuffer + mCurIndex, realReadSize );
      mCurIndex += realReadSize;
    }

    return realReadSize;
  }

  void Write( void *buf, int size )
  {
    memcpy( mBuffer + mCurIndex, buf, size );
    mCurIndex += size;
  }

  int Seek( long offset, int origin )
  {
    int nextIndex = -1;

    switch( origin )
    {
    case SEEK_SET:
      nextIndex = offset;
      break;

    case SEEK_CUR:
      nextIndex = mCurIndex + offset;
      break;

    case SEEK_END:
      nextIndex = mLength - 1 - offset;
      break;
    }

    if( nextIndex >= mLength )
    {
      nextIndex = -1;
    }

    if( 0 > nextIndex )
    {
      return 1;
    }
    else
    {
      mCurIndex = nextIndex;
      return 0;
    }
  }

  int GetPosition()
  {
    return mCurIndex;
  }

  bool IsEnd()
  {
    return ( mLength - 1 == mCurIndex );
  }

  void Close()
  {
    mBuffer = NULL;

    mCurIndex = 0;
    mLength = -1;
    mFlag = 0;
  }

protected:

private:
  char mFlag;
  BYTE *mBuffer;

  UINT mLength;
  UINT mCurIndex;
};

void* MemFOpen( uint8_t* buffer, size_t dataSize, const char * const mode )
{
  CBufferFile *pBufferFile = new CBufferFile( buffer, dataSize );
  return pBufferFile;
}

void MemFClose( const void *fp )
{
  CBufferFile *file = (CBufferFile*)fp;
  printf("MemFClose file 0x%x\n", file);
  file->Close();
  delete file;
}

int MemFRead( void* buf, int eleSize, int count, const void *fp )
{
  CBufferFile *file = (CBufferFile*)fp;
  return file->Read( buf, eleSize * count );
}

void MemFWrite( void *buf, int size, const void *fp )
{
  CBufferFile *file = (CBufferFile*)fp;
  file->Write( buf, size );
}

int MemFSeek( const void *fp, int offset, int origin )
{
  CBufferFile *file = (CBufferFile*)fp;
  return file->Seek( offset, origin );
}

int MemFTell( const void *fp )
{
  CBufferFile *file = (CBufferFile*)fp;
  return file->GetPosition();
}

bool MemFEof( const void *fp )
{
  CBufferFile *file = (CBufferFile*)fp;
  return file->IsEnd();
}
