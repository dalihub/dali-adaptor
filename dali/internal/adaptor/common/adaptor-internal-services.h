#ifndef DALI_INTERNAL_ADAPTOR_INTERNAL_SERVICES_H
#define DALI_INTERNAL_ADAPTOR_INTERNAL_SERVICES_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/integration-api/core.h>
#include <dali/integration-api/gl-abstraction.h>

// INTERNAL INCLUDES
#include <dali/integration-api/render-surface.h>
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory-interface.h>
#include <dali/integration-api/adaptor-framework/trigger-event-interface.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/gles/egl-factory-interface.h>
#include <dali/internal/network/common/socket-factory-interface.h>
#include <dali/internal/network/common/trace-interface.h>
#include <dali/internal/system/common/performance-interface.h>
#include <dali/internal/window-system/common/display-connection.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * A class to contain various interfaces provided by the adaptor which
 * can be used by the cross platform parts of adaptor.
 * E.g. any files held in adaptors/base/ directory
 *
 */
class AdaptorInternalServices
{

public:

  /**
   * @return core
   */
  virtual Dali::Integration::Core& GetCore() = 0;

  /**
   * @return platform abstraction
   */
  virtual Dali::Integration::PlatformAbstraction& GetPlatformAbstractionInterface() = 0;

  /**
   * Used to access the Display Connection interface from the Render thread
   * @return the Display Connection interface
   */
  virtual Dali::DisplayConnection& GetDisplayConnectionInterface() = 0;

  /**
   * Used to access the abstracted graphics interface
   * This also contains the depth and stencil buffers
   * @return the graphics interface
   */
  virtual GraphicsInterface& GetGraphicsInterface() = 0;

  /**
   * Used by update-thread to notify core (main-thread) it has messages to process
   * @return trigger event ProcessCoreEvents
   */
  virtual TriggerEventInterface& GetProcessCoreEventsTrigger() = 0;

  /**
   * @return trigger event factory interface
   */
  virtual TriggerEventFactoryInterface& GetTriggerEventFactoryInterface() = 0;

  /**
   * @return socket factory interface
   */
  virtual SocketFactoryInterface& GetSocketFactoryInterface() = 0;

  /**
   * @return render surface
   */
  virtual Dali::RenderSurfaceInterface* GetRenderSurfaceInterface() = 0;

  /**
   * @return performance interface
   */
  virtual PerformanceInterface* GetPerformanceInterface() = 0;

  /**
   * @return interface for logging to the kernel ( e.g. using ftrace )
   */
  virtual TraceInterface& GetKernelTraceInterface() = 0;

  /**
   * @return system trace interface, e.g. for using Tizen Trace (ttrace) or Android Trace (atrace)
   */
  virtual TraceInterface& GetSystemTraceInterface() = 0;


protected:

  /**
   * constructor
   */
  AdaptorInternalServices()
  {
  };

  /**
   * virtual destructor
   */
  virtual ~AdaptorInternalServices()
  {
  };

  // Undefined copy constructor.
  AdaptorInternalServices( const AdaptorInternalServices& ) = delete;

  // Undefined assignment operator.
  AdaptorInternalServices& operator=( const AdaptorInternalServices& ) = delete;
};

} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_INTERNAL_SERVICES_H
