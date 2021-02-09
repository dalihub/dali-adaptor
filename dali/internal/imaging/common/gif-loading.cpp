/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
 * Copyright notice for the EFL:
 * Copyright (C) EFL developers (see AUTHORS)
 */

// CLASS HEADER
#include <dali/internal/imaging/common/gif-loading.h>

// EXTERNAL INCLUDES
#include <fcntl.h>
#include <gif_lib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <memory>

#include <dali/integration-api/debug.h>
#include <dali/internal/imaging/common/file-download.h>
#include <dali/internal/system/common/file-reader.h>
#include <dali/public-api/images/pixel-data.h>

#define IMG_TOO_BIG(w, h)                                                       \
  ((static_cast<unsigned long long>(w) * static_cast<unsigned long long>(h)) >= \
   ((1ULL << (29 * (sizeof(void*) / 4))) - 2048))

#define LOADERR(x)     \
  do                   \
  {                    \
    DALI_LOG_ERROR(x); \
    goto on_error;     \
  } while(0)

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gGifLoadingLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_GIF_LOADING");
#endif

const int        IMG_MAX_SIZE                = 65000;
constexpr size_t MAXIMUM_DOWNLOAD_IMAGE_SIZE = 50 * 1024 * 1024;

#if GIFLIB_MAJOR < 5
const int DISPOSE_BACKGROUND = 2; /* Set area too background color */
const int DISPOSE_PREVIOUS   = 3; /* Restore to previous content */
#endif

struct FrameInfo
{
  FrameInfo()
  : x(0),
    y(0),
    w(0),
    h(0),
    delay(0),
    transparent(-1),
    dispose(DISPOSE_BACKGROUND),
    interlace(0)
  {
  }

  int            x, y, w, h;
  unsigned short delay;            // delay time in 1/100ths of a sec
  short          transparent : 10; // -1 == not, anything else == index
  short          dispose : 6;      // 0, 1, 2, 3 (others invalid)
  short          interlace : 1;    // interlaced or not
};

struct ImageFrame
{
  ImageFrame()
  : index(0),
    data(nullptr),
    info(),
    loaded(false)
  {
  }

  ~ImageFrame()
  {
    if(data != nullptr)
    {
      // De-allocate memory of the frame data.
      delete[] data;
      data = nullptr;
    }
  }

  int       index;
  uint32_t* data; /* frame decoding data */
  FrameInfo info; /* special image type info */
  bool      loaded : 1;
};

struct GifAnimationData
{
  GifAnimationData()
  : frames(),
    frameCount(0),
    loopCount(0),
    currentFrame(0),
    animated(false)
  {
  }

  std::vector<ImageFrame> frames;
  int                     frameCount;
  int                     loopCount;
  int                     currentFrame;
  bool                    animated;
};

// Forward declaration
struct GifAccessor;

struct LoaderInfo
{
  LoaderInfo()
  {
  }

  struct FileData
  {
    FileData()
    : fileName(nullptr),
      globalMap(nullptr),
      length(0),
      isLocalResource(true)
    {
    }

    ~FileData()
    {
      if(globalMap)
      {
        free(globalMap);
        globalMap = nullptr;
      }
    }

    bool LoadFile();

  private:
    bool LoadLocalFile();
    bool LoadRemoteFile();

  public:
    const char*    fileName;        /**< The absolute path of the file. */
    unsigned char* globalMap;       /**< A pointer to the entire contents of the file */
    long long      length;          /**< The length of the file in bytes. */
    bool           isLocalResource; /**< The flag whether the file is a local resource */
  };

  struct FileInfo
  {
    FileInfo()
    : map(nullptr),
      position(0),
      length(0)
    {
    }

    unsigned char* map;
    int            position, length; // yes - gif uses ints for file sizes.
  };

  FileData                     fileData;
  GifAnimationData             animated;
  std::unique_ptr<GifAccessor> gifAccessor{nullptr};
  int                          imageNumber{0};
  FileInfo                     fileInfo;
};

struct ImageProperties
{
  unsigned int w{0};
  unsigned int h{0};
  bool         alpha{0};
};

/**
 * Class to access gif open/close using riaa
 */
struct GifAccessor
{
  /**
   * Constructor
   * @param[in,out] fileInfo Contains ptr to memory, and is updated by DGifOpen
   */
  GifAccessor(LoaderInfo::FileInfo& fileInfo)
  {
// actually ask libgif to open the file
#if GIFLIB_MAJOR >= 5
    gif = DGifOpen(&fileInfo, FileRead, NULL);
#else
    gif = DGifOpen(&fileInfo, FileRead);
#endif

    if(!gif)
    {
      DALI_LOG_ERROR("LOAD_ERROR_UNKNOWN_FORMAT");
    }
  }

  ~GifAccessor()
  {
    if(gif)
    {
#if(GIFLIB_MAJOR > 5) || ((GIFLIB_MAJOR == 5) && (GIFLIB_MINOR >= 1))
      DGifCloseFile(gif, NULL);
#else
      DGifCloseFile(gif);
#endif
    }
  }

