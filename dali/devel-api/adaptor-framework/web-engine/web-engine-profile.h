#pragma once

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
 *
 */

#include <dali/devel-api/adaptor-framework/web-engine/web-engine.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-direct-rendering-session.h>
#include <cstdint>
#include <memory>

namespace DALI_NAMESPACE
{
/** Result of storing process-wide engine arguments before initialization. */
enum class WebEngineArgumentsResult : uint8_t
{
  SUCCESS,
  INVALID_ARGUMENT,
  ALREADY_INITIALIZED,
  BACKEND_UNAVAILABLE
};

/**
 * Owned profile dependency. Copies share one plugin token and one native reference.
 * Accessors return null after application termination. Use on the DALi event thread.
 */
class DALI_ADAPTOR_API WebEngineProfile
{
public:
  virtual ~WebEngineProfile() = default;
  virtual WebEngineContext* GetContext() const = 0;
  virtual WebEngineCookieManager* GetCookieManager() const = 0;
};
using WebEngineProfilePtr = std::shared_ptr<WebEngineProfile>;

/**
 * Optional versioned plugin extension, obtained through GetWebEngineProfileApi.
 * Existing WebEnginePlugin virtual slots remain unchanged. Tokens must be destroyed
 * by their originating plugin. All calls run on the DALi event thread.
 */
struct WebEngineProfileApi
{
  uint32_t version;
  uint32_t structSize;
  WebEngineArgumentsResult (*storeArguments)(uint32_t argc, const char* const* argv);
  void (*readArguments)(uint32_t* argc, const char* const** argv);
  bool (*initialize)();
  void* (*getDefaultProfile)();
  void* (*createProfile)(bool incognito, const char* bundlePath);
  void (*destroyProfile)(void* profile);
  WebEngineContext* (*getContext)(void* profile);
  WebEngineCookieManager* (*getCookieManager)(void* profile);
  bool (*createView)(WebEnginePlugin* plugin, void* profile, uint32_t width, uint32_t height);
  /** Optional append-only entry. Check structSize before accessing it. */
  WebEngineDirectRenderingSessionPtr (*createDirectRenderingView)(WebEnginePlugin* plugin, void* profile, uint32_t width, uint32_t height);
};

/** Shared initialization/type state for the UI and plugin loader. */
class DALI_ADAPTOR_API WebEngineRuntime
{
public:
  static bool SelectType(int32_t type);
  static bool HasInitializationStarted();
  static WebEngineArgumentsResult SetArguments(uint32_t argc, char** argv);
  static WebEngineProfilePtr GetDefaultProfile();
  static WebEngineProfilePtr CreateProfile(bool incognito, const char* bundlePath);
  static bool CreateView(WebEngine engine, const WebEngineProfilePtr& profile, uint32_t width, uint32_t height);
  /** Creates direct output with the same profile ownership and backend checks as CreateView(). */
  static WebEngineDirectRenderingConnection CreateDirectRenderingView(WebEngine engine, const WebEngineProfilePtr& profile, uint32_t width, uint32_t height);
};
} // namespace DALI_NAMESPACE
