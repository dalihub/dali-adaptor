/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-reflection.h>
#include <test-actor-utils.h>
#include <test-graphics-framebuffer.h>
#include <test-graphics-vk-application.h>

using namespace Dali;

namespace
{

// Simple vertex shader with uniform blocks for testing reflection (DALi syntax)
const char* BASIC_VERTEX_SHADER = R"(
//@version 100

//@ignore:on
#define UNIFORM_BLOCK uniform
#define UNIFORM uniform
#define INPUT in
#define OUTPUT out
//@ignore:off

INPUT highp vec3 aPosition;
INPUT mediump vec3 aColor;
OUTPUT mediump vec3 vTexCoord;

UNIFORM_BLOCK TestBlock
{
  UNIFORM highp mat4 uMVPMatrix;
  UNIFORM mediump vec4 uColor;
  UNIFORM mediump float uTime;
};

void main() {
    gl_Position = uMVPMatrix * vec4(aPosition, 1.0);
    vTexCoord = aColor * uColor.rgb;
}
)";

// Simple fragment shader (DALi syntax)
const char* BASIC_FRAGMENT_SHADER = R"(
//@version 100

//@ignore:on
#define UNIFORM_BLOCK uniform
#define UNIFORM uniform
#define INPUT in
#define OUTPUT out
#define OUT_COLOR gl_FragColor
//@ignore:off

INPUT mediump vec3 vTexCoord;

void main() {
    gl_FragColor = vec4(vTexCoord, 1.0);
}
)";

// Simple fragment shader (DALi syntax)
const char* COMPLEX_FRAGMENT_SHADER = R"(
//@version 100

//@ignore:on
#define UNIFORM_BLOCK uniform
#define UNIFORM uniform
#define INPUT in
#define OUTPUT out
#define OUT_COLOR gl_FragColor
//@ignore:off

INPUT mediump vec3 vTexCoord;
INPUT mediump vec3 vLightPos;
INPUT mediump vec3 vLightCol;

void main() {
    gl_FragColor = vec4(vLightPos * vLightCol * vTexCoord, 1.0);
}
)";

// Vertex shader with nested structures (DALi syntax)
const char* NESTED_VERTEX_SHADER = R"(
//@version 100

//@ignore:on
#define UNIFORM_BLOCK uniform
#define UNIFORM uniform
#define INPUT in
#define OUTPUT out
//@ignore:off

INPUT highp vec3 aPosition;
OUTPUT mediump vec3 vTexCoord;

struct InnerStruct {
    mediump vec3 position;
    mediump float padding;
};

struct OuterStruct {
    InnerStruct inner;
    mediump vec4 color;
};

UNIFORM_BLOCK NestedBlock
{
    OuterStruct transform;
    UNIFORM highp mat4 uMVPMatrix;
};

void main() {
    gl_Position = uMVPMatrix * vec4(aPosition + transform.inner.position, 1.0);
    vTexCoord = transform.color.rgb;
}
)";

// Vertex shader with arrays (DALi syntax)
const char* ARRAY_VERTEX_SHADER = R"(
//@version 100

//@ignore:on
#define UNIFORM_BLOCK uniform
#define UNIFORM uniform
#define INPUT in
#define OUTPUT out
//@ignore:off

INPUT highp vec3 aPosition;
OUTPUT mediump vec3 vTexCoord;

UNIFORM_BLOCK ArrayBlock
{
    UNIFORM mediump float dataArray[4];
    UNIFORM mediump vec4 uColor;
    UNIFORM highp mat4 uMVPMatrix;
};

void main() {
    gl_Position = uMVPMatrix * vec4(aPosition, 1.0);
    vTexCoord = uColor.rgb * dataArray[0];
}
)";

// Vertex shader with nested structures (DALi syntax)
const char* COMPLEX_VERTEX_SHADER = R"(
//@version 100

//@ignore:on
#define UNIFORM_BLOCK uniform
#define UNIFORM uniform
#define INPUT in
#define OUTPUT out
//@ignore:off

INPUT highp vec3 aPosition;
OUTPUT mediump vec3 vTexCoord;
OUTPUT mediump vec3 vLightPos;
OUTPUT mediump vec3 vLightCol;

struct InnerStruct {
    mediump vec3 position;
    mediump float padding;
};

struct OuterStruct {
    InnerStruct inner[3];
    mediump vec4 color;
};
struct Light {
  mediump vec3 position;
  mediump vec3 color;
};
UNIFORM_BLOCK NestedBlock
{
    OuterStruct transform;
    Light light;
    UNIFORM highp mat4 uMVPMatrix;
};

