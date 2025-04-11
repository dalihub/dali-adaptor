#ifndef DALI_GRAPHICS_VULKAN_GRAPHICS_CONTROLLER_DEBUG_H
#define DALI_GRAPHICS_VULKAN_GRAPHICS_CONTROLLER_DEBUG_H

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

#include <dali/internal/graphics/vulkan-impl/vulkan-stored-command-buffer.h>
#include <unordered_set>

namespace Dali::Graphics
{
#if defined(DEBUG_ENABLED)
/**
 * Struct to manage dumping N command buffers. In this file, "frame" really
 * means each call to VulkanGraphicsController::SubmitCommandBuffers(), and doesn't necessarily
 * correspond to a complete render frame.
 */
struct GraphicsFrameDump
{
  using UniqueFilePtr = std::unique_ptr<std::FILE, int (*)(std::FILE*)>;
  UniqueFilePtr                                   outputStream;
  FILE*                                           output{nullptr};
  bool                                            dumpingFrame{false};
  bool                                            firstBuffer{true};
  bool                                            firstFrame{true};
  int                                             frameCount{0};
  int                                             fileCount{1};
  std::unordered_set<const Vulkan::RenderTarget*> renderTargets{};

  const int NTH_FRAME{10}; // dump first N "frames"

  // Constructor
  GraphicsFrameDump();

  void Start();
  void End();
  void DumpCommandBuffer(const Vulkan::StoredCommandBuffer* cmdBuf);
  void DumpRenderTargets();
  bool IsDumpFrame();
};
#endif

#if defined(DEBUG_ENABLED) && defined(ENABLE_COMMAND_BUFFER_FRAME_DUMP)
#define DUMP_FRAME_INIT() std::unique_ptr<Dali::Graphics::GraphicsFrameDump> gGraphicsFrameDump(new Dali::Graphics::GraphicsFrameDump)
#define DUMP_FRAME_START()       \
  if(gGraphicsFrameDump)         \
  {                              \
    gGraphicsFrameDump->Start(); \
  }
#define DUMP_FRAME_COMMAND_BUFFER(cmdBuffer)          \
  if(gGraphicsFrameDump)                              \
  {                                                   \
    gGraphicsFrameDump->DumpCommandBuffer(cmdBuffer); \
  }
#define DUMP_FRAME_END()       \
  if(gGraphicsFrameDump)       \
  {                            \
    gGraphicsFrameDump->End(); \
  }
#else
#define DUMP_FRAME_INIT()
#define DUMP_FRAME_START()
#define DUMP_FRAME_COMMAND_BUFFER(cmdBuffer)
#define DUMP_FRAME_END()
#endif

} // namespace Dali::Graphics

#endif // DALI_GRAPHICS_VULKAN_GRAPHICS_CONTROLLER_DEBUG_H