  /**
   * @brief Copy data from gif file into buffer.
   *
   * @param[in] gifFileType A pointer pointing to GIF File Type
   * @param[out] buffer A pointer to buffer containing GIF raw data
   * @param[in] len The length in bytes to be copied
   * @return The data length of the image in bytes
   */
  static int FileRead(GifFileType* gifFileType, GifByteType* buffer, int length)
  {
    LoaderInfo::FileInfo* fi = reinterpret_cast<LoaderInfo::FileInfo*>(gifFileType->UserData);

    if(fi->position >= fi->length)
    {
      return 0; // if at or past end - no
    }
    if((fi->position + length) >= fi->length)
    {
      length = fi->length - fi->position;
    }
    memcpy(buffer, fi->map + fi->position, length);
    fi->position += length;
    return length;
  }

  GifFileType* gif = nullptr;
};

bool LoaderInfo::FileData::LoadFile()
{
  bool success = false;
  if(isLocalResource)
  {
    success = LoadLocalFile();
  }
  else
  {
    success = LoadRemoteFile();
  }
  return success;
}

bool LoaderInfo::FileData::LoadLocalFile()
{
  Internal::Platform::FileReader fileReader(fileName);
  FILE*                          fp = fileReader.GetFile();
  if(fp == NULL)
  {
    return false;
  }

  if(fseek(fp, 0, SEEK_END) <= -1)
  {
    return false;
  }

  length = ftell(fp);
  if(length <= -1)
  {
    return false;
  }

  if((!fseek(fp, 0, SEEK_SET)))
  {
    globalMap = reinterpret_cast<GifByteType*>(malloc(sizeof(GifByteType) * length));
    length    = fread(globalMap, sizeof(GifByteType), length, fp);
  }
  else
  {
    return false;
  }
  return true;
}

bool LoaderInfo::FileData::LoadRemoteFile()
{
  // remote file
  bool                  succeeded = false;
  Dali::Vector<uint8_t> dataBuffer;
  size_t                dataSize;

  succeeded = TizenPlatform::Network::DownloadRemoteFileIntoMemory(fileName, dataBuffer, dataSize, MAXIMUM_DOWNLOAD_IMAGE_SIZE);
  if(succeeded)
  {
    size_t blobSize = dataBuffer.Size();
    if(blobSize > 0U)
    {
      // Open a file handle on the memory buffer:
      Dali::Internal::Platform::FileReader fileReader(dataBuffer, blobSize);
      FILE* const                          fp = fileReader.GetFile();
      if(NULL != fp)
      {
        if((!fseek(fp, 0, SEEK_SET)))
        {
          globalMap = reinterpret_cast<GifByteType*>(malloc(sizeof(GifByteType) * blobSize));
          length    = fread(globalMap, sizeof(GifByteType), blobSize, fp);
          succeeded = true;
        }
        else
        {
          DALI_LOG_ERROR("Error seeking within file\n");
        }
      }
      else
      {
        DALI_LOG_ERROR("Error reading file\n");
      }
    }
  }

  return succeeded;
}

/**
 * @brief This combines R, G, B and Alpha values into a single 32-bit (ABGR) value.
 *
 * @param[in] animated A structure containing GIF animation data
 * @param[in] index Frame index to be searched in GIF
 * @return A pointer to the ImageFrame.
 */
inline int CombinePixelABGR(int a, int r, int g, int b)
{
  return (((a) << 24) + ((b) << 16) + ((g) << 8) + (r));
}

inline int PixelLookup(ColorMapObject* colorMap, int index)
{
  return CombinePixelABGR(0xFF, colorMap->Colors[index].Red, colorMap->Colors[index].Green, colorMap->Colors[index].Blue);
}

/**
 * @brief Brute force find frame index - gifs are normally small so ok for now.
 *
 * @param[in] animated A structure containing GIF animation data
 * @param[in] index Frame index to be searched in GIF
 * @return A pointer to the ImageFrame.
 */
ImageFrame* FindFrame(const GifAnimationData& animated, int index)
{
  for(auto&& elem : animated.frames)
  {
    if(elem.index == index)
    {
      return const_cast<ImageFrame*>(&elem);
    }
  }
  return nullptr;
}

/**
 * @brief Fill in an image with a specific rgba color value.
 *
 * @param[in] data A pointer pointing to an image data
 * @param[in] row A int containing the number of rows in an image
 * @param[in] val A uint32_t containing rgba color value
 * @param[in] x X-coordinate used an offset to calculate pixel position
 * @param[in] y Y-coordinate used an offset to calculate pixel position
 * @param[in] width Width of the image
 * @param[in] height Height of the image
 */
void FillImage(uint32_t* data, int row, uint32_t val, int x, int y, int width, int height)
{
  int       xAxis, yAxis;
  uint32_t* pixelPosition;

  for(yAxis = 0; yAxis < height; yAxis++)
  {
    pixelPosition = data + ((y + yAxis) * row) + x;
    for(xAxis = 0; xAxis < width; xAxis++)
    {
      *pixelPosition = val;
      pixelPosition++;
    }
  }
}

