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

#include <dali/internal/app-entity/common/entity-data-builder.h>
#include <dali/internal/app-entity/common/entity-data-service.h>

#include <cstdint>
#include <limits>
#include <string>

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
} // unnamed namespace

void utc_dali_entity_data_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_entity_data_cleanup(void)
{
  Integration::FocusedActorProvider::Unregister();
  test_return_value = TET_PASS;
}

int UtcDaliEntityDataMakeAndFocusedActorP(void)
{
  TestApplication application;
  Actor           actor = Actor::New();
  actor.SetProperty(Actor::Property::POSITION, Vector3(20.4f, 30.6f, 0.0f));
  actor.SetProperty(Actor::Property::SIZE, Vector2(100.2f, 50.8f));
  actor.SetProperty(Actor::Property::ENABLED, true);
  SetActorAnnotation(actor, "sample.entity", "Tizen.Entity.Sample");
  application.GetScene().Add(actor);
  application.SendNotification();
  application.Render();

  TestFocusedActorProvider provider(actor);

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
  const Dali::Bounds windowBounds = Internal::Adaptor::GetActorWindowBounds(actor);
  DALI_TEST_EQUALS(windowBounds.width, entityData.windowBounds.width, TEST_LOCATION);
  DALI_TEST_EQUALS(windowBounds.height, entityData.windowBounds.height, TEST_LOCATION);

  Internal::Adaptor::EntityDataService service(&provider);
  Internal::Adaptor::EntityData        focusedView;
  DALI_TEST_CHECK(service.GetFocusedEntityData(focusedView));
  DALI_TEST_EQUALS(focusedView.id, entityData.id, TEST_LOCATION);
  DALI_TEST_EQUALS(focusedView.annotation.entityId, "sample.entity", TEST_LOCATION);

  std::string presentation;
  DALI_TEST_CHECK(service.ToPresentation(focusedView, presentation));
  DALI_TEST_CHECK(presentation.find("sample.entity") != std::string::npos);
  DALI_TEST_CHECK(presentation.find("Tizen.Entity.Sample") != std::string::npos);

  END_TEST;
}

int UtcDaliEntityDataFindByIdRejectsOverflowP(void)
{
  TestApplication application;
  Actor           actor = Actor::New();
  application.GetScene().Add(actor);
  application.SendNotification();
  application.Render();

  TestFocusedActorProvider provider(actor);
  Internal::Adaptor::EntityDataService service(&provider);
  Internal::Adaptor::EntityData view;

  const uint64_t overflowId = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + actor.GetId() + 1u;
  DALI_TEST_CHECK(!service.FindById(std::to_string(overflowId), view));

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

  Actor emptyActor;
  TestFocusedActorProvider provider(emptyActor);
  Internal::Adaptor::EntityDataService service(&provider);
  Internal::Adaptor::EntityData focusedView;
  DALI_TEST_CHECK(!service.GetFocusedEntityData(focusedView));

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
  Internal::Adaptor::EntityData        view;

  view.id          = "42";
  view.type        = "Actor";
  view.description = "quote\" backslash\\";
  view.description.push_back('\b');
  view.description.push_back('\f');
  view.description.push_back('\0');
  view.description.push_back('\x01');
  view.description.push_back('\n');
  view.description.push_back('\r');
  view.description.push_back('\t');

  std::string presentation;
  DALI_TEST_CHECK(service.ToPresentation(view, presentation));
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

  view.screenBounds.y = std::numeric_limits<float>::quiet_NaN();
  presentation        = "stale";
  DALI_TEST_CHECK(!service.ToPresentation(view, presentation));
  DALI_TEST_EQUALS(presentation, "", TEST_LOCATION);

  END_TEST;
}