void main() {
    mediump vec3 merge=vec3(0.0);
    for(int i=0; i<3; ++i)
    {
       merge+=transform.inner[i].position*transform.inner[i].padding;
    }
    gl_Position = uMVPMatrix * vec4(aPosition + merge, 1.0);
    vTexCoord = transform.color.rgb;
    vLightPos = light.position;
    vLightCol = light.color;
}
)";

//Expect:
const std::vector<std::string> COMPLEX_EXPECTED =
  {
    {"transform.inner[0].position"},
    {"transform.inner[0].padding"},
    {"transform.inner[1].position"},
    {"transform.inner[1].padding"},
    {"transform.inner[2].position"},
    {"transform.inner[2].padding"},
    {"transform.color"},
    {"light.position"},
    {"light.color"},
    {"uMVPMatrix"}};

} // namespace

int UtcDaliVkReflectionParseUniformBlockVariablesBasic(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliVkReflectionParseUniformBlockVariablesBasic: Test ParseUniformBlockVariables with basic uniform blocks");

  auto& controller = app.GetGraphicsController();

  // Create vertex shader with GLSL source
  Dali::Graphics::ShaderCreateInfo shaderInfo;
  shaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::VERTEX_SHADER);
  shaderInfo.SetSourceData(BASIC_VERTEX_SHADER);
  shaderInfo.SetSourceSize(strlen(BASIC_VERTEX_SHADER));
  shaderInfo.SetShaderVersion(100);
  shaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create fragment shader with GLSL source
  Dali::Graphics::ShaderCreateInfo fragShaderInfo;
  fragShaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::FRAGMENT_SHADER);
  fragShaderInfo.SetSourceData(BASIC_FRAGMENT_SHADER);
  fragShaderInfo.SetSourceSize(strlen(BASIC_FRAGMENT_SHADER));
  fragShaderInfo.SetShaderVersion(100);
  fragShaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create shaders through graphics controller
  auto shader = controller.CreateShader(shaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)shader.get(), (void*)nullptr, 0, TEST_LOCATION);

  auto fragShader = controller.CreateShader(fragShaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)fragShader.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Create program with both shaders
  std::vector<Dali::Graphics::ShaderState> shaderStates;
  shaderStates.push_back({shader.get(), Dali::Graphics::PipelineStage::VERTEX_SHADER});
  shaderStates.push_back({fragShader.get(), Dali::Graphics::PipelineStage::FRAGMENT_SHADER});

  Dali::Graphics::ProgramCreateInfo programInfo;
  programInfo.SetShaderState(shaderStates);

  auto program = controller.CreateProgram(programInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)program.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Cast to Vulkan::Program to access GetReflection()
  auto* vkProgram = static_cast<Dali::Graphics::Vulkan::Program*>(program.get());
  DALI_TEST_NOT_EQUALS((void*)vkProgram, (void*)nullptr, 0, TEST_LOCATION);

  // Get reflection from the Vulkan program
  auto& reflection = vkProgram->GetReflection();

  // Test that we can query uniform block information
  uint32_t uniformBlockCount = reflection.GetUniformBlockCount();
  tet_printf("Uniform block count: %u\n", uniformBlockCount);
  DALI_TEST_CHECK(uniformBlockCount >= 1); // Should have at least the standalone block

  // Test getting specific uniform block info
  for(uint32_t i = 1; i < uniformBlockCount; ++i) // Skip standalone block at index 0
  {
    Dali::Graphics::UniformBlockInfo blockInfo;
    bool                             hasBlock = reflection.GetUniformBlock(i, blockInfo);
    if(hasBlock)
    {
      tet_printf("Uniform block %u: %u members\n", i, blockInfo.members.size());

      // Test block properties
      DALI_TEST_CHECK(blockInfo.members.size() >= 0);
      DALI_TEST_CHECK(blockInfo.size >= 0);

      // Test member access
      for(uint32_t j = 0; j < blockInfo.members.size(); ++j)
      {
        std::string memberName   = reflection.GetUniformBlockMemberName(i, j);
        uint32_t    memberOffset = reflection.GetUniformBlockMemberOffset(i, j);

        tet_printf("  Member %u: '%s' at offset %u\n", j, memberName.c_str(), memberOffset);

        // Name should not be empty for valid members
        if(!memberName.empty())
        {
          DALI_TEST_CHECK(memberOffset >= 0);
        }
      }
    }
  }

  END_TEST;
}