/**
 * @brief Fill a rgba data pixle blob with a frame color (bg or trans)
 *
 * @param[in] data A pointer pointing to an image data
 * @param[in] row A int containing the number of rows in an image
 * @param[in] gif A pointer pointing to GIF File Type
 * @param[in] frameInfo A pointer pointing to Frame Information data
 * @param[in] x X-coordinate used an offset to calculate pixel position
 * @param[in] y Y-coordinate used an offset to calculate pixel position
 * @param[in] width Width of the image
 * @param[in] height Height of the image
 */
void FillFrame(uint32_t* data, int row, GifFileType* gif, FrameInfo* frameInfo, int x, int y, int w, int h)
{
  // solid color fill for pre frame region
  if(frameInfo->transparent < 0)
  {
    ColorMapObject* colorMap;
    int             backGroundColor;

    // work out color to use from colorMap
    if(gif->Image.ColorMap)
    {
      colorMap = gif->Image.ColorMap;
    }
    else
    {
      colorMap = gif->SColorMap;
    }
    backGroundColor = gif->SBackGroundColor;
    // and do the fill
    FillImage(data, row, CombinePixelABGR(0xff, colorMap->Colors[backGroundColor].Red, colorMap->Colors[backGroundColor].Green, colorMap->Colors[backGroundColor].Blue), x, y, w, h);
  }
  // fill in region with 0 (transparent)
  else
  {
    FillImage(data, row, 0, x, y, w, h);
  }
}

/**
 * @brief Store common fields from gif file info into frame info
 *
 * @param[in] gif A pointer pointing to GIF File Type
 * @param[in] frameInfo A pointer pointing to Frame Information data
 */
void StoreFrameInfo(GifFileType* gif, FrameInfo* frameInfo)
{
  frameInfo->x         = gif->Image.Left;
  frameInfo->y         = gif->Image.Top;
  frameInfo->w         = gif->Image.Width;
  frameInfo->h         = gif->Image.Height;
  frameInfo->interlace = gif->Image.Interlace;
}

/**
 * @brief Check if image fills "screen space" and if so, if it is transparent
 * at all then the image could be transparent - OR if image doesnt fill,
 * then it could be trasnparent (full coverage of screen). Some gifs will
 * be recognized as solid here for faster rendering, but not all.
 *
 * @param[out] full A boolean to show whether image is transparent or not
 * @param[in] frameInfo A pointer pointing to Frame Information data
 * @param[in] width Width of the image
 * @param[in] height Height of the image
 */
void CheckTransparency(bool& full, FrameInfo* frameInfo, int width, int height)
{
  if((frameInfo->x == 0) && (frameInfo->y == 0) &&
     (frameInfo->w == width) && (frameInfo->h == height))
  {
    if(frameInfo->transparent >= 0)
    {
      full = false;
    }
  }
  else
  {
    full = false;
  }
}

/**
 * @brief Fix coords and work out an x and y inset in orig data if out of image bounds.
 */
void ClipCoordinates(int imageWidth, int imageHeight, int* xin, int* yin, int x0, int y0, int w0, int h0, int* x, int* y, int* w, int* h)
{
  if(x0 < 0)
  {
    w0 += x0;
    *xin = -x0;
    x0   = 0;
  }
  if((x0 + w0) > imageWidth)
  {
    w0 = imageWidth - x0;
  }
  if(y0 < 0)
  {
    h0 += y0;
    *yin = -y0;
    y0   = 0;
  }
  if((y0 + h0) > imageHeight)
  {
    h0 = imageHeight - y0;
  }
  *x = x0;
  *y = y0;
  *w = w0;
  *h = h0;
}

/**
 * @brief Flush out rgba frame images to save memory but skip current,
 * previous and lastPreservedFrame frames (needed for dispose mode DISPOSE_PREVIOUS)
 *
 * @param[in] animated A structure containing GIF animation data
 * @param[in] width Width of the image
 * @param[in] height Height of the image
 * @param[in] thisframe The current frame
 * @param[in] prevframe The previous frame
 * @param[in] lastPreservedFrame The last preserved frame
 */
void FlushFrames(GifAnimationData& animated, int width, int height, ImageFrame* thisframe, ImageFrame* prevframe, ImageFrame* lastPreservedFrame)
{
  DALI_LOG_INFO(gGifLoadingLogFilter, Debug::Concise, "FlushFrames() START \n");

  // target is the amount of memory we want to be under for stored frames
  int total = 0, target = 512 * 1024;

  // total up the amount of memory used by stored frames for this image
  for(auto&& frame : animated.frames)
  {
    if(frame.data)
    {
      total++;
    }
  }
  total *= (width * height * sizeof(uint32_t));

  DALI_LOG_INFO(gGifLoadingLogFilter, Debug::Concise, "Total used frame size: %d\n", total);

  // If we use more than target (512k) for frames - flush
  if(total > target)
  {
    // Clean frames (except current and previous) until below target
    for(auto&& frame : animated.frames)
    {
      if((frame.index != thisframe->index) && (!prevframe || frame.index != prevframe->index) &&
         (!lastPreservedFrame || frame.index != lastPreservedFrame->index))
      {
        if(frame.data != nullptr)
        {
          delete[] frame.data;
          frame.data = nullptr;

          // subtract memory used and if below target - stop flush
          total -= (width * height * sizeof(uint32_t));
          if(total < target)
          {
            break;
          }
        }
      }
    }
  }

  DALI_LOG_INFO(gGifLoadingLogFilter, Debug::Concise, "FlushFrames() END \n");
}

