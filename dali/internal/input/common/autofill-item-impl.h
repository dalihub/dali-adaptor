#ifndef DALI_INTERNAL_AUTOFILL_ITEM_IMPL_H
#define DALI_INTERNAL_AUTOFILL_ITEM_IMPL_H

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
#include <dali/devel-api/adaptor-framework/autofill-item.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * @brief This class is used to pass on data from the AutofillItem of control.
 */
class AutofillItem : public Dali::BaseObject
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
  virtual void Initialize() = 0;

  /**
   * @copydoc Dali::AutofillItem::GetId()
   */
  virtual const std::string& GetId() const = 0;

  /**
   * @copydoc Dali::AutofillItem::GetLabel()
   */
  virtual const std::string& GetLabel() const = 0;

  /**
   * @copydoc Dali::AutofillItem::GetHint()
   */
  virtual Dali::AutofillItem::Hint GetHint() const = 0;

  /**
   * @copydoc Dali::AutofillItem::IsSensitiveData()
   */
  virtual bool IsSensitiveData() const = 0;

  /**
   * @copydoc Dali::AutofillItem::SetSaveValue()
   */
  virtual void SetSaveValue( const std::string& value ) = 0;

  /**
   * @copydoc Dali::AutofillItem::GetSaveValue()
   */
  virtual const std::string& GetSaveValue() const = 0;

  /**
   * @brief Adds the presentation text to fill out in the list.
   *
   * @param[in] presentationText The presentation text to fill out
   */
  virtual void AddPresentationList( const std::string& presentationText ) = 0;

  /**
   * @brief Adds the value to fill out in the list.
   *
   * @param[in] fillValue The value to fill out
   */
  virtual void AddFillValueList( const std::string& fillValue ) = 0;

  /**
   * @copydoc Dali::AutofillItem::GetPresentationText()
   */
  virtual const std::string& GetPresentationText( int index ) const = 0;

  /**
   * @copydoc Dali::AutofillItem::GetFillValue()
   */
  virtual const std::string& GetFillValue( int index ) const = 0;

  /**
   * @copydoc Dali::AutofillItem::ClearPresentationTextList()
   */
  virtual void ClearPresentationTextList() = 0;

  /**
   * @copydoc Dali::AutofillItem::ClearFillValueList()
   */
  virtual void ClearFillValueList() = 0;

  /**
   * @copydoc Dali::AutofillItem::GetFillValueCount()
   */
  virtual unsigned int GetFillValueCount() = 0;

public:
  /**
   * Constructor.
   */
  AutofillItem() = default;

protected:
  /**
   * Destructor.
   */
  ~AutofillItem() = default;

private:
  // Undefined copy constructor
  AutofillItem( const AutofillItem& autofillItem ) = delete;

  // Undefined assignment operator
  AutofillItem& operator=( AutofillItem& autofillItem ) = delete;

};

inline static Internal::Adaptor::AutofillItem& GetImplementation(Dali::AutofillItem& autofillItem)
{
  DALI_ASSERT_ALWAYS( autofillItem && "AutofillItem handle is empty" );

  BaseObject& handle = autofillItem.GetBaseObject();

  return static_cast<Internal::Adaptor::AutofillItem&>(handle);
}

inline static const Internal::Adaptor::AutofillItem& GetImplementation(const Dali::AutofillItem& autofillItem)
{
  DALI_ASSERT_ALWAYS( autofillItem && "AutofillItem handle is empty" );

  const BaseObject& handle = autofillItem.GetBaseObject();

  return static_cast<const Internal::Adaptor::AutofillItem&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_AUTOFILL_ITEM_IMPL_H
