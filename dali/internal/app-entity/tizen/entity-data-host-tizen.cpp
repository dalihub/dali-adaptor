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

// CLASS HEADER
#include <dali/internal/app-entity/tizen/entity-data-host-tizen.h>

#ifdef ENABLE_ENTITY_DATA_TIDL
// EXTERNAL INCLUDES
#include <memory>
#include <cstdlib>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/devel-api/adaptor-framework/environment-variable.h>

// GENERATED INCLUDES (actionc, build tree)
#include <view-stub.h>
#endif // ENABLE_ENTITY_DATA_TIDL

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
using Dali::Integration::FocusedActorProvider;
#ifdef ENABLE_ENTITY_DATA_TIDL
namespace
{
namespace Generated = ::rpc_port::implview;
#if defined(DEBUG_ENABLED)
Dali::Integration::Log::Filter* gLogFilter = Dali::Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ENTITY_DATA");
#endif

/// Converts internal EntityData into the generated TizenEntityView.
Generated::TizenEntityView ToTizenEntityView(const EntityData& viewData)
{
  Generated::ScreenBounds screenBounds(static_cast<double>(viewData.screenBounds.x),
                                       static_cast<double>(viewData.screenBounds.y),
                                       static_cast<double>(viewData.screenBounds.width),
                                       static_cast<double>(viewData.screenBounds.height));
  Generated::WindowBounds windowBounds(static_cast<double>(viewData.windowBounds.x),
                                       static_cast<double>(viewData.windowBounds.y),
                                       static_cast<double>(viewData.windowBounds.width),
                                       static_cast<double>(viewData.windowBounds.height));
  Generated::Annotation annotation(viewData.annotation.entityId, viewData.annotation.entityType, viewData.annotation.entityInfo);

  // TizenEntity base fields: Id, Extra (Extra is unused by this contract).
  return Generated::TizenEntityView(viewData.id, std::string(), viewData.type, viewData.description, screenBounds, windowBounds, viewData.isFocused, viewData.isEnabled, annotation);
}

/// Converts a generated TizenEntityView into internal EntityData.
EntityData FromTizenEntityView(const Generated::TizenEntityView& generatedView)
{
  EntityData viewData;
  viewData.id          = generatedView.GetId();
  viewData.type        = generatedView.GetType();
  viewData.description = generatedView.GetDescription();

  const Generated::ScreenBounds& generatedScreenBounds = generatedView.GetScreenBounds();
  viewData.screenBounds.x      = static_cast<float>(generatedScreenBounds.GetX());
  viewData.screenBounds.y      = static_cast<float>(generatedScreenBounds.GetY());
  viewData.screenBounds.width  = static_cast<float>(generatedScreenBounds.GetWidth());
  viewData.screenBounds.height = static_cast<float>(generatedScreenBounds.GetHeight());

  const Generated::WindowBounds& generatedWindowBounds = generatedView.GetWindowBounds();
  viewData.windowBounds.x      = static_cast<float>(generatedWindowBounds.GetX());
  viewData.windowBounds.y      = static_cast<float>(generatedWindowBounds.GetY());
  viewData.windowBounds.width  = static_cast<float>(generatedWindowBounds.GetWidth());
  viewData.windowBounds.height = static_cast<float>(generatedWindowBounds.GetHeight());

  viewData.isFocused = generatedView.GetIsFocused();
  viewData.isEnabled = generatedView.GetIsEnabled();

  const Generated::Annotation& generatedAnnotation = generatedView.GetAnnotation();
  viewData.annotation.entityId   = generatedAnnotation.GetEntityId();
  viewData.annotation.entityType = generatedAnnotation.GetEntityType();
  viewData.annotation.entityInfo = generatedAnnotation.GetEntityInfo();
  return viewData;
}

/**
 * @brief Per-connection TIDL request handler that delegates the four entity-data
 * actions to the shared EntityDataService and maps generated/internal types.
 */
class EntityDataTidlRequestHandler : public Generated::stub::TizenActionView::ServiceBase
{
public:
  EntityDataTidlRequestHandler(EntityDataService& service, std::string sender, std::string instance)
  : ServiceBase(std::move(sender), std::move(instance)),
    mService(service)
  {
  }