/**
 * @brief allocate frame and frame info and append to list and store fields.
 *
 * @param[in] animated A structure containing GIF animation data
 * @param[in] transparent Transparent index of the new frame
 * @param[in] dispose Dispose mode of new frame
 * @param[in] delay The frame delay of new frame
 * @param[in] index The index of new frame
 */
FrameInfo* NewFrame(GifAnimationData& animated, int transparent, int dispose, int delay, int index)
{
  ImageFrame frame;

  // record transparent index to be used or -1 if none
  // for this SPECIFIC frame
  frame.info.transparent = transparent;
  // record dispose mode (3 bits)
  frame.info.dispose = dispose;
  // record delay (2 bytes so max 65546 /100 sec)
  frame.info.delay = delay;
  // record the index number we are at
  frame.index = index;
  // that frame is stored AT image/screen size

  animated.frames.push_back(frame);

  DALI_LOG_INFO(gGifLoadingLogFilter, Debug::Concise, "NewFrame: animated.frames.size() = %d\n", animated.frames.size());

  return &(animated.frames.back().info);
}

/**
 * @brief Decode a gif image into rows then expand to 32bit into the destination
 * data pointer.
 */
bool DecodeImage(GifFileType* gif, uint32_t* data, int rowpix, int xin, int yin, int transparent, int x, int y, int w, int h, bool fill)
{
  int             intoffset[] = {0, 4, 2, 1};
  int             intjump[]   = {8, 8, 4, 2};
  int             i, xx, yy, pix, gifW, gifH;
  GifRowType*     rows = NULL;
  bool            ret  = false;
  ColorMapObject* colorMap;
  uint32_t*       p;

  // what we need is image size.
  SavedImage* sp;
  sp = &gif->SavedImages[gif->ImageCount - 1];

  gifW = sp->ImageDesc.Width;
  gifH = sp->ImageDesc.Height;

  if((gifW < w) || (gifH < h))
  {
    DALI_ASSERT_DEBUG(false && "Dimensions are bigger than the Gif image size");
    goto on_error;
  }

  // build a blob of memory to have pointers to rows of pixels
  // AND store the decoded gif pixels (1 byte per pixel) as welll
  rows = static_cast<GifRowType*>(malloc((gifH * sizeof(GifRowType)) + (gifW * gifH * sizeof(GifPixelType))));
  if(!rows)
  {
    goto on_error;
  }

  // fill in the pointers at the start
  for(yy = 0; yy < gifH; yy++)
  {
    rows[yy] = reinterpret_cast<unsigned char*>(rows) + (gifH * sizeof(GifRowType)) + (yy * gifW * sizeof(GifPixelType));
  }

  // if gif is interlaced, walk interlace pattern and decode into rows
  if(gif->Image.Interlace)
  {
    for(i = 0; i < 4; i++)
    {
      for(yy = intoffset[i]; yy < gifH; yy += intjump[i])
      {
        if(DGifGetLine(gif, rows[yy], gifW) != GIF_OK)
        {
          goto on_error;
        }
      }
    }
  }
  // normal top to bottom - decode into rows
  else
  {
    for(yy = 0; yy < gifH; yy++)
    {
      if(DGifGetLine(gif, rows[yy], gifW) != GIF_OK)
      {
        goto on_error;
      }
    }
  }

  // work out what colormap to use
  if(gif->Image.ColorMap)
  {
    colorMap = gif->Image.ColorMap;
  }
  else
  {
    colorMap = gif->SColorMap;
  }

  // if we need to deal with transparent pixels at all...
  if(transparent >= 0)
  {
    // if we are told to FILL (overwrite with transparency kept)
    if(fill)
    {
      for(yy = 0; yy < h; yy++)
      {
        p = data + ((y + yy) * rowpix) + x;
        for(xx = 0; xx < w; xx++)
        {
          pix = rows[yin + yy][xin + xx];
          if(pix != transparent)
          {
            *p = PixelLookup(colorMap, pix);
          }
          else
          {
            *p = 0;
          }
          p++;
        }
      }
    }
    // paste on top with transparent pixels untouched
    else
    {
      for(yy = 0; yy < h; yy++)
      {
        p = data + ((y + yy) * rowpix) + x;
        for(xx = 0; xx < w; xx++)
        {
          pix = rows[yin + yy][xin + xx];
          if(pix != transparent)
          {
            *p = PixelLookup(colorMap, pix);
          }
          p++;
        }
      }
    }
  }
  else
  {
    // walk pixels without worring about transparency at all
    for(yy = 0; yy < h; yy++)
    {
      p = data + ((y + yy) * rowpix) + x;
      for(xx = 0; xx < w; xx++)
      {
        pix = rows[yin + yy][xin + xx];
        *p  = PixelLookup(colorMap, pix);
        p++;
      }
    }
  }
  ret = true;

on_error:
  if(rows)
  {
    free(rows);
  }
  return ret;
}

