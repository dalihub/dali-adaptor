#ifndef DALI_WIDGET_ENGINE_H
#define DALI_WIDGET_ENGINE_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/widget-engine/widget-engine-instance.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
class WidgetEngine;
}
} //namespace Internal

/**
 * @brief Proxy that loads a platform widget engine plugin on demand.
 *
 * This interface is for UI framework implementation only. Direct use by
 * applications is unsupported. Applications must use Dali::Ui::WidgetView to
 * create and control widgets.
 *
 * Each instance retains its engine. Releasing the last engine handle schedules
 * plugin destruction once its instances and deferred resources are released.
 * Create and use the engine and its instances on the event thread.
 */
class DALI_ADAPTOR_API WidgetEngine : public BaseHandle
{
public:
  WidgetEngine();
  ~WidgetEngine();

  WidgetEngine(const WidgetEngine& engine);
  WidgetEngine& operator=(const WidgetEngine& engine);
  WidgetEngine(WidgetEngine&& engine) noexcept;
  WidgetEngine& operator=(WidgetEngine&& engine) noexcept;

  /**
   * @brief Loads and initializes the platform engine for the host application.
   * @param[in] applicationId The application hosting the widget instances.
   * @return An initialized engine, or an empty handle if loading or initialization fails.
   */
  static WidgetEngine New(const std::string& applicationId);

  /**
   * @brief Gets the shared engine retained until application shutdown.
   *
   * Reuses the application engine for the same ID. A conflicting application ID,
   * an unavailable SingletonService, or initialization failure returns an empty
   * handle. New() remains an explicit-lifetime factory and does not register a singleton.
   * @param[in] applicationId The application hosting the widget instances.
   * @return The application-owned engine or an empty handle.
   */
  static WidgetEngine Get(const std::string& applicationId);
  static WidgetEngine DownCast(BaseHandle handle);

  WidgetEngineInstance CreateWidget(const std::string& widgetId,
                                    const std::string& contentInfo,
                                    int                width,
                                    int                height,
                                    float              updatePeriod);

  /**
   * @brief Whether this engine has a loaded plugin instance.
   * This does not indicate whether a widget provider is running.
   */
  bool IsAvailable() const;

public: // Not intended for application developers
  explicit DALI_INTERNAL WidgetEngine(Internal::Adaptor::WidgetEngine* implementation);
};

} // namespace DALI_NAMESPACE

#endif // DALI_WIDGET_ENGINE_H
