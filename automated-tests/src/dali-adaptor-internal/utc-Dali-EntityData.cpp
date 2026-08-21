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

#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/devel-api/actors/actor-devel.h>

#include <dali/internal/app-entity/common/entity-data-builder.h>
#include <dali/internal/app-entity/common/entity-data-service.h>

#include <cstdint>
#include <limits>
#include <locale>
#include <string>
#include <vector>

using namespace Dali;

namespace
{
class TestFocusedActorProvider : public Integration::FocusedActorProvider
{
public:
  explicit TestFocusedActorProvider(Actor focusedActor)
  : mFocusedActor(focusedActor)
  {
  }

  Actor GetFocusedActor() override
  {
    return mFocusedActor;
  }

private:
  Actor mFocusedActor;
};

void SetActorAnnotation(Actor actor, const char* entityId, const char* entityType, const char* entityInfo = "")
{
  actor.SetAnnotation(entityId, entityType, entityInfo);
}

class CommaDecimalPoint : public std::numpunct<char>
{
protected:
  char do_decimal_point() const override
  {
    return ',';
  }
};
} // unnamed namespace

void utc_dali_entity_data_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_entity_data_cleanup(void)
{
  Integration::FocusedActorProvider* provider = Integration::GetFocusedActorProvider();
  if(provider)
  {
    Integration::UnregisterFocusedActorProvider(provider);
  }
  test_return_value = TET_PASS;
}

int UtcDaliEntityDataMakeAndFocusedActorP(void)
{
  TestApplication application;
  Actor           actor = Actor::New();
  actor.SetProperty(Actor::Property::POSITION, Vector3(20.4f, 30.6f, 0.0f));
  actor.SetProperty(Actor::Property::SIZE, Vector2(100.2f, 50.8f));
  actor.SetProperty(DevelActor::Property::USER_INTERACTION_ENABLED, true);
  SetActorAnnotation(actor, "sample.entity", "Tizen.Entity.Sample");
  application.GetScene().Add(actor);
  application.SendNotification();
  application.Render();

  TestFocusedActorProvider provider(actor);

  DALI_TEST_CHECK(Integration::GetFocusedActorProvider() == nullptr);
  Integration::RegisterFocusedActorProvider(&provider);
  DALI_TEST_CHECK(Integration::GetFocusedActorProvider() == &provider);

  TestFocusedActorProvider otherProvider(Actor{});
  Integration::UnregisterFocusedActorProvider(&otherProvider);
  DALI_TEST_CHECK(Integration::GetFocusedActorProvider() == &provider);

  Internal::Adaptor::EntityDataService registeredProviderService;
  Internal::Adaptor::EntityData        registeredFocusedEntityData;
  DALI_TEST_CHECK(registeredProviderService.GetFocusedEntityData(registeredFocusedEntityData));
  DALI_TEST_EQUALS(registeredFocusedEntityData.actorId, std::to_string(actor.GetProperty<int32_t>(Actor::Property::ID)), TEST_LOCATION);

  Integration::UnregisterFocusedActorProvider(&provider);
  DALI_TEST_CHECK(Integration::GetFocusedActorProvider() == nullptr);
  DALI_TEST_CHECK(!registeredProviderService.GetFocusedEntityData(registeredFocusedEntityData));

  Internal::Adaptor::EntityData::Annotation annotation;
  DALI_TEST_CHECK(Internal::Adaptor::ReadActorAnnotation(actor, annotation));
  DALI_TEST_EQUALS(annotation.entityId, "sample.entity", TEST_LOCATION);
  DALI_TEST_EQUALS(annotation.entityType, "Tizen.Entity.Sample", TEST_LOCATION);

  Internal::Adaptor::EntityData entityData = Internal::Adaptor::MakeEntityData(actor, &provider, annotation);
  DALI_TEST_CHECK(!entityData.annotation.entityId.empty());
  DALI_TEST_CHECK(!entityData.annotation.entityType.empty());
  DALI_TEST_CHECK(entityData.isFocused);
  DALI_TEST_CHECK(entityData.isEnabled);
  DALI_TEST_CHECK(entityData.windowBounds.width > 0.0);
  DALI_TEST_CHECK(entityData.windowBounds.height > 0.0);
  DALI_TEST_CHECK(entityData.screenBounds.width > 0.0);
  DALI_TEST_CHECK(entityData.screenBounds.height > 0.0);

  // Visibility is computed on demand rather than stored in EntityData.
  DALI_TEST_CHECK(Internal::Adaptor::IsActorVisible(actor));
  const Dali::Rect<float> windowBounds = Internal::Adaptor::GetActorWindowBounds(actor);
  DALI_TEST_EQUALS(windowBounds.width, entityData.windowBounds.width, TEST_LOCATION);
  DALI_TEST_EQUALS(windowBounds.height, entityData.windowBounds.height, TEST_LOCATION);

  Internal::Adaptor::EntityDataService service(&provider);
  Internal::Adaptor::EntityData        focusedEntityData;
  DALI_TEST_CHECK(service.GetFocusedEntityData(focusedEntityData));
  DALI_TEST_EQUALS(focusedEntityData.actorId, entityData.actorId, TEST_LOCATION);
  DALI_TEST_EQUALS(focusedEntityData.actorId, std::to_string(actor.GetProperty<int32_t>(Actor::Property::ID)), TEST_LOCATION);
  DALI_TEST_EQUALS(focusedEntityData.actorTypeName, actor.GetTypeName().c_str(), TEST_LOCATION);
  DALI_TEST_EQUALS(focusedEntityData.annotation.entityId, "sample.entity", TEST_LOCATION);

  std::string presentation;
  DALI_TEST_CHECK(service.ToPresentation(focusedEntityData, presentation));
  DALI_TEST_CHECK(presentation.find("sample.entity") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("Tizen.Entity.Sample") != std::string::npos);

  END_TEST;
}

