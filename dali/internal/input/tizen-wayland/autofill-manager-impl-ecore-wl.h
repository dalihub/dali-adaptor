#ifndef DALI_INTERNAL_AUTOFILL_MANAGER_IMPL_ECORE_WL_H
#define DALI_INTERNAL_AUTOFILL_MANAGER_IMPL_ECORE_WL_H

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
#ifdef CAPI_AUTOFILL_SUPPORT
#include <autofill.h>
#endif // CAPI_AUTOFILL_SUPPORT
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/input/common/autofill-manager-impl.h>
#include <dali/internal/input/tizen-wayland/autofill-group-impl-ecore-wl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class AutofillManagerEcoreWl : public Dali::Internal::Adaptor::AutofillManager
{

public:

  using AuthSignalType = Dali::AutofillManager::AuthSignalType;
  using FillSignalType = Dali::AutofillManager::FillSignalType;
  using ListSignalType = Dali::AutofillManager::ListSignalType;

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
  void ConnectCallbacks() override;

#ifdef CAPI_AUTOFILL_SUPPORT

  /**
   * @brief Gets Autofill framework handle
   * @return Autofill framework handle, which type is 'autofill_h"
   */
  autofill_h GetAutofillHandle();

  /**
   * @brief Implement to receives Autofill Authentication Information callback.
   *
   * In this method, gets authentication information and emits the signal to check the authentication.
   * @param authInfoHandle The autofill authentication information handle
   * @param data The data from authentication information callback
   */
  void ReceiveAuthInfo( autofill_auth_info_h authInfoHandle, void *data );

  /**
   * @brief Implement to fill out the data.
   *
   * This method would emit the signal to fill out the data.
   * @param itemHandle The autofill fill response item handle
   * @param userData The data from fill response callback
   */
  void FillGroupItem( autofill_fill_response_item_h itemHandle, void *userData );

  /**
   * @brief Implement to set multiple group information.
   *
   * This method would emit the signals to make lists and fill out the data.
   * @param itemHandle The autofill fill response item handle
   * @param userData The data from fill response callback
   */
  void FillMultipleGroupItem( autofill_fill_response_item_h itemHandle, void *userData );
#endif // CAPI_AUTOFILL_SUPPORT


  /////////////////////////////////////////////// Autofill Item and Group ///////////////////////////////////////////////

  /**
   * @copydoc Dali::AutofillManager::CreateAutofillItem()
   */
  Dali::AutofillItem CreateAutofillItem( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool isSensitive ) override;

  /**
   * @copydoc Dali::AutofillManager::CreateAutofillGroup()
   */
  Dali::AutofillGroup CreateAutofillGroup( const std::string& groupId ) override;


  /////////////////////////////////////////////// Autofill Authentication Information ///////////////////////////////////////////////

  /**
   * @copydoc Dali::AutofillManager::IsAutofillDataPresent()
   */
  bool IsAutofillDataPresent() const override;

  /**
   * @copydoc Dali::AutofillManager::IsAuthenticationNeeded()
   */
  bool IsAuthenticationNeeded() const override;

  /**
   * @copydoc Dali::AutofillManager::GetAuthenticationServiceName()
   */
  const std::string& GetAuthenticationServiceName() const override;

  /**
   * @copydoc Dali::AutofillManager::GetAuthenticationServiceMessage()
   */
  const std::string& GetAuthenticationServiceMessage() const override;

  /**
   * @copydoc Dali::AutofillManager::GetAuthenticationServiceImagePath()
   */
  const std::string& GetAuthenticationServiceImagePath() const override;


  /////////////////////////////////////////////// Autofill Fill Response ///////////////////////////////////////////////

  /**
   * @copydoc Dali::AutofillManager::GetFillItemId()
   */
  const std::string& GetFillItemId() const override;

  /**
   * @copydoc Dali::AutofillManager::GetFillItemPresentationText()
   */
  const std::string& GetFillItemPresentationText() const override;

  /**
   * @copydoc Dali::AutofillManager::GetFillItemValue()
   */
  const std::string& GetFillItemValue() const override;

  /**
   * @copydoc Dali::AutofillManager::SaveAutofillData()
   */
  void SaveAutofillData( Dali::AutofillGroup group ) override;

public: // Signals

  /**
   * @copydoc Dali::AutofillManager::AuthenticationReceivedSignal()
   */
  AuthSignalType& AuthenticationReceivedSignal() override;

  /**
   * @copydoc Dali::AutofillManager::FillResponseReceivedSignal()
   */
  FillSignalType& FillResponseReceivedSignal() override;

  /**
   * @copydoc Dali::AutofillManager::ListEventSignal()
   */
  ListSignalType& ListEventSignal() override;

  /**
   * Connects a callback function with the object's signals.
   * @param[in] object The object providing the signal.
   * @param[in] tracker Used to disconnect the signal.
   * @param[in] signalName The signal to connect to.
   * @param[in] functor A newly allocated FunctorDelegate.
   * @return True if the signal was connected.
   * @post If a signal was connected, ownership of functor was passed to CallbackBase. Otherwise the caller is responsible for deleting the unused functor.
   */
  static bool DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor );

private:
  /**
   * Context created the first time and kept until deleted.
   */
  void CreateContext() override;

  /**
   * Delete Autofill context.
   */
  void DeleteContext() override;

private:
  /**
   * Constructor.
   */
  explicit AutofillManagerEcoreWl();

protected:
  /**
   * Destructor.
   */
  ~AutofillManagerEcoreWl();

private:
  // Undefined copy constructor
  explicit AutofillManagerEcoreWl( const AutofillManagerEcoreWl& autofillManager ) = delete;

  // Undefined assignment operator
  AutofillManagerEcoreWl& operator=( AutofillManagerEcoreWl& autofillManager ) = delete;

private:
#ifdef CAPI_AUTOFILL_SUPPORT
  autofill_h mAutofillHandle;        ///< The Autofill framework handle
#endif // CAPI_AUTOFILL_SUPPORT
  Dali::AutofillGroup mAutofillGroup;

  std::vector<Dali::AutofillGroup> mAutofillGroupList;     ///< The list to manage AutofillGroup
  std::vector<Dali::AutofillItem> mAutofillItemList;       ///< The list to manage AutofillItem

private:
  AuthSignalType mAuthReceivedSignal;                      ///< Authentication Received Signal
  FillSignalType mFillReceivedSignal;                      ///< Fill Response Received Signal
  ListSignalType mListReceivedSignal;                      ///< List Received Signal

private:
  std::string mAuthenticationServiceName;                  ///< The autofill authentication service name
  std::string mAuthenticationServiceMessage;               ///< The autofill authentication service message
  std::string mAuthenticationServiceImagePath;             ///< The autofill authentication service logo image path
  std::string mFillItemId;                                 ///< The autofill fill response item ID
  std::string mFillItemPresentationText;                   ///< The autofill fill response item presentation text
  std::string mFillItemValue;                              ///< The autofill fill response item value (input data)

  bool mIsDataPresent;                                      ///< The autofill data presence
  bool mIsAuthNeeded;                                       ///< The authentication need

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_AUTOFILL_MANAGER_IMPL_ECORE_WL_H
