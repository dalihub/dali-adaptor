#ifndef DALI_INTERNAL_AUTOFILL_ITEM_IMPL_ECORE_WL_H
#define DALI_INTERNAL_AUTOFILL_ITEM_IMPL_ECORE_WL_H

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
#include <dali/internal/input/common/autofill-item-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{


/**
 * @brief This class is used to pass on data from the AutofillItem of control.
 */
class AutofillItemEcorewWl : public Dali::Internal::Adaptor::AutofillItem
{

public:

  /**
   * @brief Constructor.
   *
   * @param[in] id A unique ID for this AutofillItem
   * @param[in] label An auxiliary means to guess what data is
   * @param[in] hint The hint - id (username), name, password, phone, credit card number, organization, and so on
   * @param[in] sensitiveData Whether this AutofillItem is a sensitive data or not. (The default is false)
   * @return A public handle to the newly allocated AutofillItem
   */
  static Dali::AutofillItem New( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool sensitiveData );

  /**
   * @brief Initialize AutofillItem constructor.
   */
  void Initialize() override;

  /**
   * @copydoc Dali::AutofillItem::GetId()
   */
  const std::string& GetId() const override;

  /**
   * @copydoc Dali::AutofillItem::GetLabel()
   */
  const std::string& GetLabel() const override;

  /**
   * @copydoc Dali::AutofillItem::GetHint()
   */
  Dali::AutofillItem::Hint GetHint() const override;

  /**
   * @copydoc Dali::AutofillItem::IsSensitiveData()
   */
  bool IsSensitiveData() const override;

  /**
   * @copydoc Dali::AutofillItem::SetSaveValue()
   */
  void SetSaveValue( const std::string& value ) override;

  /**
   * @copydoc Dali::AutofillItem::GetSaveValue()
   */
  const std::string& GetSaveValue() const override;

#ifdef CAPI_AUTOFILL_SUPPORT
  /**
   * @brief Gets Autofill framework item handle
   * @return Autofill framework item handle, which type is 'autofill_item_h'
   */
  autofill_item_h GetAutofillItemHandle();

  /**
   * @brief Gets Autofill framework save item handle
   * @return Autofill framework save item handle, which type is 'autofill_save_item_h'
   */
  autofill_save_item_h GetAutofillSaveItemHandle();
#endif // CAPI_AUTOFILL_SUPPORT

  /**
   * @brief Adds the presentation text to fill out in the list.
   *
   * @param[in] presentationText The presentation text to fill out
   */
  void AddPresentationList( const std::string& presentationText ) override;

  /**
   * @brief Adds the value to fill out in the list.
   *
   * @param[in] fillValue The value to fill out
   */
  void AddFillValueList( const std::string& fillValue ) override;

  /**
   * @copydoc Dali::AutofillItem::GetPresentationText()
   */
  const std::string& GetPresentationText( int index ) const override;

  /**
   * @copydoc Dali::AutofillItem::GetFillValue()
   */
  const std::string& GetFillValue( int index ) const override;

  /**
   * @copydoc Dali::AutofillItem::ClearPresentationTextList()
   */
  void ClearPresentationTextList() override;

  /**
   * @copydoc Dali::AutofillItem::ClearFillValueList()
   */
  void ClearFillValueList() override;

  /**
   * @copydoc Dali::AutofillItem::GetFillValueCount()
   */
  unsigned int GetFillValueCount() override;

private:
  /**
   * Constructor.
   */
  explicit AutofillItemEcorewWl( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool sensitiveData );

protected:
  /**
   * Destructor.
   */
  ~AutofillItemEcorewWl();

private:
  // Undefined copy constructor
  explicit AutofillItemEcorewWl( const AutofillItemEcorewWl& autofillItem ) = delete;

  // Undefined assignment operator
  AutofillItemEcorewWl& operator=( AutofillItemEcorewWl& autofillItem ) = delete;

private:
#ifdef CAPI_AUTOFILL_SUPPORT
  autofill_item_h mAutofillItemHandle;          ///< The Autofill Framework item handle
  autofill_save_item_h mAutofillSaveItemHandle; ///< The Autofill Framework save item handle for save
#endif // CAPI_AUTOFILL_SUPPORT

// Data
private:
  std::string mId;                      ///< The AutofillItem ID
  std::string mLabel;                   ///< The AutofillItem Label
  Dali::AutofillItem::Hint mHint;       ///< The AutofillItem Hint (id (username), name, password, phone, credit card number, organization, so on)
  bool mSensitiveData;                  ///< Whether the data is sensitive or not

  std::string mValue;

  std::vector<std::string> mPresentationTextList; ///< The list for the presentation text to fill out
  std::vector<std::string> mValueList;            ///< The list for the value to fill out

};



} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_AUTOFILL_ITEM_IMPL_ECORE_WL_H
