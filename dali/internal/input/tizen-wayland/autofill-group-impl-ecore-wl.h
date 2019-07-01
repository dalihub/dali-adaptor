#ifndef DALI_INTERNAL_AUTOFILL_GROUP_IMPL_ECORE_WL_H
#define DALI_INTERNAL_AUTOFILL_GROUP_IMPL_ECORE_WL_H

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
#include <dali/internal/input/common/autofill-group-impl.h>
#include <dali/internal/input/tizen-wayland/autofill-item-impl-ecore-wl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{


/**
 * @brief This class is used to group AutofillItems.
 */
class AutofillGroupEcoreWl : public Dali::Internal::Adaptor::AutofillGroup
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

#ifdef CAPI_AUTOFILL_SUPPORT
  /**
   * @brief Gets Autofill framework group handle
   * @return Autofill framework group handle, which type is 'autofill_view_info_h'
   */
  autofill_view_info_h GetAutofillGroupHandle();

  /**
   * @brief Gets Autofill framework save group handle
   * @return Autofill framework save group handle, which type is 'autofill_save_view_info_h'
   */
  autofill_save_view_info_h GetAutofillSaveGroupHandle();
#endif // CAPI_AUTOFILL_SUPPORT

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
  explicit AutofillGroupEcoreWl( const std::string groupId );

protected:
  /**
   * Destructor.
   */
  ~AutofillGroupEcoreWl();

private:
  // Undefined copy constructor
  explicit AutofillGroupEcoreWl( const AutofillGroupEcoreWl& autofillGroup ) = delete;

  // Undefined assignment operator
  AutofillGroupEcoreWl& operator=( AutofillGroupEcoreWl& autofillGroup ) = delete;


private:
#ifdef CAPI_AUTOFILL_SUPPORT
  autofill_view_info_h mAutofillGroupHandle;  ///< The Autofill Framework group handle
  autofill_save_view_info_h mAutofillSaveGroupHandle;
#endif // CAPI_AUTOFILL_SUPPORT

  std::vector<Dali::AutofillItem> mAutofillItemList; ///< The List to add AutofillItem
  std::string mGroupId;  ///< The AutofillGroup ID

};


} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_AUTOFILL_GROUP_IMPL_ECORE_WL_H
