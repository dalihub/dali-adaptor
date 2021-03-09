#ifndef DALI_INTERNAL_AUTOFILL_MANAGER_IMPL_H
#define DALI_INTERNAL_AUTOFILL_MANAGER_IMPL_H

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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/autofill-manager.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class AutofillManager : public Dali::BaseObject
{

public:

  /**
   * @brief Gets the AutofillManager instance
   *
   * It creates the instance if it has not already been created.
   * Internally, a check should be made using IsAvailable() before this is called as we do not want
   * to create an instance if not needed by applications.
   * @see IsAvailable()
   */
  static Dali::AutofillManager Get();

  /**
   * @brief Connects Callbacks required for Autofill daemon.
   */
  virtual void ConnectCallbacks() = 0;

  /**
   * @copydoc Dali::AutofillManager::CreateAutofillItem()
   */
  virtual Dali::AutofillItem CreateAutofillItem( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool isSensitive ) = 0;

  /**
   * @copydoc Dali::AutofillManager::CreateAutofillGroup()
   */
  virtual Dali::AutofillGroup CreateAutofillGroup( const std::string& groupId ) = 0;

  /**
   * @copydoc Dali::AutofillManager::IsAutofillDataPresent()
   */
  virtual bool IsAutofillDataPresent() const = 0;

  /**
   * @copydoc Dali::AutofillManager::IsAuthenticationNeeded()
   */
  virtual bool IsAuthenticationNeeded() const = 0;

  /**
   * @copydoc Dali::AutofillManager::GetAuthenticationServiceName()
   */
  virtual const std::string& GetAuthenticationServiceName() const = 0;

  /**
   * @copydoc Dali::AutofillManager::GetAuthenticationServiceMessage()
   */
  virtual const std::string& GetAuthenticationServiceMessage() const = 0;

  /**
   * @copydoc Dali::AutofillManager::GetAuthenticationServiceImagePath()
   */
  virtual const std::string& GetAuthenticationServiceImagePath() const = 0;

  /**
   * @copydoc Dali::AutofillManager::GetFillItemId()
   */
  virtual const std::string& GetFillItemId() const = 0;

  /**
   * @copydoc Dali::AutofillManager::GetFillItemPresentationText()
   */
  virtual const std::string& GetFillItemPresentationText() const = 0;

  /**
   * @copydoc Dali::AutofillManager::GetFillItemValue()
   */
  virtual const std::string& GetFillItemValue() const = 0;

  /**
   * @copydoc Dali::AutofillManager::SaveAutofillData()
   */
  virtual void SaveAutofillData( Dali::AutofillGroup group ) = 0;

public: // Signals

  /**
   * @copydoc Dali::AutofillManager::AuthenticationReceivedSignal()
   */
  virtual Dali::AutofillManager::AuthSignalType& AuthenticationReceivedSignal() { return mAuthReceivedSignal; }

  /**
   * @copydoc Dali::AutofillManager::FillResponseReceivedSignal()
   */
  virtual Dali::AutofillManager::FillSignalType& FillResponseReceivedSignal() { return mFillReceivedSignal; }

  /**
   * @copydoc Dali::AutofillManager::ListEventSignal()
   */
  virtual Dali::AutofillManager::ListSignalType& ListEventSignal() { return mListReceivedSignal; }

private:
  /**
   * Context created the first time and kept until deleted.
   */
  virtual void CreateContext() = 0;

  /**
   * Delete Autofill context.
   */
  virtual void DeleteContext() = 0;

public:
  /**
   * Constructor.
   */
  AutofillManager() = default;

protected:
  /**
   * Destructor.
   */
  ~AutofillManager() = default;

private:
  // Undefined copy constructor
  AutofillManager( const AutofillManager& autofillManager ) = delete;

  // Undefined assignment operator
  AutofillManager& operator=( AutofillManager& autofillManager ) = delete;

private:
  Dali::AutofillManager::AuthSignalType mAuthReceivedSignal;                      ///< Authentication Received Signal
  Dali::AutofillManager::FillSignalType mFillReceivedSignal;                      ///< Fill Response Received Signal
  Dali::AutofillManager::ListSignalType mListReceivedSignal;                      ///< List Received Signal


};

inline static Internal::Adaptor::AutofillManager& GetImplementation(Dali::AutofillManager& autofillManager)
{
  DALI_ASSERT_ALWAYS( autofillManager && "AutofillManager handle is empty" );

  BaseObject& handle = autofillManager.GetBaseObject();

  return static_cast<Internal::Adaptor::AutofillManager&>(handle);
}

inline static const Internal::Adaptor::AutofillManager& GetImplementation(const Dali::AutofillManager& autofillManager)
{
  DALI_ASSERT_ALWAYS( autofillManager && "AutofillManager handle is empty" );

  const BaseObject& handle = autofillManager.GetBaseObject();

  return static_cast<const Internal::Adaptor::AutofillManager&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_AUTOFILL_MANAGER_IMPL_H
