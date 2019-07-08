#ifndef DALI_AUTOFILL_GROUP_H
#define DALI_AUTOFILL_GROUP_H

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
#include <dali/devel-api/adaptor-framework/autofill-item.h>

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class AutofillGroup;
}
}


/**
 * @brief The AutofillGroup class
 *
 * is used to group AutofillItems together.
 */
class DALI_ADAPTOR_API AutofillGroup : public BaseHandle
{
public:

   /**
    * @brief Creates an uninitialized AutofillGroup.
    *
    * To create AutofillGroup instance, please refer to Dali::AutofillManager::CreateAutofillGroup().
    */
   AutofillGroup();

  /**
   * @brief AutofillGroup Destructor.
   */
  ~AutofillGroup();

  /**
    * @brief Copy constructor.
    *
    * @param[in] group AutofillGroup to copy. The copied player will point at the same implementation
    */
   AutofillGroup( const AutofillGroup& group );

  /**
    * @brief Assignment operator.
    *
    * @param[in] group The AutofillGroup to assign from.
    * @return The updated AutofillGroup.
    */
   AutofillGroup& operator=( const AutofillGroup& group );

  /**
   * @brief Downcast a handle to AutofillGroup handle.
   *
   * If handle points to a AutofillGroup the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object
   * @return Handle to a AutofillGroup or an uninitialized handle
   */
  static AutofillGroup DownCast( BaseHandle handle );

  /**
   * @brief Equality operator.
   *
   * @param[in] rhs The AutofillGroup structure to test against
   * @return True if AutofillGroups are equal
   */
  bool operator==( const AutofillGroup& rhs ) const
  {
    if( &rhs == this )
    {
      return true;
    }
    return false;
  }

  /**
   * @brief Gets AutofillGroup unique Id.
   *
   * @return AutofillGroup ID
   */
  const std::string& GetId() const;

  /**
   * @brief Adds AutofillItem to AutofillGroup itself in order to group.
   *
   * @param[in] item AutofillItem instance to be included in AutofillGroup
   */
  void AddAutofillItem( Dali::AutofillItem item );

  /**
   * @brief Gets AutofillItem instance according to the id.
   *
   * @param[in] id AutofillItem Id to get from AutofillGroup List
   * @return AutofillItem instance to match for Id
   */
  Dali::AutofillItem GetAutofillItem( const std::string& id );

  /**
   * @brief Stores Autofill data in autofill group.
   */
  void SaveAutofillData();

  /**
   * @brief Requests and receives autofill authentication information.
   */
  void RequestAuthentication();

  /**
   * @brief Sends fill request to fill out the data.
   */
  void SendFillRequest();

public: // Not intended for application developers

  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL AutofillGroup( Internal::Adaptor::AutofillGroup* internal );

};


} // namespace Dali

#endif // DALI_AUTOFILL_GROUP_H