/**
 * @brief Reader header from the gif file and populates structures accordingly.
 *
 * @param[in] loaderInfo A LoaderInfo structure containing file descriptor and other data about GIF.
 * @param[out] prop A ImageProperties structure for storing information about GIF data.
 * @param[out] error Error code
 * @return The true or false whether reading was successful or not.
 */
bool ReadHeader(LoaderInfo&      loaderInfo,
                ImageProperties& prop, //output struct
                int*             error)
{
  GifAnimationData&     animated = loaderInfo.animated;
  LoaderInfo::FileData& fileData = loaderInfo.fileData;
  bool                  success  = false;
  LoaderInfo::FileInfo  fileInfo;
  GifRecordType         rec;

  // it is possible which gif file have error midle of frames,
  // in that case we should play gif file until meet error frame.
  int        imageNumber = 0;
  int        loopCount   = -1;
  FrameInfo* frameInfo   = NULL;
  bool       full        = true;

  success = fileData.LoadFile();
  if(!success || !fileData.globalMap)
  {
    success = false;
    DALI_LOG_ERROR("LOAD_ERROR_CORRUPT_FILE");
  }
  else
  {
    fileInfo.map      = fileData.globalMap;
    fileInfo.length   = fileData.length;
    fileInfo.position = 0;
    GifAccessor gifAccessor(fileInfo);

    if(gifAccessor.gif)
    {
      // get the gif "screen size" (the actual image size)
      prop.w = gifAccessor.gif->SWidth;
      prop.h = gifAccessor.gif->SHeight;

      // if size is invalid - abort here
      if((prop.w < 1) || (prop.h < 1) || (prop.w > IMG_MAX_SIZE) || (prop.h > IMG_MAX_SIZE) || IMG_TOO_BIG(prop.w, prop.h))
      {
        if(IMG_TOO_BIG(prop.w, prop.h))
        {
          success = false;
          DALI_LOG_ERROR("LOAD_ERROR_RESOURCE_ALLOCATION_FAILED");
        }
        else
        {
          success = false;
          DALI_LOG_ERROR("LOAD_ERROR_GENERIC");
        }
      }
      else
      {
        // walk through gif records in file to figure out info
        success = true;
        do
        {
          if(DGifGetRecordType(gifAccessor.gif, &rec) == GIF_ERROR)
          {
            // if we have a gif that ends part way through a sequence
            // (or animation) consider it valid and just break - no error
            if(imageNumber <= 1)
            {
              success = true;
            }
            else
            {
              success = false;
              DALI_LOG_ERROR("LOAD_ERROR_UNKNOWN_FORMAT");
            }
            break;
          }

          // get image description section
          if(rec == IMAGE_DESC_RECORD_TYPE)
          {
            int          img_code;
            GifByteType* img;

            // get image desc
            if(DGifGetImageDesc(gifAccessor.gif) == GIF_ERROR)
            {
              success = false;
              DALI_LOG_ERROR("LOAD_ERROR_UNKNOWN_FORMAT");
              break;
            }
            // skip decoding and just walk image to next
            if(DGifGetCode(gifAccessor.gif, &img_code, &img) == GIF_ERROR)
            {
              success = false;
              DALI_LOG_ERROR("LOAD_ERROR_UNKNOWN_FORMAT");
              break;
            }
            // skip till next...
            while(img)
            {
              img = NULL;
              DGifGetCodeNext(gifAccessor.gif, &img);
            }
            // store geometry in the last frame info data
            if(frameInfo)
            {
              StoreFrameInfo(gifAccessor.gif, frameInfo);
              CheckTransparency(full, frameInfo, prop.w, prop.h);
            }
            // or if we dont have a frameInfo entry - create one even for stills
            else
            {
              // allocate and save frame with field data
              frameInfo = NewFrame(animated, -1, 0, 0, imageNumber + 1);
              if(!frameInfo)
              {
                success = false;
                DALI_LOG_ERROR("LOAD_ERROR_RESOURCE_ALLOCATION_FAILED");
                break;
              }
              // store geometry info from gif image
              StoreFrameInfo(gifAccessor.gif, frameInfo);
              // check for transparency/alpha
              CheckTransparency(full, frameInfo, prop.w, prop.h);
            }
            imageNumber++;
          }
          // we have an extension code block - for animated gifs for sure
          else if(rec == EXTENSION_RECORD_TYPE)
          {
            int          ext_code;
            GifByteType* ext = NULL;

            // get the first extension entry
            DGifGetExtension(gifAccessor.gif, &ext_code, &ext);
            while(ext)
            {
              // graphic control extension - for animated gif data
              // and transparent index + flag
              if(ext_code == 0xf9)
              {
                // create frame and store it in image
                int transparencyIndex = (ext[1] & 1) ? ext[4] : -1;
                int disposeMode       = (ext[1] >> 2) & 0x7;
                int delay             = (int(ext[3]) << 8) | int(ext[2]);
                frameInfo             = NewFrame(animated, transparencyIndex, disposeMode, delay, imageNumber + 1);
                if(!frameInfo)
                {
                  success = false;
                  DALI_LOG_ERROR("LOAD_ERROR_RESOURCE_ALLOCATION_FAILED");
                  break;
                }
              }
              // netscape extension indicating loop count.
              else if(ext_code == 0xff) /* application extension */
              {
                if(!strncmp(reinterpret_cast<char*>(&ext[1]), "NETSCAPE2.0", 11) ||
                   !strncmp(reinterpret_cast<char*>(&ext[1]), "ANIMEXTS1.0", 11))
                {
                  ext = NULL;
                  DGifGetExtensionNext(gifAccessor.gif, &ext);
                  if(ext[1] == 0x01)
                  {
                    loopCount = (int(ext[3]) << 8) | int(ext[2]);
                    if(loopCount > 0)
                    {
                      loopCount++;
                    }
                  }
                }
              }

              // and continue onto the next extension entry
              ext = NULL;
              DGifGetExtensionNext(gifAccessor.gif, &ext);
            }
          }
        } while(rec != TERMINATE_RECORD_TYPE && success);

        if(success)
        {
          // if the gif main says we have more than one image or our image counting
          // says so, then this image is animated - indicate this
          if((gifAccessor.gif->ImageCount > 1) || (imageNumber > 1))
          {
            animated.animated  = 1;
            animated.loopCount = loopCount;
          }
          animated.frameCount = std::min(gifAccessor.gif->ImageCount, imageNumber);

          if(!full)
          {
            prop.alpha = 1;
          }

          animated.currentFrame = 1;

          // no errors in header scan etc. so set err and return value
          *error = 0;
        }
      }
    }
  }
  return success;
}

