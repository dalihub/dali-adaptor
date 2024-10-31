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

#include <dali/dali.h>

#include <dali-test-suite-utils.h>
#include <dali/internal/graphics/common/shader-parser.h>
#include <fstream>

#ifndef TEST_RESOURCE_DIR
#define TEST_RESOURCE_DIR ""
#endif

using namespace Dali::Internal::ShaderParser;

static std::string LoadTextFile(std::string filename)
{
  std::ifstream t(filename);
  std::string   str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  return str;
}

static bool CompareFileWithString(std::string file1, std::string stringToCompare)
{
  std::ifstream infile1(file1);

  std::ostringstream sstr;
  sstr << infile1.rdbuf();
  auto s = sstr.str();

  auto result = (s == stringToCompare);
  if(!result)
  {
    tet_printf("%s\n", s.c_str());
    tet_printf("---\n%s\n", stringToCompare.c_str());
  }
  return result;
}

int UtcParseGLES2Shader(void)
{
  tet_infoline("UtcParseGLES2Shader - Tests parser output for generating GLES2");

  auto vertexShader   = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.vert");
  auto fragmentShader = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.frag");

  std::vector<std::string> outStrings;

  Internal::ShaderParser::ShaderParserInfo parseInfo{};
  parseInfo.vertexShaderCode            = &vertexShader;
  parseInfo.fragmentShaderCode          = &fragmentShader;
  parseInfo.vertexShaderLegacyVersion   = 0;
  parseInfo.fragmentShaderLegacyVersion = 0;
  parseInfo.language                    = Internal::ShaderParser::OutputLanguage::GLSL_100_ES; // We default to GLSL3
  parseInfo.outputVersion               = 0;

  Parse(parseInfo, outStrings);
  auto& outVertexShader   = outStrings[0];
  auto& outFragmentShader = outStrings[1];
  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view.vert.gles2", outVertexShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view.frag.gles2", outFragmentShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  END_TEST;
}

int UtcParseGLES2ShaderWithOutput(void)
{
  tet_infoline("UtcParseGLES2ShaderWithOutput - Tests parser output for generating GLES2");

  // Load fragment shader with gl_FragColor
  auto vertexShader   = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.vert");
  auto fragmentShader = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view-with-output.frag");

  std::vector<std::string> outStrings;

  Internal::ShaderParser::ShaderParserInfo parseInfo{};
  parseInfo.vertexShaderCode            = &vertexShader;
  parseInfo.fragmentShaderCode          = &fragmentShader;
  parseInfo.vertexShaderLegacyVersion   = 0;
  parseInfo.fragmentShaderLegacyVersion = 0;
  parseInfo.language                    = Internal::ShaderParser::OutputLanguage::GLSL_100_ES; // We default to GLSL3
  parseInfo.outputVersion               = 0;
  Parse(parseInfo, outStrings);

  auto& outVertexShader   = outStrings[0];
  auto& outFragmentShader = outStrings[1];

  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view.vert.gles2", outVertexShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view-with-output.frag.gles2", outFragmentShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  END_TEST;
}

int UtcParseGLES3Shader(void)
{
  tet_infoline("UtcParseGLES3Shader - Tests parser output for generating GLES3");

  auto vertexShader   = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.vert");
  auto fragmentShader = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.frag");

  std::vector<std::string> outStrings;

  Internal::ShaderParser::ShaderParserInfo parseInfo{};
  parseInfo.vertexShaderCode            = &vertexShader;
  parseInfo.fragmentShaderCode          = &fragmentShader;
  parseInfo.vertexShaderLegacyVersion   = 0;
  parseInfo.fragmentShaderLegacyVersion = 0;
  parseInfo.language                    = Internal::ShaderParser::OutputLanguage::GLSL_320_ES;
  parseInfo.outputVersion               = 0;
  Parse(parseInfo, outStrings);
  auto& outVertexShader   = outStrings[0];
  auto& outFragmentShader = outStrings[1];

  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view.vert.gles3", outVertexShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view.frag.gles3", outFragmentShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  END_TEST;
}