  void OnCreate() override
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host OnCreate: sender=%s instance=%s\n",
                  GetSender().c_str(), GetInstance().c_str());
  }

  void OnTerminate() override
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host OnTerminate: sender=%s instance=%s\n",
                  GetSender().c_str(), GetInstance().c_str());
  }

  Generated::TizenEntityStatus FindById(std::string id, Generated::TizenEntityView& generatedView) override
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host FindById: request id=%s\n", id.c_str());
    EntityData viewData;
    if(!mService.FindById(id, viewData))
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host FindById: not found id=%s\n", id.c_str());
      return Generated::TizenEntityStatus(false, "View not found");
    }

    generatedView = ToTizenEntityView(viewData);
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host FindById: response id=%s type=%s entityId=%s\n",
                  generatedView.GetId().c_str(), generatedView.GetType().c_str(),
                  generatedView.GetAnnotation().GetEntityId().c_str());
    return Generated::TizenEntityStatus(true, std::string());
  }

  Generated::TizenEntityStatus GetFocusedView(Generated::TizenEntityView& generatedView) override
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetFocusedView: request\n");
    EntityData viewData;
    if(!mService.GetFocusedEntityData(viewData))
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetFocusedView: no focused view\n");
      return Generated::TizenEntityStatus(false, "No focused view");
    }
    generatedView = ToTizenEntityView(viewData);
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetFocusedView: response id=%s type=%s entityId=%s\n",
                  generatedView.GetId().c_str(), generatedView.GetType().c_str(),
                  generatedView.GetAnnotation().GetEntityId().c_str());
    return Generated::TizenEntityStatus(true, std::string());
  }

  Generated::TizenEntityStatus GetAnnotatedViews(std::vector<Generated::TizenEntityView>& views) override
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetAnnotatedViews: request\n");
    std::vector<EntityData> viewDataList;
    mService.GetEntityData(viewDataList);
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetAnnotatedViews: service count=%zu\n", viewDataList.size());

    views.clear();
    views.reserve(viewDataList.size());
    for(const auto& viewData : viewDataList)
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetAnnotatedViews: id=%s type=%s entityId=%s\n",
                    viewData.id.c_str(), viewData.type.c_str(), viewData.annotation.entityId.c_str());
      views.push_back(ToTizenEntityView(viewData));
    }
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetAnnotatedViews: response count=%zu\n", views.size());
    return Generated::TizenEntityStatus(true, std::string());
  }

  Generated::TizenEntityStatus ToPresentation(Generated::TizenEntityView generatedView, Generated::TizenEntityPresentation& result) override
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host ToPresentation: request id=%s type=%s\n",
                  generatedView.GetId().c_str(), generatedView.GetType().c_str());
    std::string presentation;
    if(!mService.ToPresentation(FromTizenEntityView(generatedView), presentation))
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host ToPresentation: failed id=%s\n", generatedView.GetId().c_str());
      return Generated::TizenEntityStatus(false, "ToPresentation failed");
    }
    // The View presentation payload is carried in the Document field.
    result = Generated::TizenEntityPresentation(presentation, std::string());
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host ToPresentation: response bytes=%zu\n", presentation.size());
    return Generated::TizenEntityStatus(true, std::string());
  }

private:
  EntityDataService& mService;
};

/// Creates an EntityDataTidlRequestHandler for each connecting client.
class EntityDataServiceFactory : public Generated::stub::TizenActionView::ServiceBase::Factory
{
public:
  explicit EntityDataServiceFactory(EntityDataService& service)
  : mService(service)
  {
  }

  std::unique_ptr<Generated::stub::TizenActionView::ServiceBase> CreateService(std::string sender, std::string instance) override
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host CreateService: sender=%s instance=%s\n",
                  sender.c_str(), instance.c_str());
    return std::unique_ptr<Generated::stub::TizenActionView::ServiceBase>(new EntityDataTidlRequestHandler(mService, std::move(sender), std::move(instance)));
  }

private:
  EntityDataService& mService;
};

} // unnamed namespace
#endif // ENABLE_ENTITY_DATA_TIDL

EntityDataHostTizen::EntityDataHostTizen(FocusedActorProvider* provider)
: mService(std::make_unique<EntityDataService>(provider))
{
#ifdef ENABLE_ENTITY_DATA_TIDL
  try
  {
    mStub = std::make_unique<Generated::stub::TizenActionView>();
    mStub->Listen(std::make_shared<EntityDataServiceFactory>(*mService));
    DALI_LOG_INFO(gLogFilter, Debug::General, "Tizen.Action.View TIDL stub is listening\n");
  }
  catch(const Generated::stub::Exception& e)
  {
    // rpc-port not registered / insufficient privileges: keep the service usable
    // in-process but serve no TIDL requests.
    DALI_LOG_ERROR("Failed to start Tizen.Action.View TIDL stub: %s\n", e.what());
    mStub.reset();
  }
#endif // ENABLE_ENTITY_DATA_TIDL
}

EntityDataHostTizen::~EntityDataHostTizen() = default;

EntityDataService& EntityDataHostTizen::GetService()
{
  return *mService;
}

bool EntityDataHostTizen::IsListening() const
{
#ifdef ENABLE_ENTITY_DATA_TIDL
  return mStub != nullptr;
#else
  return false;
#endif // ENABLE_ENTITY_DATA_TIDL
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

std::unique_ptr<Dali::Internal::Adaptor::EntityDataHost> Dali::Internal::Adaptor::CreateEntityDataHost(Dali::Integration::FocusedActorProvider* provider)
{
  auto disabled = Dali::EnvironmentVariable::GetEnvironmentVariable("DALI_DISABLE_ENTITY_DATA_TIDL");
  if(disabled && std::atoi(disabled))
  {
    return nullptr;
  }
  return std::make_unique<Dali::Internal::Adaptor::EntityDataHostTizen>(provider);
}
