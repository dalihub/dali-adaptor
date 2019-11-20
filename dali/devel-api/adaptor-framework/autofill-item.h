#ifndef DALI_AUTOFILL_ITEM_H
#define DALI_AUTOFILL_ITEM_H

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class AutofillItem;
}
}


/**
 * @brief The AutofillItem class
 *
 * is used to pass on data from the AutofillItem.
 * AutofillItem includes Id, Label, Autofill Hint, and whether it is sensitive or not.
 */
class DALI_ADAPTOR_API AutofillItem : public BaseHandle
{
public:

  /**
   * @brief Enumeration for hint of the autofill item.
   */
  enum class Hint
  {
    CREDIT_CARD_EXPIRATION_DATE,      ///< Autofill hint for a credit card expiration date
    CREDIT_CARD_EXPIRATION_DAY,       ///< Autofill hint for a credit card expiration day
    CREDIT_CARD_EXPIRATION_MONTH,     ///< Autofill hint for a credit card expiration month
    CREDIT_CARD_EXPIRATION_YEAR,      ///< Autofill hint for a credit card expiration year
    CREDIT_CARD_NUMBER,               ///< Autofill hint for a credit card number
    EMAIL_ADDRESS,                    ///< Autofill hint for an email address
    NAME,                             ///< Autofill hint for a user's real name
    PHONE,                            ///< Autofill hint for a phone number
    POSTAL_ADDRESS,                   ///< Autofill hint for a postal address
    POSTAL_CODE,                      ///< Autofill hint for a postal code
    ID,                               ///< Autofill hint for a user's ID
    PASSWORD,                         ///< Autofill hint for password
    CREDIT_CARD_SECURITY_CODE         ///< Autofill hint for a credit card security code
  };

public:

  /**
   * @brief Creates an uninitialized AutofillItem.
   *
   * To create AutofillItem instance, please refer to Dali::AutofillManager::CreateAutofillItem().
   */
  AutofillItem();

  /**
   * @brief AutofillItem Destructor.
   */
  ~AutofillItem();

  /**
    * @brief Copy constructor.
    *
    * @param[in] item AutofillItem to copy. The copied player will point at the same implementation
    */
   AutofillItem( const AutofillItem& item );

  /**
    * @brief Assignment operator.
    *
    * @param[in] item The AutofillItem to assign from.
    * @return The updated AutofillItem.
    */
   AutofillItem& operator=( const AutofillItem& item );

  /**
   * @brief Downcast a handle to AutofillItem handle.
   *
   * If handle points to a AutofillItem the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object
   * @return Handle to a AutofillItem or an uninitialized handle
   */
  static AutofillItem DownCast( BaseHandle handle );

  /**
   * @brief Equality operator.
   *
   * @param[in] rhs The AutofillItem structure to test against
   * @return True if AutofillItems are equal
   */
  bool operator==( const AutofillItem& rhs ) const
  {
    if( &rhs == this )
    {
      return true;
    }
    return false;
  }

  /**
   * @brief Gets AutofillItem Id.
   *
   * @return AutofillItem Id
   */
  const std::string& GetId() const;

  /**
   * @brief Gets AutofillItem Label.
   *
   * @return AutofillItem Label
   */
  const std::string& GetLabel() const;

  /**
   * @brief Gets AutofillItem Hint.
   *
   * @return AutofillItem Hint
   */
  Dali::AutofillItem::Hint GetHint() const;

  /**
   * @brief Gets whether AutofillItem is sensitive data or not.
   *
   * @return True if the AutofillItem is sensitive.
   */
  bool IsSensitiveData() const;

  /**
   * @brief Sets AutofillItem value for saving.
   *
   * @param[in] value The value for saving
   */
  void SetSaveValue( const std::string& value );

  /**
   * @brief Gets the saved value of AutofillItem.
   *
   * @return The saved value
   */
  const std::string& GetSaveValue() const;

  /**
   * @brief Gets the presentation text with a index of the list.
   *
   * @param index The index for the presentation text list
   * @return The presentation text to show up for the fill value
   */
  const std::string& GetPresentationText( int index ) const;

  /**
   * @brief Gets the value to be filled with a index of the list.
   *
   * @param index The index for the value list
   * @return The value to be filled
   */
  const std::string& GetFillValue( int index ) const;

  /**
   * @brief Clears the presentation text list.
   */
  void ClearPresentationTextList();

  /**
   * @brief Clears the value list.
   */
  void ClearFillValueList();

  /**
   * @brief Gets the number of fill value in the list.
   *
   * @return The number of fill value in the list
   */
  unsigned int GetFillValueCount();

public: // Not intended for application developers

  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL AutofillItem( Internal::Adaptor::AutofillItem* internal );

};

} // namespace Dali

#endif // DALI_AUTOFILL_ITEM_H
