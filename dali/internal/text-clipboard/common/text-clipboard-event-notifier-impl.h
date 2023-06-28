#ifndef DALI_INTERNAL_TEXT_CLIPBOARD_EVENT_NOTIFIER_H
#define DALI_INTERNAL_TEXT_CLIPBOARD_EVENT_NOTIFIER_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/object/base-object.h>
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/text-clipboard-event-notifier.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * This class listens to TextClipboard events.
 */
class TextClipboardEventNotifier : public Dali::BaseObject
{
public:
  typedef Dali::TextClipboardEventNotifier::TextClipboardEventSignalType TextClipboardEventSignalType;

  // Creation

  /**
   * Create a TextClipboardEventNotifier.
   * @return A newly allocated text-clipboard-event-notifier.
   */
  static Dali::TextClipboardEventNotifier New();

  /**
   * @copydoc Dali::TextClipboardEventNotifier::Get()
   */
  static Dali::TextClipboardEventNotifier Get();

  // Public API

  /**
   * @copydoc Dali::TextClipboardEventNotifier::GetContent() const
   */
  const std::string& GetContent() const;

  /**
   * Sets the selected content.
   * @param[in] content  A string that represents the content that has been selected.
   */
  void SetContent(const std::string& content);

  /**
   * Clears the stored content.
   */
  void ClearContent();

  /**
   * Called when content is selected in the clipboard.
   */
  void EmitContentSelectedSignal();

public: // Signals
  /**
   * @copydoc Dali::TextClipboardEventNotifier::ContentSelectedSignal
   */
  TextClipboardEventSignalType& ContentSelectedSignal()
  {
    return mContentSelectedSignal;
  }

private:
  // Construction & Destruction

  /**
   * Constructor.
   */
  TextClipboardEventNotifier();

  /**
   * Destructor.
   */
  ~TextClipboardEventNotifier() override;

  // Undefined
  TextClipboardEventNotifier(const TextClipboardEventNotifier&);
  TextClipboardEventNotifier& operator=(TextClipboardEventNotifier&);

private:
  std::string mContent; ///< The current selected content.

  TextClipboardEventSignalType mContentSelectedSignal;

public:
  // Helpers for public-api forwarding methods

  inline static Internal::Adaptor::TextClipboardEventNotifier& GetImplementation(Dali::TextClipboardEventNotifier& detector)
  {
    DALI_ASSERT_ALWAYS(detector && "TextClipboardEventNotifier handle is empty");

    BaseObject& handle = detector.GetBaseObject();

    return static_cast<Internal::Adaptor::TextClipboardEventNotifier&>(handle);
  }

  inline static const Internal::Adaptor::TextClipboardEventNotifier& GetImplementation(const Dali::TextClipboardEventNotifier& detector)
  {
    DALI_ASSERT_ALWAYS(detector && "TextClipboardEventNotifier handle is empty");

    const BaseObject& handle = detector.GetBaseObject();

    return static_cast<const Internal::Adaptor::TextClipboardEventNotifier&>(handle);
  }
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_CLIPBOARD_EVENT_NOTIFIER_H
