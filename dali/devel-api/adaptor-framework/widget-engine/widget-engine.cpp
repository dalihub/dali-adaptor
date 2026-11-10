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

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/widget-engine/widget-engine.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/devel-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/widget-engine/common/widget-engine-impl.h>

namespace DALI_NAMESPACE
{
WidgetEngine::WidgetEngine()                                          = default;
WidgetEngine::~WidgetEngine()                                         = default;
WidgetEngine::WidgetEngine(const WidgetEngine& engine)                = default;
WidgetEngine& WidgetEngine::operator=(const WidgetEngine& engine)     = default;
WidgetEngine::WidgetEngine(WidgetEngine&& engine) noexcept            = default;
WidgetEngine& WidgetEngine::operator=(WidgetEngine&& engine) noexcept = default;

WidgetEngine::WidgetEngine(Internal::Adaptor::WidgetEngine* implementation)
: BaseHandle(implementation)
{
}

WidgetEngine WidgetEngine::New(const std::string& applicationId)
{
  Internal::Adaptor::WidgetEnginePtr engine = Internal::Adaptor::WidgetEngine::New(applicationId);
  return WidgetEngine(engine.Get());
}

WidgetEngine WidgetEngine::Get(const std::string& applicationId)
{
  auto service = SingletonService::Get();
  if(!service)
  {
    return {};
  }
  constexpr Dali::TypeInfoId typeId = DALI_TYPE_ID(Dali::WidgetEngine);
  auto                       engine = WidgetEngine::DownCast(service.GetSingleton(typeId));
  if(engine)
  {
    if(Internal::Adaptor::GetImplementation(engine).GetApplicationId() != applicationId)
    {
      DALI_LOG_ERROR("WidgetEngine::Get: application ID differs from the shared engine.\n");
      return {};
    }
    return engine;
  }
  engine = WidgetEngine::New(applicationId);
  if(engine)
  {
    service.Register(typeId, engine);
  }
  return engine;
}

WidgetEngine WidgetEngine::DownCast(BaseHandle handle)
{
  return WidgetEngine(dynamic_cast<Internal::Adaptor::WidgetEngine*>(handle.GetObjectPtr()));
}

WidgetEngineInstance WidgetEngine::CreateWidget(const std::string& widgetId,
                                                const std::string& contentInfo,
                                                int                width,
                                                int                height,
                                                float              updatePeriod)
{
  Internal::Adaptor::WidgetEngineInstancePtr instance = Internal::Adaptor::GetImplementation(*this).CreateWidget(widgetId, contentInfo, width, height, updatePeriod);
  return WidgetEngineInstance(instance.Get());
}

bool WidgetEngine::IsAvailable() const
{
  return Internal::Adaptor::GetImplementation(*this).IsAvailable();
}

} // namespace DALI_NAMESPACE
