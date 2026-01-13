#ifndef DALI_ADAPTOR_ACTOR_ACCESSIBLE_H
#define DALI_ADAPTOR_ACTOR_ACCESSIBLE_H

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
#include <dali/devel-api/object/base-object-observer.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/object/weak-handle.h>
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/collection.h>
namespace Dali::Accessibility
{
class DALI_ADAPTOR_API ActorAccessible : public Dali::Accessibility::Accessible,
                                         public Dali::ConnectionTracker,
                                         public Dali::BaseObjectObserver,
                                         public std::enable_shared_from_this<ActorAccessible>
{
public:
  ActorAccessible() = delete;

  ActorAccessible(Actor actor);

  // BaseObjectObserver::ObjectDestroyed
  void ObjectDestroyed() override;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetAddress()
   */
  Address GetAddress() const override;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetName()
   */
  std::string GetName() const override;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetDescription()
   */
  std::string GetDescription() const override;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetValue()
   */
  std::string GetValue() const override;

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
  Dali::Actor GetInternalActor() const final;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetStringProperty()
   */
  std::string GetStringProperty(std::string propertyName) const override;

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
  Dali::Rect<float> GetExtents(CoordinateType type) const override;

  /**
   * @brief Notifies this object that its children have changed.
   *
   * This is useful if you maintain a custom collection of children that are not derived from
   * ActorAccessible and the contents or order of elements in that collection change.
   *
   * @see DoGetChildren()
   */
  void OnChildrenChanged();

  /**
   * @brief Helper function for emiting active-descendant-changed event.
   *
   * @param[in] child The child of the object
   */
  void EmitActiveDescendantChanged(Accessible* child);

  /**
   * @brief Helper function for emiting state-changed event.
   *
   * @param[in] state The accessibility state (SHOWING, HIGHLIGHTED, etc)
   * @param[in] newValue Whether the state value is changed to new value or not.
   * @param[in] reserved Reserved. (TODO : Currently, this argument is not implemented in dali)
   *
   * @note The second argument determines which value is depending on State.
   * For instance, if the state is PRESSED, newValue means isPressed or isSelected.
   * If the state is SHOWING, newValue means isShowing.
   */
  void EmitStateChanged(State state, int newValue, int reserved = 0);

  /**
   * @brief Helper function for emiting bounds-changed event.
   *
   * @param rect The rectangle for changed bounds
   */
  void EmitBoundsChanged(Rect<int> rect);

  /**
   * @brief Emits "showing" event.
   * The method informs accessibility clients about "showing" state.
   *
   * @param[in] isShowing The flag pointing if object is showing
   */
  void EmitShowing(bool isShowing);

  /**
   * @brief Emits "visible" event.
   * The method informs accessibility clients about "visible" state.
   *
   * @param[in] isVisible The flag pointing if object is visible
   */
  void EmitVisible(bool isVisible);

  /**
   * @brief Emits "highlighted" event.
   * The method informs accessibility clients about "highlighted" state.
   *
   * @param[in] isHighlighted The flag pointing if object is highlighted
   */
  void EmitHighlighted(bool isHighlighted);

  /**
   * @brief Emits "focused" event.
   * The method informs accessibility clients about "focused" state.
   *
   * @param[in] isFocused The flag pointing if object is focused
   */
  void EmitFocused(bool isFocused);

  /**
   * @brief Emits "text inserted" event.
   *
   * @param[in] position The cursor position
   * @param[in] length The text length
   * @param[in] content The inserted text
   */
  void EmitTextInserted(unsigned int position, unsigned int length, const std::string& content);

  /**
   * @brief Emits "text deleted" event.
   *
   * @param[in] position The cursor position
   * @param[in] length The text length
   * @param[in] content The deleted text
   */
  void EmitTextDeleted(unsigned int position, unsigned int length, const std::string& content);

  /**
   * @brief Emits "cursor moved" event.
   *
   * @param[in] cursorPosition The new cursor position
   */
  void EmitTextCursorMoved(unsigned int cursorPosition);

  /**
   * @brief Emits "MoveOuted" event.
   *
   * @param[in] type moved out of screen type
   */
  void EmitMovedOutOfScreen(ScreenRelativeMoveType type);

  /**
   * @brief Emits "ScrollStarted" event.
   *
   */
  void EmitScrollStarted();

  /**
   * @brief Emits "ScrollFinished" event.
   *
   */
  void EmitScrollFinished();

  /**
   * @brief Emits "highlighted" event.
   *
   * @param[in] event The enumerated window event
   * @param[in] detail The additional parameter which interpretation depends on chosen event
   */
  void Emit(WindowEvent event, unsigned int detail = 0);

  /**
   * @brief Emits property-changed event.
   *
   * @param[in] event Property changed event
   **/
  void Emit(ObjectPropertyChangeEvent event);

  /**
   * @brief Re-emits selected states of an Accessibility Object.
   *
   * @param[in] states The chosen states to re-emit
   * @param[in] isRecursive If true, all children of the Accessibility object will also re-emit the states
   */
  void NotifyAccessibilityStateChange(Dali::Accessibility::States states, bool isRecursive);

  /**
   * @brief Clears internal cache data on bridge down
   */
  void ClearCache();

  /**
   * @brief Checks if the accessible object can be accepted even with zero size.
   */
  bool CanAcceptZeroSize() const;

protected:
  Dali::Actor Self() const
  {
    auto handle = mSelf.GetHandle();

    // It is a bug if the Accessible outlives its Actor
    DALI_ASSERT_ALWAYS(handle);

    return handle;
  }

  /**
   * @copydoc Dali::Accessibility::Accessible::DoGetInterfaces()
   */
  virtual AtspiInterfaces DoGetInterfaces() const override;

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

private:
  // Extra overload for OnChildrenChanged() to connect to signals directly
  void OnChildrenChanged(Dali::Actor);

  // Ensures children are up to date (calls DoGetChildren() if necessary)
  void UpdateChildren();

  Dali::WeakHandle<Dali::Actor> mSelf;
  std::vector<Accessible*>      mChildren;
  bool                          mChildrenDirty;
  bool                          mIsBeingDestroyed;
  const uint32_t                mActorId;
  std::map<State, int>          mLastEmittedState;
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ACTOR_ACCESSIBLE_H