/**
 * @brief Reader next frame of the gif file and populates structures accordingly.
 *
 * @param[in,out] loaderInfo A LoaderInfo structure containing file descriptor and other data about GIF.
 * @param[in,out] prop A ImageProperties structure containing information about gif data.
 * @param[out] pixels A pointer to buffer which will contain all pixel data of the frame on return.
 * @param[out] error Error code
 * @return The true or false whether reading was successful or not.
 */
bool ReadNextFrame(LoaderInfo& loaderInfo, ImageProperties& prop, //  use for w and h
                   unsigned char* pixels,
                   int*           error)
{
  GifAnimationData&     animated = loaderInfo.animated;
  LoaderInfo::FileData& fileData = loaderInfo.fileData;
  bool                  ret      = false;
  GifRecordType         rec;
  int                   index = 0, imageNumber = 0;
  FrameInfo*            frameInfo;
  ImageFrame*           frame              = NULL;
  ImageFrame*           lastPreservedFrame = NULL;

  index = animated.currentFrame;

  // if index is invalid for animated image - error out
  if((animated.animated) && ((index <= 0) || (index > animated.frameCount)))
  {
    DALI_LOG_ERROR("LOAD_ERROR_GENERIC");
    return false;
  }

  // find the given frame index
  frame = FindFrame(animated, index);
  if(!frame)
  {
    DALI_LOG_ERROR("LOAD_ERROR_CORRUPT_FILE");
    return false;
  }
  else if(!(frame->loaded) || !(frame->data))
  {
    // if we want to go backwards, we likely need/want to re-decode from the
    // start as we have nothing to build on. If there is a gif, imageNumber
    // has been set already.
    if(loaderInfo.gifAccessor && loaderInfo.imageNumber > 0)
    {
      if((index > 0) && (index < loaderInfo.imageNumber) && (animated.animated))
      {
        loaderInfo.gifAccessor.reset();
        loaderInfo.imageNumber = 0;
      }
    }

    // actually ask libgif to open the file
    if(!loaderInfo.gifAccessor)
    {
      loaderInfo.fileInfo.map      = fileData.globalMap;
      loaderInfo.fileInfo.length   = fileData.length;
      loaderInfo.fileInfo.position = 0;
      if(!loaderInfo.fileInfo.map)
      {
        DALI_LOG_ERROR("LOAD_ERROR_CORRUPT_FILE");
        return false;
      }
      std::unique_ptr<GifAccessor> gifAccessor = std::make_unique<GifAccessor>(loaderInfo.fileInfo);
      if(!gifAccessor->gif)
      {
        DALI_LOG_ERROR("LOAD_ERROR_UNKNOWN_FORMAT");
        return false;
      }
      loaderInfo.gifAccessor = std::move(gifAccessor);
      loaderInfo.imageNumber = 1;
    }

    // our current position is the previous frame we decoded from the file
    imageNumber = loaderInfo.imageNumber;

    // walk through gif records in file to figure out info
    do
    {
      if(DGifGetRecordType(loaderInfo.gifAccessor->gif, &rec) == GIF_ERROR)
      {
        DALI_LOG_ERROR("LOAD_ERROR_UNKNOWN_FORMAT");
        return false;
      }

      if(rec == EXTENSION_RECORD_TYPE)
      {
        int          ext_code;
        GifByteType* ext = NULL;
        DGifGetExtension(loaderInfo.gifAccessor->gif, &ext_code, &ext);

        while(ext)
        {
          ext = NULL;
          DGifGetExtensionNext(loaderInfo.gifAccessor->gif, &ext);
        }
      }
      // get image description section
      else if(rec == IMAGE_DESC_RECORD_TYPE)
      {
        int          xin = 0, yin = 0, x = 0, y = 0, w = 0, h = 0;
        int          img_code;
        GifByteType* img;
        ImageFrame*  previousFrame = NULL;
        ImageFrame*  thisFrame     = NULL;

        // get image desc
        if(DGifGetImageDesc(loaderInfo.gifAccessor->gif) == GIF_ERROR)
        {
          DALI_LOG_ERROR("LOAD_ERROR_UNKNOWN_FORMAT");
          return false;
        }

        // get the previous frame entry AND the current one to fill in
        previousFrame = FindFrame(animated, imageNumber - 1);
        thisFrame     = FindFrame(animated, imageNumber);

        // if we have a frame AND we're animated AND we have no data...
        if((thisFrame) && (!thisFrame->data) && (animated.animated))
        {
          bool first = false;

          // allocate it
          thisFrame->data = new uint32_t[prop.w * prop.h];

          if(!thisFrame->data)
          {
            DALI_LOG_ERROR("LOAD_ERROR_RESOURCE_ALLOCATION_FAILED");
            return false;
          }

          // if we have no prior frame OR prior frame data... empty
          if((!previousFrame) || (!previousFrame->data))
          {
            first     = true;
            frameInfo = &(thisFrame->info);
            memset(thisFrame->data, 0, prop.w * prop.h * sizeof(uint32_t));
          }
          // we have a prior frame to copy data from...
          else
          {
            frameInfo = &(previousFrame->info);

            // fix coords of sub image in case it goes out...
            ClipCoordinates(prop.w, prop.h, &xin, &yin, frameInfo->x, frameInfo->y, frameInfo->w, frameInfo->h, &x, &y, &w, &h);

            // if dispose mode is not restore - then copy pre frame
            if(frameInfo->dispose != DISPOSE_PREVIOUS)
            {
              memcpy(thisFrame->data, previousFrame->data, prop.w * prop.h * sizeof(uint32_t));
            }

            // if dispose mode is "background" then fill with bg
            if(frameInfo->dispose == DISPOSE_BACKGROUND)
            {
              FillFrame(thisFrame->data, prop.w, loaderInfo.gifAccessor->gif, frameInfo, x, y, w, h);
            }
            else if(frameInfo->dispose == DISPOSE_PREVIOUS) // GIF_DISPOSE_RESTORE
            {
              int prevIndex = 2;
              do
              {
                // Find last preserved frame.
                lastPreservedFrame = FindFrame(animated, imageNumber - prevIndex);
                if(!lastPreservedFrame)
                {
                  DALI_LOG_ERROR("LOAD_ERROR_LAST_PRESERVED_FRAME_NOT_FOUND");
                  return false;
                }
                prevIndex++;
              } while(lastPreservedFrame && lastPreservedFrame->info.dispose == DISPOSE_PREVIOUS);

              if(lastPreservedFrame)
              {
                memcpy(thisFrame->data, lastPreservedFrame->data, prop.w * prop.h * sizeof(uint32_t));
              }
            }
          }
          // now draw this frame on top
          frameInfo = &(thisFrame->info);
          ClipCoordinates(prop.w, prop.h, &xin, &yin, frameInfo->x, frameInfo->y, frameInfo->w, frameInfo->h, &x, &y, &w, &h);
          if(!DecodeImage(loaderInfo.gifAccessor->gif, thisFrame->data, prop.w, xin, yin, frameInfo->transparent, x, y, w, h, first))
          {
            DALI_LOG_ERROR("LOAD_ERROR_CORRUPT_FILE");
            return false;
          }

          // mark as loaded and done
          thisFrame->loaded = true;

          FlushFrames(animated, prop.w, prop.h, thisFrame, previousFrame, lastPreservedFrame);
        }
        // if we have a frame BUT the image is not animated. different
        // path
        else if((thisFrame) && (!thisFrame->data) && (!animated.animated))
        {
          // if we don't have the data decoded yet - decode it
          if((!thisFrame->loaded) || (!thisFrame->data))
          {
            // use frame info but we WONT allocate frame pixels
            frameInfo = &(thisFrame->info);
            ClipCoordinates(prop.w, prop.h, &xin, &yin, frameInfo->x, frameInfo->y, frameInfo->w, frameInfo->h, &x, &y, &w, &h);

            // clear out all pixels
            FillFrame(reinterpret_cast<uint32_t*>(pixels), prop.w, loaderInfo.gifAccessor->gif, frameInfo, 0, 0, prop.w, prop.h);

            // and decode the gif with overwriting
            if(!DecodeImage(loaderInfo.gifAccessor->gif, reinterpret_cast<uint32_t*>(pixels), prop.w, xin, yin, frameInfo->transparent, x, y, w, h, true))
            {
              DALI_LOG_ERROR("LOAD_ERROR_CORRUPT_FILE");
              return false;
            }

            // mark as loaded and done
            thisFrame->loaded = true;
          }
          // flush mem we don't need (at expense of decode cpu)
        }
        else
        {
          // skip decoding and just walk image to next
          if(DGifGetCode(loaderInfo.gifAccessor->gif, &img_code, &img) == GIF_ERROR)
          {
            DALI_LOG_ERROR("LOAD_ERROR_UNKNOWN_FORMAT");
            return false;
          }

          while(img)
          {
            img = NULL;
            DGifGetCodeNext(loaderInfo.gifAccessor->gif, &img);
          }
        }

        imageNumber++;
        // if we found the image we wanted - get out of here
        if(imageNumber > index)
        {
          break;
        }
      }
    } while(rec != TERMINATE_RECORD_TYPE);

    // if we are at the end of the animation or not animated, close file
    loaderInfo.imageNumber = imageNumber;
    if((animated.frameCount <= 1) || (rec == TERMINATE_RECORD_TYPE))
    {
      loaderInfo.gifAccessor.reset();
      loaderInfo.imageNumber = 0;
    }
  }

  // no errors in header scan etc. so set err and return value
  *error = 0;
  ret    = true;

  // if it was an animated image we need to copy the data to the
  // pixels for the image from the frame holding the data
  if(animated.animated && frame->data)
  {
    memcpy(pixels, frame->data, prop.w * prop.h * sizeof(uint32_t));
  }

  return ret;
}

} // unnamed namespace

