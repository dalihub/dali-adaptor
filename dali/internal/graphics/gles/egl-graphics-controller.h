#ifndef DALI_EGL_GRAPHICS_CONTROLLER_H
#define DALI_EGL_GRAPHICS_CONTROLLER_H

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

#include <dali/graphics-api/graphics-controller.h>

namespace Dali
{
namespace Integration
{
class GlAbstraction;
class GlSyncAbstraction;
class GlContextHelperAbstraction;
} // namespace Integration

namespace Graphics
{
/**
 * EGL Implementation of the graphics controller.
 *
 * Temporarily holds the old GL abstractions whilst dali-core is migrated to the new API.
 */
class EglGraphicsController : public Graphics::Controller
{
public:
  EglGraphicsController() = default;

  virtual ~EglGraphicsController() = default;

  /**
   * Initialize the GLES abstraction. This can be called from the main thread.
   */
  void InitializeGLES(Integration::GlAbstraction& glAbstraction);

  /**
   * Initialize with a reference to the GL abstractions.
   *
   * Note, this is now executed in the render thread, after core initialization
   */
  void Initialize(Integration::GlSyncAbstraction&          glSyncAbstraction,
                  Integration::GlContextHelperAbstraction& glContextHelperAbstraction);

  Integration::GlAbstraction&              GetGlAbstraction() override;
  Integration::GlSyncAbstraction&          GetGlSyncAbstraction() override;
  Integration::GlContextHelperAbstraction& GetGlContextHelperAbstraction() override;

private:
  Integration::GlAbstraction*              mGlAbstraction{nullptr};
  Integration::GlSyncAbstraction*          mGlSyncAbstraction{nullptr};
  Integration::GlContextHelperAbstraction* mGlContextHelperAbstraction{nullptr};
};

} // namespace Graphics

} // namespace Dali

#endif //DALI_EGL_GRAPHICS_CONTROLLER_H