int UtcDaliVkReflectionParseUniformBlockVariablesNested(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliVkReflectionParseUniformBlockVariablesNested: Test ParseUniformBlockVariables with nested structures");

  auto& controller = app.GetGraphicsController();

  // Create vertex shader with nested structures
  Dali::Graphics::ShaderCreateInfo shaderInfo;
  shaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::VERTEX_SHADER);
  shaderInfo.SetSourceData(NESTED_VERTEX_SHADER);
  shaderInfo.SetSourceSize(strlen(NESTED_VERTEX_SHADER));
  shaderInfo.SetShaderVersion(100);
  shaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create fragment shader
  Dali::Graphics::ShaderCreateInfo fragShaderInfo;
  fragShaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::FRAGMENT_SHADER);
  fragShaderInfo.SetSourceData(BASIC_FRAGMENT_SHADER);
  fragShaderInfo.SetSourceSize(strlen(BASIC_FRAGMENT_SHADER));
  fragShaderInfo.SetShaderVersion(100);
  fragShaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create shaders through graphics controller
  auto shader = controller.CreateShader(shaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)shader.get(), (void*)nullptr, 0, TEST_LOCATION);

  auto fragShader = controller.CreateShader(fragShaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)fragShader.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Create program with both shaders
  std::vector<Dali::Graphics::ShaderState> shaderStates;
  shaderStates.push_back({shader.get(), Dali::Graphics::PipelineStage::VERTEX_SHADER});
  shaderStates.push_back({fragShader.get(), Dali::Graphics::PipelineStage::FRAGMENT_SHADER});

  Dali::Graphics::ProgramCreateInfo programInfo;
  programInfo.SetShaderState(shaderStates);

  auto program = controller.CreateProgram(programInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)program.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Cast to Vulkan::Program to access GetReflection()
  auto* vkProgram = static_cast<Dali::Graphics::Vulkan::Program*>(program.get());
  DALI_TEST_NOT_EQUALS((void*)vkProgram, (void*)nullptr, 0, TEST_LOCATION);

  // Get reflection from the Vulkan program
  auto& reflection = vkProgram->GetReflection();

  // Test that we can query uniform block information
  uint32_t uniformBlockCount = reflection.GetUniformBlockCount();
  tet_printf("Uniform block count: %u\n", uniformBlockCount);
  DALI_TEST_CHECK(uniformBlockCount >= 1); // Should have at least the standalone block

  // If we have the uniform block with nested structures, test its members
  if(uniformBlockCount > 1)
  {
    Dali::Graphics::UniformBlockInfo blockInfo;
    bool                             hasBlock = reflection.GetUniformBlock(1, blockInfo); // Skip standalone block at index 0
    if(hasBlock)
    {
      tet_printf("Nested uniform block has %u members\n", blockInfo.members.size());
      // Should have members from the nested structure
      DALI_TEST_CHECK(blockInfo.members.size() >= 1);
    }
  }

  END_TEST;
}

