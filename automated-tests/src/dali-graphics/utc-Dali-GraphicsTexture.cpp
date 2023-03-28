/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
 */

#include <dali-test-suite-utils.h>
#include <dali/dali.h>

#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <test-actor-utils.h>
#include <test-graphics-application.h>
#include <test-graphics-framebuffer.h>

using namespace Dali;

namespace
{
} // namespace

void utc_dali_texture_startup(void)
{
  test_return_value = TET_UNDEF;
}
void utc_dali_texture_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliTextureConvertUpload(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsConvertUpload - Test that an RGB image is converted to RGBA after upload");

  auto& gl = app.GetGlAbstraction();
  gl.EnableTextureCallTrace(true);
  gl.EnableTexParameterCallTrace(true);

  int     size    = 200;
  Texture texture = Texture::New(TextureType::TEXTURE_2D, Pixel::RGBA8888, size, size);

  int       bufferSize = size * size * 3;
  uint8_t*  buffer     = reinterpret_cast<uint8_t*>(malloc(bufferSize));
  PixelData pixelData  = PixelData::New(buffer, bufferSize, size, size, Pixel::RGB888, PixelData::FREE);
  texture.Upload(pixelData, 0u, 0u, 0u, 0u, size, size);

  Sampler sampler = Sampler::New();
  sampler.SetFilterMode(FilterMode::LINEAR, FilterMode::LINEAR);

  TextureSet textureSet = TextureSet::New();
  textureSet.SetTexture(0u, texture);
  textureSet.SetSampler(0u, sampler);

  Actor dummyActor = CreateRenderableActor2(textureSet, "", "");
  app.GetScene().Add(dummyActor);

  app.SendNotification();
  app.Render(16);

  // Check that TexImage2D was called with right format
  auto&                       textureTrace      = gl.GetTextureTrace();
  auto&                       texParameterTrace = gl.GetTexParameterTrace();
  TraceCallStack::NamedParams namedParams;
  DALI_TEST_CHECK(textureTrace.FindMethodAndGetParameters("TexImage2D", namedParams));
  std::stringstream s;
  s << std::hex << GL_RGBA;

  DALI_TEST_EQUALS(texParameterTrace.CountMethod("TexParameteri"), 4, TEST_LOCATION);
  std::stringstream out;
  out << std::hex << GL_TEXTURE_2D << ", " << GL_TEXTURE_MIN_FILTER << ", " << GL_LINEAR;
  DALI_TEST_EQUALS(texParameterTrace.TestMethodAndParams(0, "TexParameteri", out.str()), true, TEST_LOCATION);

  DALI_TEST_EQUALS(namedParams["format"].str(), s.str(), TEST_LOCATION);
  END_TEST;
}
