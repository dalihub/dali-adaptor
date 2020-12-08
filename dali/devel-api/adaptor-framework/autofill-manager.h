#ifndef DALI_AUTOFILL_MANAGER_H
#define DALI_AUTOFILL_MANAGER_H

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
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/devel-api/adaptor-framework/autofill-group.h>

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class AutofillManager;
}
}


/**
 * @brief The AutofillManager class
 *
 * allows the application to fill out the user data, such as email, account and address previously saved.
 * Currently, Autofill is limited to text input box. Later it can be ScrollView, etc.
 *
 * Signals
 * | %Signal Name           | Method                              |
 * |------------------------|-------------------------------------|
 * | authenticationReceived | @ref AuthenticationReceivedSignal() |
 * | fillResponseReceived   | @ref FillResponseReceivedSignal()   |
 */
class DALI_ADAPTOR_API AutofillManager : public BaseHandle
{
public:


  // TODO : Need to update parameter and return value according to each Signal
  typedef Signal< void () > AuthSignalType;                ///< Authentication Received Signal
  typedef Signal< void ( AutofillItem ) > FillSignalType;  ///< Fill Response Received Signal
  typedef Signal< void () > ListSignalType;                ///< List Event Signal for multi-group

public:

  /**
   * @brief Retrieves a handle to the instance of AutofillManager.
   *
   * @return A handle to the AutofillManager.
   */
  static AutofillManager Get();


  /////////////////////////////////////////////// Autofill Item and Group ///////////////////////////////////////////////

  /**
   * @brief Creates AutofillItem instance.
   *
   * @param[in] id A unique ID that does not always change on each launching
   * @param[in] label An auxiliary means to guess heuristically what data is
   * @param[in] hint The Hint - id (username), name, password, phone, credit card number, organization, and so on
   * @param[in] isSensitive Whether this AutofillItem is a sensitive data or not
   * @return A public handle to the newly allocated AutofillItem
   */
  Dali::AutofillItem CreateAutofillItem( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool isSensitive );

  /**
   * @brief Creates AutofillGroup instance.
   *
   * @param[in] groupId A unique ID value of each AutofillGroup
   * @return A public handle to the newly allocated AutofillGroup
   */
  Dali::AutofillGroup CreateAutofillGroup( const std::string& groupId );


  /////////////////////////////////////////////// Autofill Authentication Information ///////////////////////////////////////////////

  /**
   * @brief Gets the 'autofill data present' attribute in autofill authentication information.
   *
   * @return True if Autofill data is present
   */
  bool IsAutofillDataPresent() const;

  /**
   * @brief Gets the 'authentication needed' attribute in autofill authentication information.
   *
   * @return True if the authentication is needed in the current Autofill process.
   */
  bool IsAuthenticationNeeded() const;

  /**
   * @brief Gets the service name in autofill authentication information.
   *
   * @return The autofill authentication service name
   */
  const std::string& GetAuthenticationServiceName() const;

  /**
   * @brief Gets the service message in autofill authentication information.
   *
   * @return The autofill authentication service message
   */
  const std::string& GetAuthenticationServiceMessage() const;

  /**
   * @brief Gets the service logo image path in autofill authentication information.
   *
   * @return The autofill authentication service logo image path
   */
  const std::string& GetAuthenticationServiceImagePath() const;


  /////////////////////////////////////////////// Autofill Fill Response ///////////////////////////////////////////////

  /**
   * @brief Gets the autofill ID in an autofill fill response item.
   *
   * @return The autofill fill response item ID
   */
  const std::string& GetFillItemId() const;

  /**
   * @brief Gets the presentation text in an autofill fill response item.
   *
   * @return The presentation text
   */
  const std::string& GetFillItemPresentationText() const;

  /**
   * @brief Gets the autofill value in an autofill fill response item.
   *
   * @return The autofill fill response item value
   */
  const std::string& GetFillItemValue() const;

  /**
   * @brief Stores the current Autofill data.
   *
   * @param[in] group The AutofillGroup to store the data
   */
  void SaveAutofillData( Dali::AutofillGroup group );

public:
  // Signals
  /**
   * @brief This is emitted when the authentication is needed and AutofillManager gets the information.
   *
   * @return The signal containing the received data
   */
  AuthSignalType& AuthenticationReceivedSignal();

  /**
   * @brief This is emitted when AutofillManager receives the fill response.
   *
   * @return The signal containing the received data
   */
  FillSignalType& FillResponseReceivedSignal();

  /**
   * @brief This is emitted when the list for multi fill response group is needed.
   *
   * @return The signal containing the received data
   */
  ListSignalType& ListEventSignal();

  // Construction & Destruction
  /**
   * @brief Constructor.
   *
   * Create an uninitialized handle.
   * This can be initialized by calling AutofillManager::Get().
   */
  AutofillManager();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~AutofillManager();

  /**
   * @brief This constructor is used by AutofillManager::Get().
   *
   * @param[in] autofillManager A pointer to the AutofillManager.
   */
  explicit DALI_INTERNAL AutofillManager( Internal::Adaptor::AutofillManager* autofillManager );

};

} // namespace Dali

#endif // DALI_AUTOFILL_MANAGER_H
