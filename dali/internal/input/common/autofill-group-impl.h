#ifndef DALI_INTERNAL_AUTOFILL_GROUP_IMPL_H
#define DALI_INTERNAL_AUTOFILL_GROUP_IMPL_H

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
#include <dali/devel-api/adaptor-framework/autofill-group.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * @brief This class is used to group AutofillItems.
 */
class AutofillGroup : public Dali::BaseObject
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
  virtual void Initialize() = 0;

  /**
   * @copydoc Dali::AutofillGroup::GetId()
   */
  virtual const std::string& GetId() const = 0;

  /**
   * @copydoc Dali::AutofillGroup::AddAutofillItem()
   */
  virtual void AddAutofillItem( Dali::AutofillItem item ) = 0;

  /**
   * @copydoc Dali::AutofillGroup::GetAutofillItem()
   */
  virtual Dali::AutofillItem GetAutofillItem( const std::string& id ) = 0;

  /**
   * @brief Clears all lists of AutofillItem added in AutofillGroup.
   */
  virtual void ClearAutofillItemList() = 0;

  /**
   * @copydoc Dali::AutofillGroup::SaveAutofillData()
   */
  virtual void SaveAutofillData() = 0;

  /**
   * @copydoc Dali::AutofillGroup::RequestAuthentication()
   */
  virtual void RequestAuthentication() = 0;

  /**
   * @copydoc Dali::AutofillGroup::SendFillRequest()
   */
  virtual void SendFillRequest() = 0;

public:
  /**
   * Constructor.
   */
  AutofillGroup() = default;

protected:
  /**
   * Destructor.
   */
  ~AutofillGroup() = default;

private:
  // Undefined copy constructor
  AutofillGroup( const AutofillGroup& autofillGroup ) = delete;

  // Undefined assignment operator
  AutofillGroup& operator=( AutofillGroup& autofillGroup ) = delete;

};

inline static Internal::Adaptor::AutofillGroup& GetImplementation(Dali::AutofillGroup& autofillGroup)
{
  DALI_ASSERT_ALWAYS( autofillGroup && "AutofillGroup handle is empty" );

  BaseObject& handle = autofillGroup.GetBaseObject();

  return static_cast<Internal::Adaptor::AutofillGroup&>(handle);
}

inline static const Internal::Adaptor::AutofillGroup& GetImplementation(const Dali::AutofillGroup& autofillGroup)
{
  DALI_ASSERT_ALWAYS( autofillGroup && "AutofillGroup handle is empty" );

  const BaseObject& handle = autofillGroup.GetBaseObject();

  return static_cast<const Internal::Adaptor::AutofillGroup&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_AUTOFILL_GROUP_IMPL_H