int UtcParseGLES3ShaderWithOutput(void)
{
  tet_infoline("UtcParseGLES3ShaderWithOutput - Tests parser output for generating GLES3 with OUTPUT in fragment shader");

  auto vertexShader   = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.vert");
  auto fragmentShader = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view-with-output.frag");

  std::vector<std::string> outStrings;

  Internal::ShaderParser::ShaderParserInfo parseInfo{};
  parseInfo.vertexShaderCode            = &vertexShader;
  parseInfo.fragmentShaderCode          = &fragmentShader;
  parseInfo.vertexShaderLegacyVersion   = 0;
  parseInfo.fragmentShaderLegacyVersion = 0;
  parseInfo.language                    = Internal::ShaderParser::OutputLanguage::GLSL_320_ES;
  parseInfo.outputVersion               = 0;
  Parse(parseInfo, outStrings);

  // save to compare

  {
    //std::ofstream outv(TEST_RESOURCE_DIR "/shaders/canvas-view.vert.gles3");
    //std::ofstream outf(TEST_RESOURCE_DIR "/shaders/canvas-view-with-output.frag.gles3");
    //outv << outVertexShader;
    //outf << outFragmentShader;
  }
  auto& outVertexShader   = outStrings[0];
  auto& outFragmentShader = outStrings[1];

  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view.vert.gles3", outVertexShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view-with-output.frag.gles3", outFragmentShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  END_TEST;
}

int UtcParseSPIRVShader(void)
{
  tet_infoline("UtcParseSPIRVShader - Tests parser output for generating GLES3");

  // TODO: this test should fail in future after modifying sampler keywords!

  auto vertexShader   = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.vert");
  auto fragmentShader = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.frag");

  std::vector<std::string> outStrings;

  Internal::ShaderParser::ShaderParserInfo parseInfo{};
  parseInfo.vertexShaderCode            = &vertexShader;
  parseInfo.fragmentShaderCode          = &fragmentShader;
  parseInfo.vertexShaderLegacyVersion   = 0;
  parseInfo.fragmentShaderLegacyVersion = 0;
  parseInfo.language                    = Internal::ShaderParser::OutputLanguage::SPIRV_GLSL;
  parseInfo.outputVersion               = 0;
  Parse(parseInfo, outStrings);

  // save to compare

  {
    //std::ofstream outv(TEST_RESOURCE_DIR "/shaders/canvas-view.vert.glsl-spirv");
    //std::ofstream outf(TEST_RESOURCE_DIR "/shaders/canvas-view.frag.glsl-spirv");
    //outv << outVertexShader;
    //outf << outFragmentShader;
  }
  auto& outVertexShader   = outStrings[0];
  auto& outFragmentShader = outStrings[1];

  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view.vert.glsl-spirv", outVertexShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view.frag.glsl-spirv", outFragmentShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  END_TEST;
}

int UtcParseSPIRVShaderWithOutput(void)
{
  tet_infoline("UtcParseSPIRVShaderWithOutput - Tests parser output for generating GLES3");

  // TODO: this test should fail in future after modifying sampler keywords!

  auto vertexShader   = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.vert");
  auto fragmentShader = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view-with-output.frag");

  std::vector<std::string> outStrings;

  Internal::ShaderParser::ShaderParserInfo parseInfo{};
  parseInfo.vertexShaderCode            = &vertexShader;
  parseInfo.fragmentShaderCode          = &fragmentShader;
  parseInfo.vertexShaderLegacyVersion   = 0;
  parseInfo.fragmentShaderLegacyVersion = 0;
  parseInfo.language                    = Internal::ShaderParser::OutputLanguage::SPIRV_GLSL;
  parseInfo.outputVersion               = 0;
  Parse(parseInfo, outStrings);

  // save to compare

  {
    //std::ofstream outv(TEST_RESOURCE_DIR "/shaders/canvas-view.vert.glsl-spirv");
    //std::ofstream outf(TEST_RESOURCE_DIR "/shaders/canvas-view-with-output.frag.glsl-spirv");
    //outv << outVertexShader;
    //outf << outFragmentShader;
  }
  auto& outVertexShader   = outStrings[0];
  auto& outFragmentShader = outStrings[1];

  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view.vert.glsl-spirv", outVertexShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/canvas-view-with-output.frag.glsl-spirv", outFragmentShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  END_TEST;
}

