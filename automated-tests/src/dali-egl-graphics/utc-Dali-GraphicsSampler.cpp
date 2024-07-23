/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <test-graphics-egl-application.h>
#include <test-graphics-sampler.h>

using namespace Dali;

void utc_dali_sampler_startup(void)
{
  test_return_value = TET_UNDEF;
}
void utc_dali_sampler_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliGraphicsSamplerDefault(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliSampler01 - check defaults");

  Texture texture = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);
  Actor   actor   = CreateRenderableActor(texture, "myVertShaderSource", "myFragShaderSource");
  app.GetScene().Add(actor);

  auto& gl           = app.GetGlAbstraction();
  auto& glParamTrace = gl.GetTexParameterTrace();
  glParamTrace.Enable(true);
  glParamTrace.EnableLogging(true);

  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  TraceCallStack::NamedParams minFilter;
  minFilter["pname"] << std::hex << GL_TEXTURE_MIN_FILTER;
  std::ostringstream LINEAR;
  LINEAR << GL_LINEAR;
  std::ostringstream CLAMP_TO_EDGE;
  CLAMP_TO_EDGE << GL_CLAMP_TO_EDGE;

  auto params = glParamTrace.FindLastMatch("TexParameteri", minFilter);
  DALI_TEST_CHECK(params != nullptr);
  auto iter = params->find("param");
  DALI_TEST_EQUALS(iter->value.str(), LINEAR.str(), TEST_LOCATION);

  TraceCallStack::NamedParams magFilter;
  magFilter["pname"] << std::hex << GL_TEXTURE_MAG_FILTER;
  params = glParamTrace.FindLastMatch("TexParameteri", magFilter);
  DALI_TEST_CHECK(params != nullptr);
  iter = params->find("param");
  DALI_TEST_EQUALS(iter->value.str(), LINEAR.str(), TEST_LOCATION);

  TraceCallStack::NamedParams wrapS;
  wrapS["pname"] << std::hex << GL_TEXTURE_WRAP_S;
  params = glParamTrace.FindLastMatch("TexParameteri", wrapS);
  DALI_TEST_CHECK(params != nullptr);
  iter = params->find("param");
  DALI_TEST_EQUALS(iter->value.str(), CLAMP_TO_EDGE.str(), TEST_LOCATION);

  TraceCallStack::NamedParams wrapT;
  wrapT["pname"] << std::hex << GL_TEXTURE_WRAP_T;
  params = glParamTrace.FindLastMatch("TexParameteri", wrapT);
  DALI_TEST_CHECK(params != nullptr);
  iter = params->find("param");
  DALI_TEST_EQUALS(iter->value.str(), CLAMP_TO_EDGE.str(), TEST_LOCATION);

  END_TEST;
}

int UtcDaliGraphicsSamplerAllSet(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliSampler01 - check defaults");

  Texture    texture    = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);
  Actor      actor      = CreateRenderableActor(texture, "myVertShaderSource", "myFragShaderSource");
  TextureSet textureSet = actor.GetRendererAt(0u).GetTextures();

  Sampler sampler = Sampler::New();
  sampler.SetFilterMode(FilterMode::LINEAR_MIPMAP_NEAREST, FilterMode::NEAREST);
  sampler.SetWrapMode(WrapMode::REPEAT, WrapMode::MIRRORED_REPEAT);
  textureSet.SetSampler(0, sampler);

  app.GetScene().Add(actor);

  auto& gl           = app.GetGlAbstraction();
  auto& glParamTrace = gl.GetTexParameterTrace();
  glParamTrace.Enable(true);
  glParamTrace.EnableLogging(true);

  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  TraceCallStack::NamedParams minFilter;
  minFilter["pname"] << std::hex << GL_TEXTURE_MIN_FILTER;

  auto params = glParamTrace.FindLastMatch("TexParameteri", minFilter);
  DALI_TEST_CHECK(params != nullptr);
  auto iter = params->find("param");

  DALI_TEST_CHECK(*iter == GL_LINEAR_MIPMAP_NEAREST);

  TraceCallStack::NamedParams magFilter;
  magFilter["pname"] << std::hex << GL_TEXTURE_MAG_FILTER;
  params = glParamTrace.FindLastMatch("TexParameteri", magFilter);
  DALI_TEST_CHECK(params != nullptr);
  iter = params->find("param");
  DALI_TEST_CHECK(*iter == GL_NEAREST);

  TraceCallStack::NamedParams wrapS;
  wrapS["pname"] << std::hex << GL_TEXTURE_WRAP_S;
  params = glParamTrace.FindLastMatch("TexParameteri", wrapS);
  DALI_TEST_CHECK(params != nullptr);
  iter = params->find("param");
  DALI_TEST_CHECK(*iter == GL_REPEAT);

  TraceCallStack::NamedParams wrapT;
  wrapT["pname"] << std::hex << GL_TEXTURE_WRAP_T;
  params = glParamTrace.FindLastMatch("TexParameteri", wrapT);
  DALI_TEST_CHECK(params != nullptr);
  iter = params->find("param");
  DALI_TEST_CHECK(*iter == GL_MIRRORED_REPEAT);

  sampler.SetFilterMode(FilterMode::NEAREST_MIPMAP_LINEAR, FilterMode::LINEAR);
  sampler.SetWrapMode(WrapMode::CLAMP_TO_EDGE, WrapMode::DEFAULT);

  glParamTrace.Reset();
  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  params = glParamTrace.FindLastMatch("TexParameteri", minFilter);
  DALI_TEST_CHECK(params != nullptr);
  iter = params->find("param");
  DALI_TEST_CHECK(*iter == GL_NEAREST_MIPMAP_LINEAR);

  END_TEST;
}