struct GifLoading::Impl
{
public:
  Impl(const std::string& url, bool isLocalResource)
  : mUrl(url)
  {
    loaderInfo.gifAccessor = nullptr;
    int error;
    loaderInfo.fileData.fileName        = mUrl.c_str();
    loaderInfo.fileData.isLocalResource = isLocalResource;

    ReadHeader(loaderInfo, imageProperties, &error);
  }

  // Moveable but not copyable
  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;
  Impl(Impl&&)                 = default;
  Impl& operator=(Impl&&) = default;

  std::string     mUrl;
  LoaderInfo      loaderInfo;
  ImageProperties imageProperties;
};

AnimatedImageLoadingPtr GifLoading::New(const std::string& url, bool isLocalResource)
{
  return AnimatedImageLoadingPtr(new GifLoading(url, isLocalResource));
}

GifLoading::GifLoading(const std::string& url, bool isLocalResource)
: mImpl(new GifLoading::Impl(url, isLocalResource))
{
}

GifLoading::~GifLoading()
{
  delete mImpl;
}

bool GifLoading::LoadNextNFrames(uint32_t frameStartIndex, int count, std::vector<Dali::PixelData>& pixelData)
{
  int  error;
  bool ret = false;

  const int bufferSize = mImpl->imageProperties.w * mImpl->imageProperties.h * sizeof(uint32_t);

  DALI_LOG_INFO(gGifLoadingLogFilter, Debug::Concise, "LoadNextNFrames( frameStartIndex:%d, count:%d )\n", frameStartIndex, count);

  for(int i = 0; i < count; ++i)
  {
    auto pixelBuffer = new unsigned char[bufferSize];

    mImpl->loaderInfo.animated.currentFrame = 1 + ((frameStartIndex + i) % mImpl->loaderInfo.animated.frameCount);

    if(ReadNextFrame(mImpl->loaderInfo, mImpl->imageProperties, pixelBuffer, &error))
    {
      if(pixelBuffer)
      {
        pixelData.push_back(Dali::PixelData::New(pixelBuffer, bufferSize, mImpl->imageProperties.w, mImpl->imageProperties.h, Dali::Pixel::RGBA8888, Dali::PixelData::DELETE_ARRAY));
        ret = true;
      }
    }
  }

  return ret;
}

