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
#include <test-graphics-application.h>
#include <test-graphics-framebuffer.h>

using namespace Dali;

namespace
{
const char* VERTEX_SHADER = DALI_COMPOSE_SHADER(
  INPUT mediump vec2       aPos;\n
    INPUT mediump int      aCount;\n
      uniform mediump mat4 uMvpMatrix;\n
        OUTPUT flat int vCount;\n void main()\n {
          \n
            mediump vec4 vertexPosition(aPos, 0.0, 1.0);
          \n
            gl_Position = uMvpMatrix * vertexPosition;
          \n
        }\n);

const char* FRAGMENT_SHADER = DALI_COMPOSE_SHADER(
  uniform lowp vec4 uColor;\n
    INPUT flat int vCount;\n void main()\n {
      \n
        mediump float g = (128.0 + vCount * 16) / 255.0;
      \n
        gl_FragColor = uColor * g;
      \n
    }\n);

} // namespace

void utc_dali_texture_startup(void)
{
  test_return_value = TET_UNDEF;
}
void utc_dali_texture_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliGraphicsDrawIntegerVertexAttribs(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsDrawIntegerVertexAttribs - Test that integer vertex attribs use correct GL call");

  auto& gl          = app.GetGlAbstraction();
  auto& bufferTrace = gl.GetBufferTrace();
  bufferTrace.EnableLogging(true);
  bufferTrace.Enable(true);

  // Initalize GL shader reflection
  std::vector<std::string> aLocs  = {"aPos", "aCount"};
  std::vector<GLenum>      aTypes = {GL_FLOAT, GL_INT};
  gl.SetAttribLocations(aLocs);
  gl.SetAttribTypes(aTypes);

  TextureSet    textureSet = CreateTextureSet(Pixel::RGBA8888, 200, 200);
  Property::Map vertexFormat{{"aPos", Property::VECTOR2}, {"aCount", Property::INTEGER}};
  VertexBuffer  vertexBuffer = VertexBuffer::New(vertexFormat);

  struct VertexFormat
  {
    Vector2 aPos;
    int     aCount;
  };
  std::vector<VertexFormat> vertexData = {{Vector2{10, 20}, 1}, {Vector2{10, 20}, 2}, {Vector2{10, 20}, 3}, {Vector2{10, 20}, 4}};
  vertexBuffer.SetData(&vertexData[0], sizeof(vertexData) / sizeof(VertexFormat));
  Geometry geometry = Geometry::New();
  geometry.AddVertexBuffer(vertexBuffer);
  Shader   shader   = Shader::New(VERTEX_SHADER, FRAGMENT_SHADER);
  Renderer renderer = Renderer::New(geometry, shader);
  renderer.SetTextures(textureSet);
  Actor dummyActor                  = Actor::New();
  dummyActor[Actor::Property::SIZE] = Vector2(200, 200);
  dummyActor.AddRenderer(renderer);
  app.GetScene().Add(dummyActor);

  app.SendNotification();
  app.Render(16);

  tet_infoline("Test that we have both VertexAttribPointer and VertexAttribIPointer called");
  DALI_TEST_CHECK(bufferTrace.FindMethod("VertexAttribPointer"));
  DALI_TEST_CHECK(bufferTrace.FindMethod("VertexAttribIPointer"));

  END_TEST;
}
