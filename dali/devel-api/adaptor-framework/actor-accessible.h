#ifndef DALI_ADAPTOR_ACTOR_ACCESSIBLE_H
#define DALI_ADAPTOR_ACTOR_ACCESSIBLE_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/object/weak-handle.h>
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/collection.h>
#include <dali/devel-api/atspi-interfaces/component.h>

namespace Dali::Accessibility
{
class DALI_ADAPTOR_API ActorAccessible : public virtual Accessible,
                                         public virtual Collection,
                                         public virtual Component,
                                         public Dali::ConnectionTracker
{
public:
  ActorAccessible() = delete;

  ActorAccessible(Actor actor);

  /**
   * @copydoc Dali::Accessibility::Accessible::GetName()
   */
  std::string GetName() const override;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetDescription()
   */
  std::string GetDescription() const override;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetParent()
   */
  Accessible* GetParent() final;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetChildCount()
   */
  std::size_t GetChildCount() const final;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetChildren()
   */
  std::vector<Accessible*> GetChildren() final;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetChildAtIndex()
   */
  Accessible* GetChildAtIndex(std::size_t index) final;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetIndexInParent()
   */
  std::size_t GetIndexInParent() final;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetInternalActor()
   */
  Dali::Actor GetInternalActor() final;

  /**
   * @copydoc Dali::Accessibility::Component::GetLayer()
   */
  ComponentLayer GetLayer() const override;

  /**
   * @copydoc Dali::Accessibility::Component::GetMdiZOrder()
   */
  std::int16_t GetMdiZOrder() const override;

  /**
   * @copydoc Dali::Accessibility::Component::GetAlpha()
   */
  double GetAlpha() const override;

  /**
   * @copydoc Dali::Accessibility::Component::IsScrollable()
   */
  bool IsScrollable() const override;

  /**
   * @copydoc Dali::Accessibility::Component::GetExtents()
   */
  Dali::Rect<> GetExtents(CoordinateType type) const override;

  /**
   * @brief Notifies this object that its children have changed.
   *
   * This is useful if you maintain a custom collection of children that are not derived from
   * ActorAccessible and the contents or order of elements in that collection change.
   *
   * @see DoGetChildren()
   */
  void OnChildrenChanged();

protected:
  Dali::Actor Self() const
  {
    auto handle = mSelf.GetHandle();

    // It is a bug if the Accessible outlives its Actor
    DALI_ASSERT_ALWAYS(handle);

    return handle;
  }

  /**
   * @brief Populates the collection of children of this Accessible.
   *
   * The default implementation retrieves the children from the Actor hierarchy. Override this if
   * you want to report other objects as children, either instead of, or together with the
   * dependent Actor-derived Accessibles. Remember to call OnChildrenChanged() if you want your
   * implementation of DoGetChildren() to be called again (in case your custom collection of
   * children changes).
   *
   * @param[out] children The initially empty vector to insert children into
   *
   * @note GetChildCound(), GetChildren() and GetChildAtIndex() are not available for overriding,
   * but they respect the children collection reported by this method.
   *
   * @see OnChildrenChanged()
   * @see GetChildCount()
   * @see GetChildren()
   * @see GetChildAtIndex()
   */
  virtual void DoGetChildren(std::vector<Accessible*>& children);

  /**
   * @copydoc Dali::Accessibility::Accessible::UpdateAttributes()
   */
  void UpdateAttributes(Attributes& attributes) const override;

private:
  // Extra overload for OnChildrenChanged() to connect to signals directly
  void OnChildrenChanged(Dali::Actor);

  // Ensures children are up to date (calls DoGetChildren() if necessary)
  void UpdateChildren();

  Dali::WeakHandle<Dali::Actor> mSelf;
  std::vector<Accessible*>      mChildren;
  bool                          mChildrenDirty;
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ACTOR_ACCESSIBLE_H