Dali::Devel::PixelBuffer GifLoading::LoadFrame(uint32_t frameIndex)
{
  int                      error;
  Dali::Devel::PixelBuffer pixelBuffer;

  DALI_LOG_INFO(gGifLoadingLogFilter, Debug::Concise, "LoadFrame( frameIndex:%d )\n", frameIndex);

  pixelBuffer = Dali::Devel::PixelBuffer::New(mImpl->imageProperties.w, mImpl->imageProperties.h, Dali::Pixel::RGBA8888);

  mImpl->loaderInfo.animated.currentFrame = 1 + (frameIndex % mImpl->loaderInfo.animated.frameCount);
  ReadNextFrame(mImpl->loaderInfo, mImpl->imageProperties, pixelBuffer.GetBuffer(), &error);

  if(error != 0)
  {
    pixelBuffer = Dali::Devel::PixelBuffer();
  }
  return pixelBuffer;
}

ImageDimensions GifLoading::GetImageSize() const
{
  return ImageDimensions(mImpl->imageProperties.w, mImpl->imageProperties.h);
}

uint32_t GifLoading::GetImageCount() const
{
  return mImpl->loaderInfo.animated.frameCount;
}

uint32_t GifLoading::GetFrameInterval(uint32_t frameIndex) const
{
  return mImpl->loaderInfo.animated.frames[frameIndex].info.delay * 10;
}

std::string GifLoading::GetUrl() const
{
  return mImpl->mUrl;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
