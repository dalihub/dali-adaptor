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
 */

#include <dali-test-suite-utils.h>
#include <dali/dali.h>

#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <test-actor-utils.h>
#include <test-graphics-application.h>
#include <test-graphics-sampler.h>

using namespace Dali;

void utc_dali_program_startup(void)
{
  test_return_value = TET_UNDEF;
}
void utc_dali_program_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
const std::string VERT_SHADER_SOURCE = "myVertShaderSource";
const std::string FRAG_SHADER_SOURCE =
  "\n"
  "uniform sampler2D sAlbedo;\n"
  "uniform sampler2D sMetalRoughness;\n"
  "uniform sampler2D sNormals;\n"
  "uniform sampler2D sAmbientOcclusion;\n"
  "uniform mediump vec3 lightDirection;\n"
  "in mediump vec2 vTexCoord;\n"
  "main()\n"
  "{\n"
  "  gl_fragColor = texture2d(sAlbedo, vTexCoord) + lightDirection*texture2d(sNormals, vTexCoord);\n"
  "}\n";
} //anonymous namespace

int UtcDaliGraphicsProgram(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliProgram - check that right sampler uniforms are bound for textures");

  Texture normals        = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);
  Texture metalroughness = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);
  Texture ao             = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);
  Texture albedo         = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);

  TextureSet textureSet = TextureSet::New();
  textureSet.SetTexture(0, albedo);
  textureSet.SetTexture(1, metalroughness);
  textureSet.SetTexture(2, normals);
  textureSet.SetTexture(3, ao);

  Actor actor = CreateRenderableActor2(textureSet, VERT_SHADER_SOURCE, FRAG_SHADER_SOURCE);
  app.GetScene().Add(actor);

  auto& gl             = app.GetGlAbstraction();
  auto& glUniformTrace = gl.GetSetUniformTrace();
  glUniformTrace.Enable(true);
  glUniformTrace.EnableLogging(true);

  std::vector<ActiveUniform> activeUniforms{
    {"sAlbedo", GL_SAMPLER_2D, 1},
    {"sAmbientOcclusion", GL_SAMPLER_2D, 1},
    {"sNormals", GL_SAMPLER_2D, 1},
    {"sMetalRoughness", GL_SAMPLER_2D, 1}};
  gl.SetActiveUniforms(activeUniforms);

  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  // Check what uniform values were set:
  int value;
  DALI_TEST_CHECK(gl.GetUniformValue("sAlbedo", value)); // First in frag shader
  DALI_TEST_EQUALS(value, 0, TEST_LOCATION);
  DALI_TEST_CHECK(gl.GetUniformValue("sAmbientOcclusion", value)); // 4th
  DALI_TEST_EQUALS(value, 3, TEST_LOCATION);
  DALI_TEST_CHECK(gl.GetUniformValue("sNormals", value)); // 3rd
  DALI_TEST_EQUALS(value, 2, TEST_LOCATION);
  DALI_TEST_CHECK(gl.GetUniformValue("sMetalRoughness", value)); // 2nd
  DALI_TEST_EQUALS(value, 1, TEST_LOCATION);

  END_TEST;
}