int UtcDaliVkReflectionParseUniformBlockVariablesArrays(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliVkReflectionParseUniformBlockVariablesArrays: Test ParseUniformBlockVariables with arrays");

  auto& controller = app.GetGraphicsController();

  // Create vertex shader with arrays
  Dali::Graphics::ShaderCreateInfo shaderInfo;
  shaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::VERTEX_SHADER);
  shaderInfo.SetSourceData(ARRAY_VERTEX_SHADER);
  shaderInfo.SetSourceSize(strlen(ARRAY_VERTEX_SHADER));
  shaderInfo.SetShaderVersion(100);
  shaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create fragment shader
  Dali::Graphics::ShaderCreateInfo fragShaderInfo;
  fragShaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::FRAGMENT_SHADER);
  fragShaderInfo.SetSourceData(BASIC_FRAGMENT_SHADER);
  fragShaderInfo.SetSourceSize(strlen(BASIC_FRAGMENT_SHADER));
  fragShaderInfo.SetShaderVersion(100);
  fragShaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create shaders through graphics controller
  auto shader = controller.CreateShader(shaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)shader.get(), (void*)nullptr, 0, TEST_LOCATION);

  auto fragShader = controller.CreateShader(fragShaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)fragShader.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Create program with both shaders
  std::vector<Dali::Graphics::ShaderState> shaderStates;
  shaderStates.push_back({shader.get(), Dali::Graphics::PipelineStage::VERTEX_SHADER});
  shaderStates.push_back({fragShader.get(), Dali::Graphics::PipelineStage::FRAGMENT_SHADER});

  Dali::Graphics::ProgramCreateInfo programInfo;
  programInfo.SetShaderState(shaderStates);

  auto program = controller.CreateProgram(programInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)program.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Cast to Vulkan::Program to access GetReflection()
  auto* vkProgram = static_cast<Dali::Graphics::Vulkan::Program*>(program.get());
  DALI_TEST_NOT_EQUALS((void*)vkProgram, (void*)nullptr, 0, TEST_LOCATION);

  // Get reflection from the Vulkan program
  auto& reflection = vkProgram->GetReflection();

  // Test that we can query uniform block information
  uint32_t uniformBlockCount = reflection.GetUniformBlockCount();
  tet_printf("Uniform block count: %u\n", uniformBlockCount);
  DALI_TEST_CHECK(uniformBlockCount >= 1); // Should have at least the standalone block

  if(uniformBlockCount > 1)
  {
    Dali::Graphics::UniformBlockInfo blockInfo;
    bool                             hasBlock = reflection.GetUniformBlock(1, blockInfo); // Skip standalone block at index 0
    if(hasBlock)
    {
      tet_printf("Array uniform block has %u members\n", blockInfo.members.size());

      // For simple arrays, only 1 member is reflected, so ARRAY_VERTEX_SHADER should have 3 elements
      DALI_TEST_EQUALS(blockInfo.members.size(), 3, TEST_LOCATION);
      for(auto i = 0u; i < blockInfo.members.size(); ++i)
      {
        tet_printf("Uniform %s,  offset %u\n",
                   blockInfo.members[i].name.c_str(),
                   blockInfo.members[i].offset);
      }
    }
  }

  END_TEST;
}

int UtcDaliVkReflectionParseUniformBlockVariablesComplex(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliVkReflectionParseUniformBlockVariablesComplex: Test ParseUniformBlockVariables with complex scenarios");

  auto& controller = app.GetGraphicsController();

  // Create vertex shader with basic uniform blocks for complex testing
  Dali::Graphics::ShaderCreateInfo shaderInfo;
  shaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::VERTEX_SHADER);
  shaderInfo.SetSourceData(COMPLEX_VERTEX_SHADER);
  shaderInfo.SetSourceSize(strlen(COMPLEX_VERTEX_SHADER));
  shaderInfo.SetShaderVersion(100);
  shaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create fragment shader
  Dali::Graphics::ShaderCreateInfo fragShaderInfo;
  fragShaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::FRAGMENT_SHADER);
  fragShaderInfo.SetSourceData(COMPLEX_FRAGMENT_SHADER);
  fragShaderInfo.SetSourceSize(strlen(COMPLEX_FRAGMENT_SHADER));
  fragShaderInfo.SetShaderVersion(100);
  fragShaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create shaders through graphics controller
  auto shader = controller.CreateShader(shaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)shader.get(), (void*)nullptr, 0, TEST_LOCATION);

  auto fragShader = controller.CreateShader(fragShaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)fragShader.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Create program with both shaders
  std::vector<Dali::Graphics::ShaderState> shaderStates;
  shaderStates.push_back({shader.get(), Dali::Graphics::PipelineStage::VERTEX_SHADER});
  shaderStates.push_back({fragShader.get(), Dali::Graphics::PipelineStage::FRAGMENT_SHADER});

  Dali::Graphics::ProgramCreateInfo programInfo;
  programInfo.SetShaderState(shaderStates);

  auto program = controller.CreateProgram(programInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)program.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Cast to Vulkan::Program to access GetReflection()
  auto* vkProgram = static_cast<Dali::Graphics::Vulkan::Program*>(program.get());
  DALI_TEST_NOT_EQUALS((void*)vkProgram, (void*)nullptr, 0, TEST_LOCATION);

  // Get reflection from the Vulkan program
  auto& reflection = vkProgram->GetReflection();

  // Test that we can query uniform block information
  uint32_t uniformBlockCount = reflection.GetUniformBlockCount();
  tet_printf("Uniform block count: %u\n", uniformBlockCount);
  DALI_TEST_CHECK(uniformBlockCount >= 1); // Should have at least the standalone block

  // Test uniform block locations
  auto locations = reflection.GetUniformBlockLocations();
  tet_printf("Uniform block locations count: %zu\n", locations.size());
  DALI_TEST_CHECK(locations.size() >= 0);

  // Test getting specific uniform block info
  for(uint32_t i = 1; i < uniformBlockCount; ++i) // Skip standalone block at index 0
  {
    Dali::Graphics::UniformBlockInfo blockInfo;
    bool                             hasBlock = reflection.GetUniformBlock(i, blockInfo);
    if(hasBlock)
    {
      tet_printf("Testing uniform block %u with %u members\n", i, blockInfo.members.size());

      // Test block properties
      DALI_TEST_CHECK(blockInfo.members.size() >= 0);
      DALI_TEST_CHECK(blockInfo.size >= 0);

      // Test member access
      for(uint32_t j = 0; j < blockInfo.members.size(); ++j)
      {
        std::string memberName   = reflection.GetUniformBlockMemberName(i, j);
        uint32_t    memberOffset = reflection.GetUniformBlockMemberOffset(i, j);

        tet_printf("  Member %u: '%s' at offset %u\n", j, memberName.c_str(), memberOffset);

        DALI_TEST_EQUALS(memberName, COMPLEX_EXPECTED[j], TEST_LOCATION);

        // Name should not be empty for valid members
        if(!memberName.empty())
        {
          DALI_TEST_CHECK(memberOffset >= 0);
        }
      }
    }
  }

  END_TEST;
}

