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

#include "test-graphics-program.h"

static uint32_t gId{0};

namespace Dali
{
TestGraphicsProgramImpl::TestGraphicsProgramImpl(
  TestGraphicsController& controller,
  const Graphics::ProgramCreateInfo& createInfo,
  Property::Array& vertexFormats,
  std::vector<UniformData>& customUniforms,
  std::vector<TestGraphicsReflection::TestUniformBlockInfo>& customUniformBlocks)

: mController(controller),
  mId(++gId/*gl.CreateProgram()*/),
  mCreateInfo(createInfo),
  mReflection(controller, mId, vertexFormats, createInfo, customUniforms, customUniformBlocks)
{
  // Ensure active sampler uniforms are set
  //mGl.SetCustomUniforms(customUniforms); @todo FIXME!!!

  // Don't need to re-register uniforms in GL side - now done in creation of mReflection.
  // Was previously done in mGl.LinkProgram(mId);
}

bool TestGraphicsProgramImpl::GetParameter(uint32_t parameterId, void* outData)
{
  reinterpret_cast<uint32_t*>(outData)[0] = mId;
  return true;
}

TestGraphicsProgram::TestGraphicsProgram(TestGraphicsProgramImpl* impl)
{
  mImpl = impl;
}

} // namespace Dali
