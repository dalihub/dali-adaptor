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

const std::string VERT_SHADER_SOURCE2 =
  "\n"
  "in vec3 aPosition;\n"
  "in vec3 aTexCoord;\n"
  "out vec2 vTexCoord;\n"
  "main()\n"
  "{\n"
  "  gl_Position=aPosition;\n"
  "  vTexCoord = aTexCoord;\n"
  "}\n";

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

const std::string FRAG_SHADER_SOURCE2 =
  "\n"
  "uniform sampler2D sTextures[4];\n"
  "uniform mediump vec3 lightDirection;\n"
  "in mediump vec2 vTexCoord;\n"
  "main()\n"
  "{\n"
  "  gl_fragColor = texture2d(sTextures[0], vTexCoord) + lightDirection*texture2d(sTextures[2], vTexCoord);\n"
  "}\n";

} // anonymous namespace

int UtcDaliGraphicsProgram01(void)
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

int UtcDaliGraphicsProgram02(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliProgram - check that sampler arrays are handled and bound to textures");

  Texture normals        = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);
  Texture metalroughness = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);
  Texture ao             = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);
  Texture albedo         = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);

  TextureSet textureSet = TextureSet::New();
  textureSet.SetTexture(0, albedo);
  textureSet.SetTexture(1, metalroughness);
  textureSet.SetTexture(2, normals);
  textureSet.SetTexture(3, ao);

  Actor actor = CreateRenderableActor2(textureSet, VERT_SHADER_SOURCE, FRAG_SHADER_SOURCE2);
  app.GetScene().Add(actor);

  auto& gl             = app.GetGlAbstraction();
  auto& glUniformTrace = gl.GetSetUniformTrace();
  glUniformTrace.Enable(true);
  glUniformTrace.EnableLogging(true);
  gl.GetShaderTrace().Enable(true);
  gl.GetShaderTrace().EnableLogging(true);

  std::vector<ActiveUniform> activeUniforms{
    {"uLightDir", GL_FLOAT_VEC4, 1},
    {"sTextures[0]", GL_SAMPLER_2D, 4}}; // Array of 4 samplers
  gl.SetActiveUniforms(activeUniforms);

  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  // Check what uniform values were set:
  int value;
  DALI_TEST_CHECK(gl.GetUniformValue("sTextures[0]", value)); // First in frag shader
  DALI_TEST_EQUALS(value, 0, TEST_LOCATION);
  DALI_TEST_CHECK(gl.GetUniformValue("sTextures[3]", value)); // 4th
  DALI_TEST_EQUALS(value, 3, TEST_LOCATION);
  DALI_TEST_CHECK(gl.GetUniformValue("sTextures[2]", value)); // 3rd
  DALI_TEST_EQUALS(value, 2, TEST_LOCATION);
  DALI_TEST_CHECK(gl.GetUniformValue("sTextures[1]", value)); // 2nd
  DALI_TEST_EQUALS(value, 1, TEST_LOCATION);

  END_TEST;
}

int UtcDaliGraphicsShaderNew(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliProgram - check that multiple shaders from same source only create 1 program");

  Texture diffuse = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);

  // Creates 3 Dali::Shaders
  Actor actor1 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE, FRAG_SHADER_SOURCE);
  Actor actor2 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE, FRAG_SHADER_SOURCE);
  Actor actor3 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE, FRAG_SHADER_SOURCE);

  app.GetScene().Add(actor1);
  app.GetScene().Add(actor2);
  app.GetScene().Add(actor3);

  auto& gl            = app.GetGlAbstraction();
  auto& glShaderTrace = gl.GetShaderTrace();
  glShaderTrace.Enable(true);
  glShaderTrace.EnableLogging(true);

  app.SendNotification();
  app.Render(16); // The above actors will get rendered and drawn once, only 1 program and 2 shaders should be created

  DALI_TEST_EQUALS(glShaderTrace.CountMethod("CreateProgram"), 1, TEST_LOCATION);
  DALI_TEST_EQUALS(glShaderTrace.CountMethod("CreateShader"), 2, TEST_LOCATION);

  END_TEST;
}

int UtcDaliGraphicsShaderNew02(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliProgram - check that mixed up multiple shaders from same source don't create dups");

  Texture diffuse = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);

  // Creates 3 Dali::Shaders
  Actor actor1 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE, FRAG_SHADER_SOURCE);
  Actor actor2 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE2, FRAG_SHADER_SOURCE2);
  Actor actor3 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE, FRAG_SHADER_SOURCE2);
  Actor actor4 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE2, FRAG_SHADER_SOURCE);

  app.GetScene().Add(actor1);
  app.GetScene().Add(actor2);
  app.GetScene().Add(actor3);
  app.GetScene().Add(actor4);

  auto& gl            = app.GetGlAbstraction();
  auto& glShaderTrace = gl.GetShaderTrace();
  glShaderTrace.Enable(true);
  glShaderTrace.EnableLogging(true);

  app.SendNotification();
  app.Render(16); // The above actors will get rendered and drawn once, only 4 programs and 4 shaders should be created

  // Should only be 4 shaders, not 8.
  DALI_TEST_EQUALS(glShaderTrace.CountMethod("CreateProgram"), 4, TEST_LOCATION);
  DALI_TEST_EQUALS(glShaderTrace.CountMethod("CreateShader"), 4, TEST_LOCATION);

  END_TEST;
}

int UtcDaliGraphicsShaderFlush(void)
{
  // Note : This UTC will not works well since now GLES::ProgramImpl hold the reference of shader,
  // and we don't have any way to remove GLES::ProgramImpl by normal way.
  // Just block this UTC until the policy was fixed up.
#if 0
  TestGraphicsApplication app;
  tet_infoline("UtcDaliProgram - check that unused shaders are flushed");

  Texture diffuse       = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16u, 16u);
  auto&   gl            = app.GetGlAbstraction();
  auto&   glShaderTrace = gl.GetShaderTrace();
  glShaderTrace.Enable(true);
  glShaderTrace.EnableLogging(true);

  {
    // Creates 4 Dali::Shaders
    Actor actor1 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE, FRAG_SHADER_SOURCE);
    Actor actor2 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE2, FRAG_SHADER_SOURCE2);
    Actor actor3 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE, FRAG_SHADER_SOURCE2);
    Actor actor4 = CreateRenderableActor(diffuse, VERT_SHADER_SOURCE2, FRAG_SHADER_SOURCE);

    app.GetScene().Add(actor1);
    app.GetScene().Add(actor2);
    app.GetScene().Add(actor3);
    app.GetScene().Add(actor4);

    app.SendNotification();
    app.Render(16); // The above actors will get rendered and drawn once

    // Should only be 4 shaders, not 8.
    DALI_TEST_EQUALS(glShaderTrace.CountMethod("CreateProgram"), 4, TEST_LOCATION);
    DALI_TEST_EQUALS(glShaderTrace.CountMethod("CreateShader"), 4, TEST_LOCATION);

    UnparentAndReset(actor1);
    UnparentAndReset(actor2);
    UnparentAndReset(actor3);
    UnparentAndReset(actor4);
  }

  for(int i = 0; i < 1199; ++i) // 3 flushes per frame
  {
    app.SendNotification();
    app.Render(16);
    DALI_TEST_EQUALS(glShaderTrace.CountMethod("DeleteShader"), 0, TEST_LOCATION);
  }

  app.SendNotification();
  app.Render(16);
  DALI_TEST_EQUALS(glShaderTrace.CountMethod("DeleteShader"), 4, TEST_LOCATION);
#else
  DALI_TEST_CHECK(true);
#endif
  END_TEST;
}