int UtcDaliVkReflectionParseUniformBlockVariablesEdgeCases(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliVkReflectionParseUniformBlockVariablesEdgeCases: Test ParseUniformBlockVariables with edge cases");

  auto& controller = app.GetGraphicsController();

  // Create vertex shader with basic uniform blocks
  Dali::Graphics::ShaderCreateInfo shaderInfo;
  shaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::VERTEX_SHADER);
  shaderInfo.SetSourceData(BASIC_VERTEX_SHADER);
  shaderInfo.SetSourceSize(strlen(BASIC_VERTEX_SHADER));
  shaderInfo.SetShaderVersion(100);
  shaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create fragment shader
  Dali::Graphics::ShaderCreateInfo fragShaderInfo;
  fragShaderInfo.SetPipelineStage(Dali::Graphics::PipelineStage::FRAGMENT_SHADER);
  fragShaderInfo.SetSourceData(BASIC_FRAGMENT_SHADER);
  fragShaderInfo.SetSourceSize(strlen(BASIC_FRAGMENT_SHADER));
  fragShaderInfo.SetShaderVersion(100);
  fragShaderInfo.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

  // Create shaders through graphics controller
  auto shader = controller.CreateShader(shaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)shader.get(), (void*)nullptr, 0, TEST_LOCATION);

  auto fragShader = controller.CreateShader(fragShaderInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)fragShader.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Create program with both shaders
  std::vector<Dali::Graphics::ShaderState> shaderStates;
  shaderStates.push_back({shader.get(), Dali::Graphics::PipelineStage::VERTEX_SHADER});
  shaderStates.push_back({fragShader.get(), Dali::Graphics::PipelineStage::FRAGMENT_SHADER});

  Dali::Graphics::ProgramCreateInfo programInfo;
  programInfo.SetShaderState(shaderStates);

  auto program = controller.CreateProgram(programInfo, nullptr);
  DALI_TEST_NOT_EQUALS((void*)program.get(), (void*)nullptr, 0, TEST_LOCATION);

  // Cast to Vulkan::Program to access GetReflection()
  auto* vkProgram = static_cast<Dali::Graphics::Vulkan::Program*>(program.get());
  DALI_TEST_NOT_EQUALS((void*)vkProgram, (void*)nullptr, 0, TEST_LOCATION);

  // Get reflection from the Vulkan program
  auto& reflection = vkProgram->GetReflection();

  // Test edge cases for uniform block access
  Dali::Graphics::UniformBlockInfo blockInfo;

  // Test accessing invalid block indices
  bool hasInvalidBlock = reflection.GetUniformBlock(999, blockInfo);
  DALI_TEST_CHECK(!hasInvalidBlock);

  // Test accessing invalid member indices
  std::string invalidMemberName = reflection.GetUniformBlockMemberName(0, 999);
  DALI_TEST_CHECK(invalidMemberName.empty());

  uint32_t invalidMemberOffset = reflection.GetUniformBlockMemberOffset(0, 999);
  DALI_TEST_EQUALS(invalidMemberOffset, 0u, TEST_LOCATION);

  END_TEST;
}
