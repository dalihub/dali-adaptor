//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include <dali/public-api/adaptor-framework/common/pixmap-image.h>

// INTERNAL INCLUDES
#include <internal/common/pixmap-image-impl.h>

namespace Dali
{

PixmapImagePtr PixmapImage::New(unsigned int width, unsigned int height, ColorDepth depth, Adaptor& adaptor)
{
  boost::any empty;
  PixmapImagePtr image = new PixmapImage(width, height, depth, adaptor, empty);
  return image;
}

PixmapImagePtr PixmapImage::New(boost::any pixmap, Adaptor& adaptor)
{
  PixmapImagePtr image = new PixmapImage(0, 0, COLOR_DEPTH_DEFAULT, adaptor, pixmap);
  return image;
}

boost::any PixmapImage::GetPixmap(PixmapAPI api)
{
  return mImpl->GetPixmap(api);
}

boost::any PixmapImage::GetDisplay()
{
  return mImpl->GetDisplay();
}

bool PixmapImage::GetPixels(std::vector<unsigned char> &pixbuf, unsigned int &width, unsigned int &height, Pixel::Format& pixelFormat ) const
{
  return mImpl->GetPixels( pixbuf, width, height, pixelFormat );
}

bool PixmapImage::EncodeToFile(const std::string& filename) const
{
  return mImpl->EncodeToFile(filename);
}

bool PixmapImage::GlExtensionCreate()
{
  return mImpl->GlExtensionCreate();
}

void PixmapImage::GlExtensionDestroy()
{
  mImpl->GlExtensionDestroy();
}

unsigned int PixmapImage::TargetTexture()
{
  return mImpl->TargetTexture();
}

void PixmapImage::PrepareTexture()
{

}

unsigned int PixmapImage::GetWidth() const
{
  return mImpl->GetWidth();
}

unsigned int PixmapImage::GetHeight() const
{
  return mImpl->GetHeight();
}

Pixel::Format PixmapImage::GetPixelFormat() const
{
  return mImpl->GetPixelFormat();
}

PixmapImage::PixmapImage(unsigned int width, unsigned int height, ColorDepth depth, Adaptor& adaptor, boost::any pixmap)
{
   mImpl = Internal::Adaptor::PixmapImage::New( width, height, depth, adaptor, pixmap);
}

PixmapImage::~PixmapImage()
{
  delete mImpl;
}

} // namespace Dali