int UtcDaliEntityDataFindByActorIdP(void)
{
  TestApplication application;
  Actor           root  = Actor::New();
  Actor           actor = Actor::New();
  root.Add(actor);
  application.GetScene().Add(root);
  application.SendNotification();
  application.Render();

  TestFocusedActorProvider             provider(actor);
  Internal::Adaptor::EntityDataService service(&provider);
  Internal::Adaptor::EntityData        entityData;

  DALI_TEST_CHECK(service.FindByActorId(std::to_string(actor.GetProperty<int32_t>(Actor::Property::ID)), root, entityData));
  DALI_TEST_EQUALS(entityData.actorId, std::to_string(actor.GetProperty<int32_t>(Actor::Property::ID)), TEST_LOCATION);

  const std::vector<std::string> malformedActorIds{
    "",
    "-1",
    "+1",
    " 1",
    "1 ",
    "1x",
    std::to_string(static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1u)};
  for(const auto& malformedActorId : malformedActorIds)
  {
    DALI_TEST_CHECK(!service.FindByActorId(malformedActorId, root, entityData));
  }

  DALI_TEST_CHECK(!service.FindByActorId(std::to_string(actor.GetProperty<int32_t>(Actor::Property::ID)), Actor(), entityData));

  END_TEST;
}

int UtcDaliEntityDataGetAnnotatedEntitiesP(void)
{
  TestApplication application;
  Actor           first       = Actor::New();
  Actor           second      = Actor::New();
  Actor           unannotated = Actor::New();
  Actor           hidden      = Actor::New();
  Actor           clipped     = Actor::New();

  const std::vector<Actor> actors{first, second, unannotated, hidden, clipped};
  for(auto actor : actors)
  {
    actor.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_LEFT);
    actor.SetProperty(Actor::Property::SIZE, Vector2(100.0f, 80.0f));
    application.GetScene().Add(actor);
  }

  first.SetProperty(Actor::Property::POSITION, Vector3(30.0f, 180.0f, 0.0f));
  second.SetProperty(Actor::Property::POSITION, Vector3(30.0f, 60.0f, 0.0f));
  hidden.SetProperty(Actor::Property::POSITION, Vector3(160.0f, 60.0f, 0.0f));
  hidden.SetProperty(Actor::Property::VISIBLE, false);
  clipped.SetProperty(Actor::Property::POSITION, Vector3(560.0f, 60.0f, 0.0f));

  SetActorAnnotation(first, "living-room.light", "Tizen.Entity.Light", "{\"level\":72}");
  SetActorAnnotation(second, "hall.thermostat", "Tizen.Entity.Thermostat", "{\"temperature\":23.5}");
  SetActorAnnotation(hidden, "hidden.entity", "Tizen.Entity.Hidden");
  SetActorAnnotation(clipped, "clipped.entity", "Tizen.Entity.Clipped");

  application.SendNotification();
  application.Render();

  TestFocusedActorProvider                   provider(first);
  Internal::Adaptor::EntityDataService       service(&provider);
  std::vector<Internal::Adaptor::EntityData> entities;
  DALI_TEST_CHECK(service.GetAnnotatedEntities(application.GetScene().GetRootLayer(), Dali::Rect<float>{0.0f, 0.0f, 480.0f, 800.0f}, entities));
  DALI_TEST_EQUALS(entities.size(), 2u, TEST_LOCATION);
  DALI_TEST_EQUALS(entities[0].annotation.entityId, "hall.thermostat", TEST_LOCATION);
  DALI_TEST_EQUALS(entities[1].annotation.entityId, "living-room.light", TEST_LOCATION);
  DALI_TEST_CHECK(!entities[0].isFocused);
  DALI_TEST_CHECK(entities[1].isFocused);
  DALI_TEST_EQUALS(entities[1].actorId, std::to_string(first.GetProperty<int32_t>(Actor::Property::ID)), TEST_LOCATION);

  DALI_TEST_CHECK(service.GetAnnotatedEntities(Actor(), Dali::Rect<float>{0.0f, 0.0f, 480.0f, 800.0f}, entities));
  DALI_TEST_CHECK(entities.empty());

  END_TEST;
}

