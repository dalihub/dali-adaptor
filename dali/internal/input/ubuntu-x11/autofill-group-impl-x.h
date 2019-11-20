#ifndef DALI_INTERNAL_AUTOFILL_GROUP_IMPL_X_H
#define DALI_INTERNAL_AUTOFILL_GROUP_IMPL_X_H

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
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/input/common/autofill-group-impl.h>
#include <dali/internal/input/ubuntu-x11/autofill-item-impl-x.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{


/**
 * @brief This class is used to group AutofillItems.
 */
class AutofillGroupX : public Dali::Internal::Adaptor::AutofillGroup
{

public:

  /**
   * @brief Constructor.
   *
   * @param[in] groupId A unique ID value of each AutofillGroup
   * @return A public handle to the newly allocated AutofillGroup.
   */
  static Dali::AutofillGroup New( const std::string& groupId );

  /**
   * @brief Initialize AutofillGroup constructor.
   */
  void Initialize() override;

  /**
   * @copydoc Dali::AutofillGroup::GetId()
   */
  const std::string& GetId() const override;

  /**
   * @copydoc Dali::AutofillGroup::AddAutofillItem()
   */
  void AddAutofillItem( Dali::AutofillItem item ) override;

  /**
   * @copydoc Dali::AutofillGroup::GetAutofillItem()
   */
  Dali::AutofillItem GetAutofillItem( const std::string& id ) override;

  /**
   * @brief Clears all lists of AutofillItem added in AutofillGroup.
   */
  void ClearAutofillItemList() override;

  /**
   * @copydoc Dali::AutofillGroup::SaveAutofillData()
   */
  void SaveAutofillData() override;

  /**
   * @copydoc Dali::AutofillGroup::RequestAuthentication()
   */
  void RequestAuthentication() override;

  /**
   * @copydoc Dali::AutofillGroup::SendFillRequest()
   */
  void SendFillRequest() override;

private:
  /**
   * Constructor.
   */
  explicit AutofillGroupX( const std::string groupId );

protected:
  /**
   * Destructor.
   */
  ~AutofillGroupX();

private:
  // Undefined copy constructor
  explicit AutofillGroupX( const AutofillGroupX& autofillGroup ) = delete;

  // Undefined assignment operator
  AutofillGroupX& operator=( AutofillGroupX& autofillGroup ) = delete;

private:
  std::vector<Dali::AutofillItem> mAutofillItemList; ///< The List to add AutofillItem
  std::string mGroupId;  ///< The AutofillGroup ID

};


} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_AUTOFILL_GROUP_IMPL_X_H
