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

int UtcDaliBufferReuseTest(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliBufferReuseTest: Tests whether GLES buffer can be reused (orphaning content)");

  auto& controller = app.GetGraphicsController();

  Graphics::BufferCreateInfo info;
  info.size            = 1024;
  info.usage           = 0u | Graphics::BufferUsage::VERTEX_BUFFER;
  info.propertiesFlags = 0u;

  auto buffer = controller.CreateBuffer(info, nullptr);
  controller.WaitIdle();

  DALI_TEST_NOT_EQUALS((void*)buffer.get(), (void*)nullptr, 0, TEST_LOCATION);

  // New buffer with different spec, should create new object
  Graphics::BufferCreateInfo info2;
  info.size            = 2024;
  info.usage           = 0u | Graphics::BufferUsage::VERTEX_BUFFER;
  info.propertiesFlags = 0u;

  auto buffer2 = controller.CreateBuffer(info2, std::move(buffer));
  controller.WaitIdle();

  DALI_TEST_NOT_EQUALS(buffer.get(), buffer2.get(), 0, TEST_LOCATION);

  auto ptr = buffer2.get(); // store pointer for testing, the uptr will be emptied
  // Create new buffer using the same spec
  auto buffer3 = controller.CreateBuffer(info2, std::move(buffer2));
  controller.WaitIdle();

  DALI_TEST_EQUALS(ptr, buffer3.get(), TEST_LOCATION);

  END_TEST;
}
