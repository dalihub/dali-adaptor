/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/imaging/common/loader-jpeg.h>

// EXTERNAL HEADERS
#include <jpeglib.h>
#include <libexif/exif-data.h>
#include <libexif/exif-loader.h>
#include <libexif/exif-tag.h>
#include <setjmp.h>
#include <turbojpeg.h>
#include <array>
#include <cstring>
#include <functional>
#include <memory>
#include <utility>

#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/public-api/object/property-array.h>
#include <dali/public-api/object/property-map.h>

// INTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>
#include <dali/internal/legacy/tizen/platform-capabilities.h>
#include <dali/internal/system/common/system-error-print.h>

namespace
{
using Dali::Vector;
namespace Pixel                     = Dali::Pixel;
using PixelArray                    = uint8_t*;
const unsigned int DECODED_L8       = 1;
const unsigned int DECODED_RGB888   = 3;
const unsigned int DECODED_RGBA8888 = 4;

const char* CHROMINANCE_SUBSAMPLING_OPTIONS_ENV[] = {"DALI_ENABLE_DECODE_JPEG_TO_YUV_444",
                                                     "DALI_ENABLE_DECODE_JPEG_TO_YUV_422",
                                                     "DALI_ENABLE_DECODE_JPEG_TO_YUV_420",
                                                     "",
                                                     "DALI_ENABLE_DECODE_JPEG_TO_YUV_440",
                                                     "DALI_ENABLE_DECODE_JPEG_TO_YUV_411",
                                                     "DALI_ENABLE_DECODE_JPEG_TO_YUV_441"};

const int CHROMINANCE_SUBSAMPLING_OPTIONS_ENV_COUNT = sizeof(CHROMINANCE_SUBSAMPLING_OPTIONS_ENV) / sizeof(CHROMINANCE_SUBSAMPLING_OPTIONS_ENV[0]);

static bool gSubsamplingFormatTable[TJ_NUMSAMP] = {
  false,
};
static bool gIsSubsamplingFormatTableInitialized = false;

/** Transformations that can be applied to decoded pixels to respect exif orientation
  *  codes in image headers */
enum class JpegTransform
{
  NONE,            //< no transformation 0th-Row = top & 0th-Column = left
  FLIP_HORIZONTAL, //< horizontal flip 0th-Row = top & 0th-Column = right
  ROTATE_180,      //< 180-degree rotation   0th-Row = bottom & 0th-Column = right
  FLIP_VERTICAL,   //< vertical flip  0th-Row = bottom & 0th-Column = left
  TRANSPOSE,       //< transpose across UL-to-LR axis  0th-Row = left   & 0th-Column = top
  ROTATE_90,       //< 90-degree clockwise rotation  0th-Row = right  & 0th-Column = top
  TRANSVERSE,      //< transpose across UR-to-LL axis  0th-Row = right  & 0th-Column = bottom
  ROTATE_270,      //< 270-degree clockwise (or 90 ccw) 0th-Row = left  & 0th-Column = bottom
};

/**
  * @brief Error handling bookeeping for the JPEG Turbo library's
  * setjmp/longjmp simulated exceptions.
  */
struct JpegErrorState
{
  struct jpeg_error_mgr errorManager;
  jmp_buf               jumpBuffer;
};

static bool IsSubsamplingFormatEnabled(int chrominanceSubsampling)
{
  if(!gIsSubsamplingFormatTableInitialized)
  {
    for(int i = 0; i < TJ_NUMSAMP; i++)
    {
      const char* envName        = DALI_LIKELY(i < CHROMINANCE_SUBSAMPLING_OPTIONS_ENV_COUNT) ? CHROMINANCE_SUBSAMPLING_OPTIONS_ENV[i] : "";
      auto        valueString    = Dali::EnvironmentVariable::GetEnvironmentVariable(envName);
      gSubsamplingFormatTable[i] = valueString ? std::atoi(valueString) : false;
    }

    gIsSubsamplingFormatTableInitialized = true;
  }

  if(DALI_UNLIKELY(chrominanceSubsampling >= TJ_NUMSAMP))
  {
    DALI_LOG_ERROR("WARNING! Input subsampling value [%d] is bigger than turbojpeg library support [%d]\n", chrominanceSubsampling, TJ_NUMSAMP);
  }

  return chrominanceSubsampling < TJ_NUMSAMP ? gSubsamplingFormatTable[chrominanceSubsampling] : false;
}

/**
  * @brief Called by the JPEG library when it hits an error.
  * We jump out of the library so our loader code can return an error.
  */
void JpegErrorHandler(j_common_ptr cinfo)
{
  DALI_LOG_ERROR("JpegErrorHandler(): libjpeg-turbo fatal error in JPEG decoding.\n");
  /* cinfo->err really points to a JpegErrorState struct, so coerce pointer */
  JpegErrorState* myerr = reinterpret_cast<JpegErrorState*>(cinfo->err);

  /* Return control to the setjmp point */
  longjmp(myerr->jumpBuffer, 1);
}

void JpegOutputMessageHandler(j_common_ptr cinfo)
{
  /* Stop libjpeg from printing to stderr - Do Nothing */
}

/**
  * LibJPEG Turbo tjDecompress2 API doesn't distinguish between errors that still allow
  * the JPEG to be displayed and fatal errors.
  */
bool IsJpegErrorFatal(const std::string& errorMessage)
{
  if((errorMessage.find("Corrupt JPEG data") != std::string::npos) ||
     (errorMessage.find("Invalid SOS parameters") != std::string::npos) ||
     (errorMessage.find("Invalid JPEG file structure") != std::string::npos) ||
     (errorMessage.find("Unsupported JPEG process") != std::string::npos) ||
     (errorMessage.find("Unsupported marker type") != std::string::npos) ||
     (errorMessage.find("Bogus marker length") != std::string::npos) ||
     (errorMessage.find("Bogus DQT index") != std::string::npos) ||
     (errorMessage.find("Bogus Huffman table definition") != std::string::npos))
  {
    return false;
  }
  return true;
}

bool IsJpegDecodingFailed()
{
  std::string errorString = tjGetErrorStr();

  if(DALI_UNLIKELY(IsJpegErrorFatal(errorString)))
  {
    DALI_LOG_ERROR("%s\n", errorString.c_str());
    return true;
  }
  else
  {
    DALI_LOG_DEBUG_INFO("%s\n", errorString.c_str());
    return false;
  }
}

// helpers for safe exif memory handling
using ExifHandle = std::unique_ptr<ExifData, decltype(exif_data_free)*>;

ExifHandle MakeNullExifData()
{
  return ExifHandle{nullptr, exif_data_free};
}

ExifHandle MakeExifDataFromData(uint8_t* data, unsigned int size)
{
  return ExifHandle{exif_data_new_from_data(data, size), exif_data_free};
}

// Helpers for safe Jpeg memory handling
using JpegHandle = std::unique_ptr<void /*tjhandle*/, decltype(tjDestroy)*>;

JpegHandle MakeJpegCompressor()
{
  return JpegHandle{tjInitCompress(), tjDestroy};
}

JpegHandle MakeJpegDecompressor()
{
  return JpegHandle{tjInitDecompress(), tjDestroy};
}

using JpegMemoryHandle = std::unique_ptr<uint8_t, decltype(tjFree)*>;

JpegMemoryHandle MakeJpegMemory()
{
  return JpegMemoryHandle{nullptr, tjFree};
}

template<class T, class Deleter>
class UniquePointerSetter final
{
public:
  UniquePointerSetter(std::unique_ptr<T, Deleter>& uniquePointer)
  : mUniquePointer(uniquePointer),
    mRawPointer(nullptr)
  {
  }