int UtcParseSPIRVShaderDuplicateUBO(void)
{
  tet_infoline("UtcParseSPIRVShaderDuplicateUBO - Tests binding for reused UBOs for Vulkan");

  // TODO: this test should fail in future after modifying sampler keywords!

  auto vertexShader   = LoadTextFile(TEST_RESOURCE_DIR "/shaders/ubo-reused.vert");
  auto fragmentShader = LoadTextFile(TEST_RESOURCE_DIR "/shaders/ubo-reused.frag");

  std::vector<std::string> outStrings;

  Internal::ShaderParser::ShaderParserInfo parseInfo{};
  parseInfo.vertexShaderCode            = &vertexShader;
  parseInfo.fragmentShaderCode          = &fragmentShader;
  parseInfo.vertexShaderLegacyVersion   = 0;
  parseInfo.fragmentShaderLegacyVersion = 0;
  parseInfo.language                    = Internal::ShaderParser::OutputLanguage::SPIRV_GLSL;
  parseInfo.outputVersion               = 0;
  Parse(parseInfo, outStrings);

  auto& outVertexShader   = outStrings[0];
  auto& outFragmentShader = outStrings[1];
  // save to compare
  /*
  {
    std::ofstream outv(TEST_RESOURCE_DIR "/shaders/tmp.vert.glsl-spirv");
    std::ofstream outf(TEST_RESOURCE_DIR "/shaders/tmp.frag.glsl-spirv");
    outv << outVertexShader;
    outf << outFragmentShader;
  }*/
  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/ubo-reused.vert.glsl-spirv", outVertexShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  {
    bool cmp = CompareFileWithString(TEST_RESOURCE_DIR "/shaders/ubo-reused.frag.glsl-spirv", outFragmentShader);
    DALI_TEST_EQUALS(cmp, true, TEST_LOCATION);
  }
  END_TEST;
}

int UtcParseShaderGLSLEnumValues(void)
{
  tet_infoline("UtcParseGLES3Shader - Tests parser output for generating GLES3");

  auto vertexShader   = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.vert");
  auto fragmentShader = LoadTextFile(TEST_RESOURCE_DIR "/shaders/canvas-view.frag");

  std::vector<std::string> outStrings;

  Internal::ShaderParser::ShaderParserInfo parseInfo{};
  parseInfo.vertexShaderCode            = &vertexShader;
  parseInfo.fragmentShaderCode          = &fragmentShader;
  parseInfo.vertexShaderLegacyVersion   = 0;
  parseInfo.fragmentShaderLegacyVersion = 0;
  parseInfo.outputVersion               = 0;

  std::vector<Internal::ShaderParser::OutputLanguage> values =
    {
      Internal::ShaderParser::OutputLanguage::GLSL_100_ES,
      Internal::ShaderParser::OutputLanguage::GLSL_300_ES,
      Internal::ShaderParser::OutputLanguage::GLSL_310_ES,
      Internal::ShaderParser::OutputLanguage::GLSL_320_ES};
  std::vector<std::string> expectedOutput = {
    "#version 100\n",
    "#version 300 es\n",
    "#version 310 es\n",
    "#version 320 es\n"};
  int i = 0;
  for(auto& v : values)
  {
    parseInfo.language = v;
    Parse(parseInfo, outStrings);
    auto& outVertexShader   = outStrings[0];
    auto& outFragmentShader = outStrings[1];

    auto vshResult = expectedOutput[i].compare(0, expectedOutput[i].length(), outVertexShader, 0, expectedOutput[i].length());
    auto fshResult = expectedOutput[i].compare(0, expectedOutput[i].length(), outFragmentShader, 0, expectedOutput[i].length());
    DALI_TEST_EQUALS(vshResult, 0, TEST_LOCATION);
    DALI_TEST_EQUALS(fshResult, 0, TEST_LOCATION);
    ++i;
  }
  END_TEST;
}
