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
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>

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
Generated::TizenEntityView ToTizenEntityView(const EntityData& entityData)
{
  Generated::ScreenBounds screenBounds(static_cast<double>(entityData.screenBounds.x),
                                       static_cast<double>(entityData.screenBounds.y),
                                       static_cast<double>(entityData.screenBounds.width),
                                       static_cast<double>(entityData.screenBounds.height));
  Generated::WindowBounds windowBounds(static_cast<double>(entityData.windowBounds.x),
                                       static_cast<double>(entityData.windowBounds.y),
                                       static_cast<double>(entityData.windowBounds.width),
                                       static_cast<double>(entityData.windowBounds.height));
  Generated::Annotation   annotation(entityData.annotation.entityId, entityData.annotation.entityType, entityData.annotation.entityInfo);

  // TizenEntity base fields: Id, Extra (Extra is unused by this contract).
  return Generated::TizenEntityView(entityData.actorId, std::string(), entityData.actorTypeName, entityData.description, screenBounds, windowBounds, entityData.isFocused, entityData.isEnabled, annotation);
}

/// Converts a generated TizenEntityView into internal EntityData.
EntityData FromTizenEntityView(const Generated::TizenEntityView& generatedView)
{
  EntityData entityData;
  entityData.actorId       = generatedView.GetId();
  entityData.actorTypeName = generatedView.GetType();
  entityData.description   = generatedView.GetDescription();

  const Generated::ScreenBounds& generatedScreenBounds = generatedView.GetScreenBounds();
  entityData.screenBounds.x                            = static_cast<float>(generatedScreenBounds.GetX());
  entityData.screenBounds.y                            = static_cast<float>(generatedScreenBounds.GetY());
  entityData.screenBounds.width                        = static_cast<float>(generatedScreenBounds.GetWidth());
  entityData.screenBounds.height                       = static_cast<float>(generatedScreenBounds.GetHeight());

  const Generated::WindowBounds& generatedWindowBounds = generatedView.GetWindowBounds();
  entityData.windowBounds.x                            = static_cast<float>(generatedWindowBounds.GetX());
  entityData.windowBounds.y                            = static_cast<float>(generatedWindowBounds.GetY());
  entityData.windowBounds.width                        = static_cast<float>(generatedWindowBounds.GetWidth());
  entityData.windowBounds.height                       = static_cast<float>(generatedWindowBounds.GetHeight());

  entityData.isFocused = generatedView.GetIsFocused();
  entityData.isEnabled = generatedView.GetIsEnabled();

  const Generated::Annotation& generatedAnnotation = generatedView.GetAnnotation();
  entityData.annotation.entityId                   = generatedAnnotation.GetEntityId();
  entityData.annotation.entityType                 = generatedAnnotation.GetEntityType();
  entityData.annotation.entityInfo                 = generatedAnnotation.GetEntityInfo();
  return entityData;
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
    EntityData entityData;
    if(!mService.FindByActorId(id, entityData))
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host FindById: not found id=%s\n", id.c_str());
      return Generated::TizenEntityStatus(false, "View not found");
    }

    generatedView = ToTizenEntityView(entityData);
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host FindById: response id=%s type=%s entityId=%s\n",
                  generatedView.GetId().c_str(), generatedView.GetType().c_str(),
                  generatedView.GetAnnotation().GetEntityId().c_str());
    return Generated::TizenEntityStatus(true, std::string());
  }

  Generated::TizenEntityStatus GetFocusedView(Generated::TizenEntityView& generatedView) override
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetFocusedView: request\n");
    EntityData entityData;
    if(!mService.GetFocusedEntityData(entityData))
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetFocusedView: no focused view\n");
      return Generated::TizenEntityStatus(false, "No focused view");
    }
    generatedView = ToTizenEntityView(entityData);
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetFocusedView: response id=%s type=%s entityId=%s\n",
                  generatedView.GetId().c_str(), generatedView.GetType().c_str(),
                  generatedView.GetAnnotation().GetEntityId().c_str());
    return Generated::TizenEntityStatus(true, std::string());
  }

  Generated::TizenEntityStatus GetAnnotatedViews(std::vector<Generated::TizenEntityView>& views) override
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetAnnotatedViews: request\n");
    std::vector<EntityData> entityDataList;
    if(!mService.GetAnnotatedEntities(entityDataList))
    {
      views.clear();
      DALI_LOG_ERROR("Failed to enumerate annotated entities\n");
      return Generated::TizenEntityStatus(false, "Failed to enumerate annotated views");
    }
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetAnnotatedViews: service count=%zu\n", entityDataList.size());

    views.clear();
    views.reserve(entityDataList.size());
    for(const auto& entityData : entityDataList)
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData host GetAnnotatedViews: id=%s type=%s entityId=%s\n",
                    entityData.actorId.c_str(), entityData.actorTypeName.c_str(), entityData.annotation.entityId.c_str());
      views.push_back(ToTizenEntityView(entityData));
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
