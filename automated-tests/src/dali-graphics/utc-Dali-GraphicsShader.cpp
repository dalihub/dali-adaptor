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
*
*/

#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/integration-api/testing.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-shader.h>
#include <test-graphics-application.h>

int UtcDaliGlesStripLegacyCodeIfNeededTest1(void)
{
  TestGraphicsApplication application;

  {
    Dali::Graphics::ShaderCreateInfo info;
    info.SetPipelineStage(Dali::Graphics::PipelineStage::VERTEX_SHADER);
    std::string vertexShader =
      "//@version 100\n"
      "some code\n";

    info.SetShaderVersion(100);
    info.SetSourceData(vertexShader.data());
    info.SetSourceSize(vertexShader.size());
    info.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

    size_t dataSize  = 0;
    size_t dataIndex = 0;
    Graphics::GLES::ShaderImpl::StripLegacyCodeIfNeeded(info, dataIndex, dataSize);

    DALI_TEST_EQUALS(dataIndex, 0, TEST_LOCATION);
    DALI_TEST_EQUALS(dataSize, vertexShader.size(), TEST_LOCATION);
  }

  END_TEST;
}

int UtcDaliGlesStripLegacyCodeTestDifferentPrefix(void)
{
  TestGraphicsApplication application;

  std::string vertexShader =
    "//@version 100\n"
    "some code\n";

  std::string somePrefix =
    "This is some prefix\n";

  auto newVertexPrefix = Dali::Integration::Test::GenerateTaggedShaderPrefix(somePrefix);
  {
    Dali::Graphics::ShaderCreateInfo info;
    info.SetPipelineStage(Dali::Graphics::PipelineStage::VERTEX_SHADER);

    std::string prefixedVertexShader = newVertexPrefix + vertexShader;

    info.SetShaderVersion(100);
    info.SetSourceData(prefixedVertexShader.data());
    info.SetSourceSize(prefixedVertexShader.size());
    info.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

    size_t dataSize  = 0;
    size_t dataIndex = 0;
    Graphics::GLES::ShaderImpl::StripLegacyCodeIfNeeded(info, dataIndex, dataSize);

    auto index = prefixedVertexShader.find("//@version");

    DALI_TEST_EQUALS(dataIndex, index, TEST_LOCATION);

    // should match original shader size
    DALI_TEST_EQUALS(dataSize, vertexShader.size(), TEST_LOCATION);
  }

  END_TEST;
}

int UtcDaliGlesStripLegacyCodeIfNeededTest2(void)
{
  TestGraphicsApplication application;

  std::string vertexShader =
    "//@version 100\n"
    "some code\n";

  auto vertexPrefix = Dali::Shader::GetVertexShaderPrefix();

  {
    Dali::Graphics::ShaderCreateInfo info;
    info.SetPipelineStage(Dali::Graphics::PipelineStage::VERTEX_SHADER);

    std::string prefixedVertexShader = Dali::Shader::GetVertexShaderPrefix() + vertexShader;

    info.SetShaderVersion(100);
    info.SetSourceData(prefixedVertexShader.data());
    info.SetSourceSize(prefixedVertexShader.size());
    info.SetSourceMode(Dali::Graphics::ShaderSourceMode::TEXT);

    size_t dataSize  = 0;
    size_t dataIndex = 0;
    Graphics::GLES::ShaderImpl::StripLegacyCodeIfNeeded(info, dataIndex, dataSize);

    DALI_TEST_EQUALS(dataIndex, vertexPrefix.length(), TEST_LOCATION);

    // should match original shader size
    DALI_TEST_EQUALS(dataSize, vertexShader.size(), TEST_LOCATION);
  }

  END_TEST;
}