  /// @brief Pointer to Pointer cast operator
  operator T* *()
  {
    return &mRawPointer;
  }

  /// @brief Destructor, reset the unique_ptr
  ~UniquePointerSetter()
  {
    mUniquePointer.reset(mRawPointer);
  }

private:
  std::unique_ptr<T, Deleter>& mUniquePointer;
  T*                           mRawPointer;
};

template<typename T, typename Deleter>
UniquePointerSetter<T, Deleter> SetPointer(std::unique_ptr<T, Deleter>& uniquePointer)
{
  return UniquePointerSetter<T, Deleter>{uniquePointer};
}

using TransformFunction      = std::function<void(PixelArray, unsigned, unsigned)>;
using TransformFunctionArray = std::array<TransformFunction, 3>; // 1, 3 and 4 bytes per pixel

/// @brief Select the transform function depending on the pixel format
TransformFunction GetTransformFunction(const TransformFunctionArray& functions,
                                       Pixel::Format                 pixelFormat)
{
  auto function = TransformFunction{};

  int decodedPixelSize = Pixel::GetBytesPerPixel(pixelFormat);
  switch(decodedPixelSize)
  {
    case DECODED_L8:
    {
      function = functions[0];
      break;
    }
    case DECODED_RGB888:
    {
      function = functions[1];
      break;
    }
    case DECODED_RGBA8888:
    {
      function = functions[2];
      break;
    }
    default:
    {
      DALI_LOG_ERROR("Transform operation not supported on this Pixel::Format!");
      function = functions[1];
      break;
    }
  }
  return function;
}

// Storing Exif fields as properties
template<class R, class V>
R ConvertExifNumeric(const ExifEntry& entry)
{
  return static_cast<R>((*reinterpret_cast<V*>(entry.data)));
}

void AddExifFieldPropertyMap(Dali::Property::Map& out, const ExifEntry& entry, ExifIfd ifd)
{
  auto shortName = std::string(exif_tag_get_name_in_ifd(entry.tag, ifd));
  switch(entry.format)
  {
    case EXIF_FORMAT_ASCII:
    {
      out.Insert(shortName, std::string(reinterpret_cast<char*>(entry.data), entry.size));
      break;
    }
    case EXIF_FORMAT_SHORT:
    {
      out.Insert(shortName, ConvertExifNumeric<int, uint16_t>(entry));
      break;
    }
    case EXIF_FORMAT_LONG:
    {
      out.Insert(shortName, ConvertExifNumeric<int, uint32_t>(entry));
      break;
    }
    case EXIF_FORMAT_SSHORT:
    {
      out.Insert(shortName, ConvertExifNumeric<int, int16_t>(entry));
      break;
    }
    case EXIF_FORMAT_SLONG:
    {
      out.Insert(shortName, ConvertExifNumeric<int, int32_t>(entry));
      break;
    }
    case EXIF_FORMAT_FLOAT:
    {
      out.Insert(shortName, ConvertExifNumeric<float, float>(entry));
      break;
    }
    case EXIF_FORMAT_DOUBLE:
    {
      out.Insert(shortName, ConvertExifNumeric<float, double>(entry));
      break;
    }
    case EXIF_FORMAT_RATIONAL:
    {
      auto                  values = reinterpret_cast<unsigned int*>(entry.data);
      Dali::Property::Array array;
      array.Add(static_cast<int>(values[0]));
      array.Add(static_cast<int>(values[1]));
      out.Insert(shortName, array);
      break;
    }
    case EXIF_FORMAT_SBYTE:
    {
      out.Insert(shortName, "EXIF_FORMAT_SBYTE Unsupported");
      break;
    }
    case EXIF_FORMAT_BYTE:
    {
      out.Insert(shortName, "EXIF_FORMAT_BYTE Unsupported");
      break;
    }
    case EXIF_FORMAT_SRATIONAL:
    {
      auto                  values = reinterpret_cast<int*>(entry.data);
      Dali::Property::Array array;
      array.Add(values[0]);
      array.Add(values[1]);
      out.Insert(shortName, array);
      break;
    }
    case EXIF_FORMAT_UNDEFINED:
    default:
    {
      std::stringstream ss;
      ss << "EXIF_FORMAT_UNDEFINED, size: " << entry.size << ", components: " << entry.components;
      out.Insert(shortName, ss.str());
    }
  }
}

/// @brief Apply a transform to a buffer
bool Transform(const TransformFunctionArray& transformFunctions,
               PixelArray                    buffer,
               int                           width,
               int                           height,
               Pixel::Format                 pixelFormat)
{
  auto transformFunction = GetTransformFunction(transformFunctions, pixelFormat);
  if(transformFunction)
  {
    transformFunction(buffer, width, height);
  }
  return bool(transformFunction);
}

/// @brief Auxiliar type to represent pixel data with different number of bytes
template<size_t N>
struct PixelType
{
  char _[N];
};

template<size_t N>
void Rotate180(PixelArray buffer, int width, int height)
{
  // Destination pixel, set as the first pixel of screen
  auto to = reinterpret_cast<PixelType<N>*>(buffer);

  // Source pixel, as the image is flipped horizontally and vertically,
  // the source pixel is the end of the buffer of size width * height
  auto from = reinterpret_cast<PixelType<N>*>(buffer) + width * height - 1;

  for(auto ix = 0, endLoop = (width * height) / 2; ix < endLoop; ++ix, ++to, --from)
  {
    std::swap(*from, *to);
  }
}

template<size_t N>
void FlipHorizontal(PixelArray buffer, int width, int height)
{
  for(auto iy = 0; iy < height; ++iy)
  {
    //Set the destination pixel as the beginning of the row
    auto to = reinterpret_cast<PixelType<N>*>(buffer) + width * iy;
    //Set the source pixel as the end of the row to flip in X axis
    auto from = reinterpret_cast<PixelType<N>*>(buffer) + width * (iy + 1) - 1;
    for(auto ix = 0; ix < width / 2; ++ix, ++to, --from)
    {
      std::swap(*from, *to);
    }
  }
}

template<size_t N>
void FlipVertical(PixelArray buffer, int width, int height)
{
  //Transform vertically only
  for(auto iy = 0; iy < height / 2; ++iy)
  {
    for(auto ix = 0; ix < width; ++ix)
    {
      auto to   = reinterpret_cast<PixelType<N>*>(buffer) + iy * width + ix;
      auto from = reinterpret_cast<PixelType<N>*>(buffer) + (height - 1 - iy) * width + ix;
      std::swap(*from, *to);
    }
  }
}

template<size_t N>
void Transpose(PixelArray buffer, int width, int height)
{
  using PixelT = PixelType<N>;
  Vector<PixelT> data;
  data.ResizeUninitialized(width * height);
  auto dataPtr = data.Begin();

  auto original = reinterpret_cast<PixelT*>(buffer);
  std::copy(original, original + width * height, dataPtr);

  auto to = original;
  for(auto iy = 0; iy < width; ++iy)
  {
    for(auto ix = 0; ix < height; ++ix, ++to)
    {
      auto from = dataPtr + ix * width + iy;
      *to       = *from;
    }
  }
}

template<size_t N>
void Rotate90(PixelArray buffer, int width, int height)
{
  using PixelT = PixelType<N>;
  Vector<PixelT> data;
  data.ResizeUninitialized(width * height);
  auto dataPtr = data.Begin();

  auto original = reinterpret_cast<PixelT*>(buffer);
  std::copy(original, original + width * height, dataPtr);

  std::swap(width, height);
  auto hw = width * height;
  hw      = -hw - 1;

  auto to   = original + width - 1;
  auto from = dataPtr;

  for(auto ix = width; --ix >= 0;)
  {
    for(auto iy = height; --iy >= 0; ++from)
    {
      *to = *from;
      to += width;
    }
    to += hw;
  }
}

template<size_t N>
void Transverse(PixelArray buffer, int width, int height)
{
  using PixelT = PixelType<N>;
  Vector<PixelT> data;
  data.ResizeUninitialized(width * height);
  auto dataPtr = data.Begin();

  auto original = reinterpret_cast<PixelT*>(buffer);
  std::copy(original, original + width * height, dataPtr);

  auto to = original;
  for(auto iy = 0; iy < width; iy++)
  {
    for(auto ix = 0; ix < height; ix++)
    {
      auto from = dataPtr + (height - ix) * width - 1 - iy;
      *to       = *from;
      ++to;
    }
  }
}

template<size_t N>
void Rotate270(PixelArray buffer, int width, int height)
{
  using PixelT = PixelType<N>;
  Vector<PixelT> data;
  data.ResizeUninitialized(width * height);
  auto dataPtr = data.Begin();

  auto original = reinterpret_cast<PixelT*>(buffer);
  std::copy(original, original + width * height, dataPtr);

  auto w = height;
  std::swap(width, height);
  auto hw = width * height;

  auto* to   = original + hw - width;
  auto* from = dataPtr;

  w  = -w;
  hw = hw + 1;
  for(auto ix = width; --ix >= 0;)
  {
    for(auto iy = height; --iy >= 0;)
    {
      *to = *from;
      ++from;
      to += w;
    }
    to += hw;
  }
}

void GetJpegPixelFormat(int jpegColorspace, TJPF& pixelLibJpegType, Pixel::Format& pixelFormat)
{
  pixelLibJpegType = TJPF_RGB;
  pixelFormat      = Pixel::RGB888;

  switch(jpegColorspace)
  {
    case TJCS_RGB:
    // YCbCr is not an absolute colorspace but rather a mathematical transformation of RGB designed solely for storage and transmission.
    // YCbCr images must be converted to RGB before they can actually be displayed.
    case TJCS_YCbCr:
    {
      pixelLibJpegType = TJPF_RGB;
      pixelFormat      = Pixel::RGB888;
      break;
    }
    case TJCS_GRAY:
    {
      pixelLibJpegType = TJPF_GRAY;
      pixelFormat      = Pixel::L8;
      break;
    }
    case TJCS_CMYK:
    case TJCS_YCCK:
    {
      pixelLibJpegType = TJPF_CMYK;
      pixelFormat      = Pixel::RGB888;
      break;
    }
    default:
    {
      pixelLibJpegType = TJPF_RGB;
      pixelFormat      = Pixel::RGB888;
      break;
    }
  }
}

bool TransformBitmap(int scaledPreXformWidth, int scaledPreXformHeight, JpegTransform transform, uint8_t* bitmapPixelBuffer, Pixel::Format pixelFormat)
{
  const unsigned int bufferWidth  = Dali::TizenPlatform::GetTextureDimension(scaledPreXformWidth);
  const unsigned int bufferHeight = Dali::TizenPlatform::GetTextureDimension(scaledPreXformHeight);

  bool result = false;

  switch(transform)
  {
    case JpegTransform::NONE:
    {
      result = true;
      break;
    }
    // 3 orientation changes for a camera held perpendicular to the ground or upside-down:
    case JpegTransform::ROTATE_180:
    {
      static auto rotate180Functions = TransformFunctionArray{
        &Rotate180<1>,
        &Rotate180<3>,
        &Rotate180<4>,
      };
      result = Transform(rotate180Functions, bitmapPixelBuffer, bufferWidth, bufferHeight, pixelFormat);
      break;
    }
    case JpegTransform::ROTATE_270:
    {
      static auto rotate270Functions = TransformFunctionArray{
        &Rotate270<1>,
        &Rotate270<3>,
        &Rotate270<4>,
      };
      result = Transform(rotate270Functions, bitmapPixelBuffer, bufferWidth, bufferHeight, pixelFormat);
      break;
    }
    case JpegTransform::ROTATE_90:
    {
      static auto rotate90Functions = TransformFunctionArray{
        &Rotate90<1>,
        &Rotate90<3>,
        &Rotate90<4>,
      };
      result = Transform(rotate90Functions, bitmapPixelBuffer, bufferWidth, bufferHeight, pixelFormat);
      break;
    }
    case JpegTransform::FLIP_VERTICAL:
    {
      static auto flipVerticalFunctions = TransformFunctionArray{
        &FlipVertical<1>,
        &FlipVertical<3>,
        &FlipVertical<4>,
      };
      result = Transform(flipVerticalFunctions, bitmapPixelBuffer, bufferWidth, bufferHeight, pixelFormat);
      break;
    }
    // Less-common orientation changes, since they don't correspond to a camera's physical orientation:
    case JpegTransform::FLIP_HORIZONTAL:
    {
      static auto flipHorizontalFunctions = TransformFunctionArray{
        &FlipHorizontal<1>,
        &FlipHorizontal<3>,
        &FlipHorizontal<4>,
      };
      result = Transform(flipHorizontalFunctions, bitmapPixelBuffer, bufferWidth, bufferHeight, pixelFormat);
      break;
    }
    case JpegTransform::TRANSPOSE:
    {
      static auto transposeFunctions = TransformFunctionArray{
        &Transpose<1>,
        &Transpose<3>,
        &Transpose<4>,
      };
      result = Transform(transposeFunctions, bitmapPixelBuffer, bufferWidth, bufferHeight, pixelFormat);
      break;
    }
    case JpegTransform::TRANSVERSE:
    {
      static auto transverseFunctions = TransformFunctionArray{
        &Transverse<1>,
        &Transverse<3>,
        &Transverse<4>,
      };
      result = Transform(transverseFunctions, bitmapPixelBuffer, bufferWidth, bufferHeight, pixelFormat);
      break;
    }
    default:
    {
      DALI_LOG_ERROR("Unsupported JPEG Orientation transformation: %x.\n", transform);
      break;
    }
  }
  return result;
}

bool LoadJpegFile(const Dali::ImageLoader::Input& input, Vector<uint8_t>& jpegBuffer, unsigned int& jpegBufferSize)
{
  FILE* const fp = input.file;

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_END)))
  {
    DALI_LOG_ERROR("Error seeking to end of file\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  long positionIndicator = ftell(fp);
  jpegBufferSize         = 0u;
  if(positionIndicator > -1L)
  {
    jpegBufferSize = static_cast<unsigned int>(positionIndicator);
  }

  if(DALI_UNLIKELY(0u == jpegBufferSize))
  {
    DALI_LOG_ERROR("Jpeg buffer size error\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking to start of file\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  try
  {
    jpegBuffer.ResizeUninitialized(jpegBufferSize);
  }
  catch(...)
  {
    DALI_LOG_ERROR("Could not allocate temporary memory to hold JPEG file of size %uMB.\n", jpegBufferSize / 1048576U);
    return false;
  }
  uint8_t* const jpegBufferPtr = jpegBuffer.Begin();

  // Pull the compressed JPEG image bytes out of a file and into memory:
  if(DALI_UNLIKELY(fread(jpegBufferPtr, 1, jpegBufferSize, fp) != jpegBufferSize))
  {
    DALI_LOG_ERROR("Error on image file read.\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking to start of file\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  return true;
}

/**
 * @brief Helper function to convert from Turbo Jpeg Pixel Format as TJPF_CMYK to RGB888 by naive method.
 *
 * @param[in] cmykBuffer buffer of cmyk.
 * @param[in] rgbBuffer buffer of Pixel::RGB888
 * @param[in] width width of image.
 * @param[in] height height of image.
 */
void ConvertTjpfCMYKToRGB888(PixelArray __restrict__ cmykBuffer, PixelArray __restrict__ rgbBuffer, int32_t width, int32_t height)
{
  const int32_t pixelCount = width * height;
  const uint8_t cmykBpp    = 4u;
  const uint8_t bpp        = 3u;

  const PixelArray cmykBufferEnd = cmykBuffer + pixelCount * cmykBpp;
  // Convert every pixel
  while(cmykBuffer != cmykBufferEnd)
  {
    const uint8_t channelK = *(cmykBuffer + 3u);
    *(rgbBuffer + 0u)      = Dali::Internal::Platform::MultiplyAndNormalizeColor(*(cmykBuffer + 0u), channelK);
    *(rgbBuffer + 1u)      = Dali::Internal::Platform::MultiplyAndNormalizeColor(*(cmykBuffer + 1u), channelK);
    *(rgbBuffer + 2u)      = Dali::Internal::Platform::MultiplyAndNormalizeColor(*(cmykBuffer + 2u), channelK);
    cmykBuffer += cmykBpp;
    rgbBuffer += bpp;
  }
}
} // namespace

namespace Dali
{
namespace TizenPlatform
{
bool          DecodeJpeg(const Dali::ImageLoader::Input& input, std::vector<Dali::Devel::PixelBuffer>& pixelBuffers, bool decodeToYuv);
JpegTransform ConvertExifOrientation(ExifData* exifData);
bool          TransformSize(int requiredWidth, int requiredHeight, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, JpegTransform transform, int& preXformImageWidth, int& preXformImageHeight, int& postXformImageWidth, int& postXformImageHeight);

bool LoadJpegHeader(FILE* fp, unsigned int& width, unsigned int& height)
{
  // using libjpeg API to avoid having to read the whole file in a buffer
  struct jpeg_decompress_struct cinfo;
  struct JpegErrorState         jerr;
  cinfo.err = jpeg_std_error(&jerr.errorManager);

  jerr.errorManager.output_message = JpegOutputMessageHandler;
  jerr.errorManager.error_exit     = JpegErrorHandler;

  // On error exit from the JPEG lib, control will pass via JpegErrorHandler
  // into this branch body for cleanup and error return:
  if(DALI_UNLIKELY(setjmp(jerr.jumpBuffer)))
  {
    DALI_LOG_ERROR("setjmp failed\n");
    jpeg_destroy_decompress(&cinfo);
    return false;
  }

// jpeg_create_decompress internally uses C casts
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
  jpeg_create_decompress(&cinfo);
#pragma GCC diagnostic pop

  jpeg_stdio_src(&cinfo, fp);

  // Check header to see if it is  JPEG file
  if(DALI_UNLIKELY(jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK))
  {
    DALI_LOG_ERROR("jpeg_read_header failed\n");
    width = height = 0;
    jpeg_destroy_decompress(&cinfo);
    return false;
  }

  width  = cinfo.image_width;
  height = cinfo.image_height;

  jpeg_destroy_decompress(&cinfo);
  return true;
}

bool LoadBitmapFromJpeg(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap)
{
  std::vector<Dali::Devel::PixelBuffer> pixelBuffers;

  bool result = DecodeJpeg(input, pixelBuffers, false);
  if(!result && pixelBuffers.empty())
  {
    bitmap.Reset();
  }
  else
  {
    bitmap = pixelBuffers[0];
  }
  return result;
}

bool LoadPlanesFromJpeg(const Dali::ImageLoader::Input& input, std::vector<Dali::Devel::PixelBuffer>& pixelBuffers)
{
  return DecodeJpeg(input, pixelBuffers, true);
}

bool DecodeJpeg(const Dali::ImageLoader::Input& input, std::vector<Dali::Devel::PixelBuffer>& pixelBuffers, bool decodeToYuv)
{
  Vector<uint8_t> jpegBuffer;
  unsigned int    jpegBufferSize = 0u;

  if(!LoadJpegFile(input, jpegBuffer, jpegBufferSize))
  {
    DALI_LOG_ERROR("LoadJpegFile failed\n");
    return false;
  }

  auto jpeg = MakeJpegDecompressor();
  if(DALI_UNLIKELY(!jpeg))
  {
    DALI_LOG_ERROR("%s\n", tjGetErrorStr());
    return false;
  }

  uint8_t* const jpegBufferPtr = jpegBuffer.Begin();
  auto           transform     = JpegTransform::NONE;

  // extract exif data
  auto exifData = MakeExifDataFromData(jpegBufferPtr, jpegBufferSize);

  if(exifData && input.reorientationRequested)
  {
    transform = ConvertExifOrientation(exifData.get());
  }

  // Push jpeg data in memory buffer through TurboJPEG decoder to make a raw pixel array:
  int chrominanceSubsampling = -1;
  int preXformImageWidth = 0, preXformImageHeight = 0;
  int jpegColorspace = -1;

  if(tjDecompressHeader3(jpeg.get(), jpegBufferPtr, jpegBufferSize, &preXformImageWidth, &preXformImageHeight, &chrominanceSubsampling, &jpegColorspace) == -1)
  {
    DALI_LOG_ERROR("%s\n", tjGetErrorStr());
    // Do not set width and height to 0 or return early as this sometimes fails only on determining subsampling type.
  }

  if(DALI_UNLIKELY(preXformImageWidth == 0 || preXformImageHeight == 0))
  {
    DALI_LOG_ERROR("Invalid Image!\n");
    return false;
  }

  int requiredWidth  = input.scalingParameters.dimensions.GetWidth();
  int requiredHeight = input.scalingParameters.dimensions.GetHeight();

  // If transform is a 90 or 270 degree rotation, the logical width and height
  // request from the client needs to be adjusted to account by effectively
  // rotating that too, and the final width and height need to be swapped:
  int postXformImageWidth  = preXformImageWidth;
  int postXformImageHeight = preXformImageHeight;

  int scaledPreXformWidth   = preXformImageWidth;
  int scaledPreXformHeight  = preXformImageHeight;
  int scaledPostXformWidth  = postXformImageWidth;
  int scaledPostXformHeight = postXformImageHeight;

  TransformSize(requiredWidth, requiredHeight, input.scalingParameters.scalingMode, input.scalingParameters.samplingMode, transform, scaledPreXformWidth, scaledPreXformHeight, scaledPostXformWidth, scaledPostXformHeight);

  bool result = false;

  // Check decoding format
  if(decodeToYuv && IsSubsamplingFormatEnabled(chrominanceSubsampling) && transform == JpegTransform::NONE)
  {
    uint8_t* planes[3];

    // Allocate buffers for each plane and decompress the jpeg buffer into the buffers
    for(int i = 0; i < 3; i++)
    {
      auto planeSize = tjPlaneSizeYUV(i, scaledPostXformWidth, 0, scaledPostXformHeight, chrominanceSubsampling);

      if(DALI_UNLIKELY(planeSize == static_cast<decltype(planeSize)>(-1)))
      {
        DALI_LOG_ERROR("Plane image for %d'th tjPlaneSizeYUV failed! planeSize : %lu\n", i, planeSize);
        pixelBuffers.clear();
        return false;
      }

      uint8_t* buffer = static_cast<uint8_t*>(malloc(planeSize));
      if(DALI_UNLIKELY(!buffer))
      {
        DALI_LOG_ERROR("Buffer allocation is failed [%d]\n", planeSize);
        pixelBuffers.clear();
        return false;
      }

      int           width, height, planeWidth;
      Pixel::Format pixelFormat = Pixel::RGB888;

      if(i == 0)
      {
        // luminance plane
        width       = scaledPostXformWidth;
        height      = scaledPostXformHeight;
        planeWidth  = tjPlaneWidth(i, scaledPostXformWidth, chrominanceSubsampling);
        pixelFormat = Pixel::L8;
      }
      else
      {
        // chrominance plane
        width       = tjPlaneWidth(i, scaledPostXformWidth, chrominanceSubsampling);
        height      = tjPlaneHeight(i, scaledPostXformHeight, chrominanceSubsampling);
        planeWidth  = width;
        pixelFormat = (i == 1 ? Pixel::CHROMINANCE_U : Pixel::CHROMINANCE_V);
      }

      if(DALI_UNLIKELY(width <= 0 || height <= 0 || planeWidth <= 0))
      {
        DALI_LOG_ERROR("Plane image for %d'th size invalid! width : %d, height : %d, planeWidth : %d\n", i, width, height, planeWidth);
        pixelBuffers.clear();
        free(buffer);
        return false;
      }

      Internal::Adaptor::PixelBufferPtr internal = Internal::Adaptor::PixelBuffer::New(buffer, planeSize, width, height, planeWidth * Pixel::GetBytesPerPixel(pixelFormat), pixelFormat);
      Dali::Devel::PixelBuffer          bitmap   = Devel::PixelBuffer(internal.Get());
      planes[i]                                  = buffer;
      pixelBuffers.push_back(bitmap);
    }

    const int flags = 0;

    int decodeResult = tjDecompressToYUVPlanes(jpeg.get(), jpegBufferPtr, jpegBufferSize, reinterpret_cast<uint8_t**>(&planes), scaledPostXformWidth, nullptr, scaledPostXformHeight, flags);
    if(DALI_UNLIKELY(decodeResult == -1 && IsJpegDecodingFailed()))
    {
      pixelBuffers.clear();
      return false;
    }

    result = true;
  }
  else
  {
    // Colorspace conversion options
    TJPF          pixelLibJpegType = TJPF_RGB;
    Pixel::Format pixelFormat      = Pixel::RGB888;

    GetJpegPixelFormat(jpegColorspace, pixelLibJpegType, pixelFormat);

    // Allocate a bitmap and decompress the jpeg buffer into its pixel buffer:
    Dali::Devel::PixelBuffer bitmap = Dali::Devel::PixelBuffer::New(scaledPostXformWidth, scaledPostXformHeight, pixelFormat);

    // Set metadata
    if(DALI_LIKELY(exifData))
    {
      std::unique_ptr<Property::Map> exifMap = std::make_unique<Property::Map>();

      for(auto k = 0u; k < EXIF_IFD_COUNT; ++k)
      {
        auto content = exifData->ifd[k];
        for(auto i = 0u; i < content->count; ++i)
        {
          auto&&      tag       = content->entries[i];
          const char* shortName = exif_tag_get_name_in_ifd(tag->tag, static_cast<ExifIfd>(k));
          if(shortName)
          {
            AddExifFieldPropertyMap(*exifMap, *tag, static_cast<ExifIfd>(k));
          }
        }
      }

      GetImplementation(bitmap).SetMetadata(std::move(exifMap));
    }

    auto      bitmapPixelBuffer = bitmap.GetBuffer();
    const int flags             = 0;

    int decodeResult = -1;
    if(pixelLibJpegType == TJPF_CMYK)
    {
      // Currently we support only for 4 bytes per each CMYK pixel.
      const uint8_t cmykBytesPerPixel = 4u;

      uint8_t* cmykBuffer = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * scaledPostXformWidth * scaledPostXformHeight * cmykBytesPerPixel));

      if(DALI_UNLIKELY(!cmykBuffer))
      {
        DALI_LOG_ERROR("cmykBuffer allocation is failed [%zu]\n", sizeof(uint8_t) * scaledPostXformWidth * scaledPostXformHeight * cmykBytesPerPixel);
        return false;
      }

      decodeResult = tjDecompress2(jpeg.get(), jpegBufferPtr, jpegBufferSize, reinterpret_cast<uint8_t*>(cmykBuffer), scaledPreXformWidth, 0, scaledPreXformHeight, pixelLibJpegType, flags);
      if(DALI_UNLIKELY(decodeResult == -1 && IsJpegDecodingFailed()))
      {
        free(cmykBuffer);
        return false;
      }
      ConvertTjpfCMYKToRGB888(cmykBuffer, bitmapPixelBuffer, scaledPostXformWidth, scaledPostXformHeight);

      free(cmykBuffer);
    }
    else
    {
      decodeResult = tjDecompress2(jpeg.get(), jpegBufferPtr, jpegBufferSize, reinterpret_cast<uint8_t*>(bitmapPixelBuffer), scaledPreXformWidth, 0, scaledPreXformHeight, pixelLibJpegType, flags);
      if(DALI_UNLIKELY(decodeResult == -1 && IsJpegDecodingFailed()))
      {
        return false;
      }
    }
    pixelBuffers.push_back(bitmap);

    // Transform bitmap
    result = TransformBitmap(scaledPreXformWidth, scaledPreXformHeight, transform, bitmapPixelBuffer, pixelFormat);
  }

  return result;
}

bool EncodeToJpeg(const uint8_t* const pixelBuffer, Vector<uint8_t>& encodedPixels, const std::size_t width, const std::size_t height, const Pixel::Format pixelFormat, unsigned quality)
{
  if(DALI_UNLIKELY(!pixelBuffer))
  {
    DALI_LOG_ERROR("Null input buffer\n");
    return false;
  }

  // Translate pixel format enum:
  int jpegPixelFormat = -1;

  switch(pixelFormat)
  {
    case Pixel::L8:
    {
      jpegPixelFormat = TJPF_GRAY;
      break;
    }
    case Pixel::RGB888:
    {
      jpegPixelFormat = TJPF_RGB;
      break;
    }
    case Pixel::RGBA8888:
    {
      // Ignore the alpha:
      jpegPixelFormat = TJPF_RGBX;
      break;
    }
    case Pixel::BGRA8888:
    {
      // Ignore the alpha:
      jpegPixelFormat = TJPF_BGRX;
      break;
    }
    default:
    {
      DALI_LOG_ERROR("Unsupported pixel format for encoding to JPEG. Format enum : [%d]\n", pixelFormat);
      return false;
    }
  }

  // Assert quality is in the documented allowable range of the jpeg-turbo lib:
  DALI_ASSERT_DEBUG(quality >= 1);
  DALI_ASSERT_DEBUG(quality <= 100);
  if(quality < 1)
  {
    quality = 1;
  }
  if(quality > 100)
  {
    quality = 100;
  }

  // Initialise a JPEG codec:
  {
    auto jpeg = MakeJpegCompressor();
    if(DALI_UNLIKELY(!jpeg))
    {
      DALI_LOG_ERROR("JPEG Compressor init failed: %s\n", tjGetErrorStr());
      return false;
    }

    // Safely wrap the jpeg codec's buffer in case we are about to throw, then
    // save the pixels to a persistent buffer that we own and let our cleaner
    // class clean up the buffer as it goes out of scope:
    auto dstBuffer = MakeJpegMemory();

    // Run the compressor:
    unsigned long dstBufferSize = 0;
    const int     flags         = 0;

    if(DALI_UNLIKELY(tjCompress2(jpeg.get(),
                                 const_cast<uint8_t*>(pixelBuffer),
                                 width,
                                 0,
                                 height,
                                 jpegPixelFormat,
                                 SetPointer(dstBuffer),
                                 &dstBufferSize,
                                 TJSAMP_444,
                                 quality,
                                 flags)))
    {
      DALI_LOG_ERROR("JPEG Compression failed: %s\n", tjGetErrorStr());
      return false;
    }

    encodedPixels.ResizeUninitialized(dstBufferSize);
    memcpy(encodedPixels.Begin(), dstBuffer.get(), dstBufferSize);
  }
  return true;
}

JpegTransform ConvertExifOrientation(ExifData* exifData)
{
  auto             transform   = JpegTransform::NONE;
  ExifEntry* const entry       = exif_data_get_entry(exifData, EXIF_TAG_ORIENTATION);
  int              orientation = 0;
  if(entry)
  {
    orientation = exif_get_short(entry->data, exif_data_get_byte_order(entry->parent->parent));
    switch(orientation)
    {
      case 1:
      {
        transform = JpegTransform::NONE;
        break;
      }
      case 2:
      {
        transform = JpegTransform::FLIP_HORIZONTAL;
        break;
      }
      case 3:
      {
        transform = JpegTransform::ROTATE_180;
        break;
      }
      case 4:
      {
        transform = JpegTransform::FLIP_VERTICAL;
        break;
      }
      case 5:
      {
        transform = JpegTransform::TRANSPOSE;
        break;
      }
      case 6:
      {
        transform = JpegTransform::ROTATE_90;
        break;
      }
      case 7:
      {
        transform = JpegTransform::TRANSVERSE;
        break;
      }
      case 8:
      {
        transform = JpegTransform::ROTATE_270;
        break;
      }
      default:
      {
        // Try to keep loading the file, but let app developer know there was something fishy:
        DALI_LOG_DEBUG_INFO("Incorrect/Unknown Orientation setting found in EXIF header of JPEG image (%x). Orientation setting will be ignored.\n", entry);
        break;
      }
    }
  }
  return transform;
}

bool TransformSize(int requiredWidth, int requiredHeight, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, JpegTransform transform, int& preXformImageWidth, int& preXformImageHeight, int& postXformImageWidth, int& postXformImageHeight)
{
  bool success = true;
  if(transform == JpegTransform::TRANSPOSE || transform == JpegTransform::ROTATE_90 || transform == JpegTransform::TRANSVERSE || transform == JpegTransform::ROTATE_270)
  {
    std::swap(requiredWidth, requiredHeight);
    std::swap(postXformImageWidth, postXformImageHeight);
  }

  // Apply the special rules for when there are one or two zeros in requested dimensions:
  const ImageDimensions correctedDesired = Internal::Platform::CalculateDesiredDimensions(ImageDimensions(postXformImageWidth, postXformImageHeight), ImageDimensions(requiredWidth, requiredHeight), fittingMode);
  requiredWidth                          = correctedDesired.GetWidth();
  requiredHeight                         = correctedDesired.GetHeight();

  // Rescale image during decode using one of the decoder's built-in rescaling
  // ratios (expected to be powers of 2), keeping the final image at least as
  // wide and high as was requested:

  int              numFactors = 0;
  tjscalingfactor* factors    = tjGetScalingFactors(&numFactors);
  if(DALI_UNLIKELY(factors == NULL))
  {
    DALI_LOG_DEBUG_INFO("TurboJpeg tjGetScalingFactors error!\n");
    success = false;
  }
  else
  {
    // Internal jpeg downscaling is the same as our BOX_X sampling modes so only
    // apply it if the application requested one of those:
    // (use a switch case here so this code will fail to compile if other modes are added)
    bool useTurboJpegScaleFactor = true;
    switch(samplingMode)
    {
      case SamplingMode::BOX:
      case SamplingMode::BOX_THEN_NEAREST:
      case SamplingMode::BOX_THEN_LINEAR:
      case SamplingMode::BOX_THEN_LANCZOS:
      case SamplingMode::DONT_CARE:
      {
        useTurboJpegScaleFactor = true;
        break;
      }
      case SamplingMode::NO_FILTER:
      case SamplingMode::NEAREST:
      case SamplingMode::LINEAR:
      case SamplingMode::LANCZOS:
      {
        useTurboJpegScaleFactor = false;
        break;
      }
    }

    int scaleFactorIndex(-1);
    int fittedScaledWidth(postXformImageWidth);
    int fittedScaledHeight(postXformImageHeight);
    if(useTurboJpegScaleFactor)
    {
      // Find nearest supported scaling factor (factors are in sequential order, getting smaller)
      for(int i = 0; i < numFactors; ++i)
      {
        int  scaledWidth        = TJSCALED(postXformImageWidth, factors[i]);
        int  scaledHeight       = TJSCALED(postXformImageHeight, factors[i]);
        bool widthLessRequired  = scaledWidth < requiredWidth;
        bool heightLessRequired = scaledHeight < requiredHeight;
        // If either scaled dimension is smaller than the desired one, we were done at the last iteration
        if(((fittingMode == FittingMode::SCALE_TO_FILL) || (fittingMode == FittingMode::VISUAL_FITTING)) && (widthLessRequired || heightLessRequired))
        {
          break;
        }
        // If both dimensions are smaller than the desired one, we were done at the last iteration:
        if((fittingMode == FittingMode::SHRINK_TO_FIT) && (widthLessRequired && heightLessRequired))
        {
          break;
        }
        // If the width is smaller than the desired one, we were done at the last iteration:
        if(fittingMode == FittingMode::FIT_WIDTH && widthLessRequired)
        {
          break;
        }
        // If the width is smaller than the desired one, we were done at the last iteration:
        if(fittingMode == FittingMode::FIT_HEIGHT && heightLessRequired)
        {
          break;
        }
        // This factor stays is within our fitting mode constraint so remember it:
        scaleFactorIndex   = i;
        fittedScaledWidth  = scaledWidth;
        fittedScaledHeight = scaledHeight;
      }
    }

    const int maxTextureSize = static_cast<int>(Dali::GetMaxTextureSize());

    // Regardless of requested size, downscale to avoid exceeding the maximum texture size:
    if(fittedScaledWidth >= maxTextureSize ||
       fittedScaledHeight >= maxTextureSize)
    {
      for(int i = scaleFactorIndex + 1; i < numFactors; ++i)
      {
        // Continue downscaling to below maximum texture size (if possible)
        scaleFactorIndex   = i;
        fittedScaledWidth  = TJSCALED(postXformImageWidth, factors[i]);
        fittedScaledHeight = TJSCALED(postXformImageHeight, factors[i]);

        if(fittedScaledWidth < maxTextureSize &&
           fittedScaledHeight < maxTextureSize)
        {
          // Current scale-factor downscales to below maximum texture size
          break;
        }
      }
    }

    // We have finally chosen the scale-factor, return width/height values
    if(scaleFactorIndex >= 0 && scaleFactorIndex < numFactors)
    {
      preXformImageWidth   = TJSCALED(preXformImageWidth, (factors[scaleFactorIndex]));
      preXformImageHeight  = TJSCALED(preXformImageHeight, (factors[scaleFactorIndex]));
      postXformImageWidth  = fittedScaledWidth;
      postXformImageHeight = fittedScaledHeight;
    }
  }

  return success;
}

ExifHandle LoadExifData(FILE* fp)
{
  auto    exifData = MakeNullExifData();
  uint8_t dataBuffer[1024];

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking to start of file\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
  }
  else
  {
    auto exifLoader = std::unique_ptr<ExifLoader, decltype(exif_loader_unref)*>{
      exif_loader_new(), exif_loader_unref};

    while(!feof(fp))
    {
      int size = fread(dataBuffer, 1, sizeof(dataBuffer), fp);
      if(size <= 0)
      {
        DALI_PRINT_SYSTEM_ERROR_LOG();
        break;
      }
      if(!exif_loader_write(exifLoader.get(), dataBuffer, size))
      {
        break;
      }
    }

    exifData.reset(exif_loader_get_data(exifLoader.get()));
  }

  return exifData;
}

bool LoadJpegHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  unsigned int requiredWidth  = input.scalingParameters.dimensions.GetWidth();
  unsigned int requiredHeight = input.scalingParameters.dimensions.GetHeight();
  FILE* const  fp             = input.file;

  bool success = false;

  unsigned int headerWidth;
  unsigned int headerHeight;

  success = LoadJpegHeader(fp, headerWidth, headerHeight);
  if(DALI_LIKELY(success))
  {
    auto transform = JpegTransform::NONE;

    if(input.reorientationRequested)
    {
      auto exifData = LoadExifData(fp);
      if(exifData)
      {
        transform = ConvertExifOrientation(exifData.get());
      }
    }

    if(requiredWidth == 0 && requiredHeight == 0)
    {
      if(transform == JpegTransform::TRANSPOSE || transform == JpegTransform::ROTATE_90 || transform == JpegTransform::TRANSVERSE || transform == JpegTransform::ROTATE_270)
      {
        std::swap(headerWidth, headerHeight);
      }
    }
    else
    {
      int preXformImageWidth   = headerWidth;
      int preXformImageHeight  = headerHeight;
      int postXformImageWidth  = headerWidth;
      int postXformImageHeight = headerHeight;

      success = TransformSize(requiredWidth, requiredHeight, input.scalingParameters.scalingMode, input.scalingParameters.samplingMode, transform, preXformImageWidth, preXformImageHeight, postXformImageWidth, postXformImageHeight);
      if(DALI_LIKELY(success))
      {
        headerWidth  = postXformImageWidth;
        headerHeight = postXformImageHeight;
      }
    }
    width  = headerWidth;
    height = headerHeight;
  }

  return success;
}

} // namespace TizenPlatform

} // namespace Dali