int UtcDaliEntityDataInvalidAnnotationAndNoFocusedActorP(void)
{
  TestApplication application;
  Actor           actor = Actor::New();
  application.GetScene().Add(actor);

  Internal::Adaptor::EntityData::Annotation annotation;
  annotation.entityId   = "stale.entity";
  annotation.entityType = "Tizen.Entity.Stale";
  DALI_TEST_CHECK(!Internal::Adaptor::ReadActorAnnotation(actor, annotation));
  DALI_TEST_EQUALS(annotation.entityId, "", TEST_LOCATION);
  DALI_TEST_EQUALS(annotation.entityType, "", TEST_LOCATION);

  Actor                                emptyActor;
  TestFocusedActorProvider             provider(emptyActor);
  Internal::Adaptor::EntityDataService service(&provider);
  Internal::Adaptor::EntityData        focusedEntityData;
  DALI_TEST_CHECK(!service.GetFocusedEntityData(focusedEntityData));

  END_TEST;
}

int UtcDaliEntityDataHierarchyVisibilityP(void)
{
  TestApplication application;
  Actor           parent = Actor::New();
  Actor           child  = Actor::New();
  parent.Add(child);
  application.GetScene().Add(parent);

  DALI_TEST_CHECK(Internal::Adaptor::IsActorVisible(child));
  DALI_TEST_CHECK(Internal::Adaptor::IsActorHierarchyVisible(child));

  // A non-clipping parent still controls whether its descendants are visible.
  DALI_TEST_CHECK(parent.GetClippingMode() == ClippingMode::DISABLED);
  parent.SetProperty(Actor::Property::VISIBLE, false);
  DALI_TEST_CHECK(Internal::Adaptor::IsActorVisible(child));
  DALI_TEST_CHECK(!Internal::Adaptor::IsActorHierarchyVisible(child));

  END_TEST;
}

int UtcDaliEntityDataPresentationValidationP(void)
{
  Internal::Adaptor::EntityDataService service;
  Internal::Adaptor::EntityData        entityData;

  entityData.actorId       = "42";
  entityData.actorTypeName = "Actor";
  entityData.description   = "quote\" backslash\\";
  entityData.description.push_back('\b');
  entityData.description.push_back('\f');
  entityData.description.push_back('\0');
  entityData.description.push_back('\x01');
  entityData.description.push_back('\n');
  entityData.description.push_back('\r');
  entityData.description.push_back('\t');
  entityData.screenBounds = Dali::Rect<float>{1.5f, 2.25f, 30.5f, 40.75f};
  entityData.windowBounds = entityData.screenBounds;

  std::string       presentation;
  const std::locale previousLocale = std::locale();
  std::locale::global(std::locale(previousLocale, new CommaDecimalPoint));
  DALI_TEST_CHECK(service.ToPresentation(entityData, presentation));
  std::locale::global(previousLocale);
  DALI_TEST_CHECK(presentation.find("\"x\":1.5") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("1,5") == std::string::npos);
  DALI_TEST_CHECK(presentation.find("\\\"") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("\\\\") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("\\b") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("\\f") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("\\u0000") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("\\u0001") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("\\n") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("\\r") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("\\t") != std::string::npos);
  for(const unsigned char c : presentation)
  {
    DALI_TEST_CHECK(c >= 0x20u);
  }

  entityData.screenBounds.y = std::numeric_limits<float>::quiet_NaN();
  presentation              = "stale";
  DALI_TEST_CHECK(!service.ToPresentation(entityData, presentation));
  DALI_TEST_EQUALS(presentation, "", TEST_LOCATION);

  END_TEST;
}
