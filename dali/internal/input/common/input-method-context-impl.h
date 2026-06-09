#ifndef DALI_INTERNAL_INPUT_COMMON_INPUT_METHOD_CONTEXT_IMPL_H
#define DALI_INTERNAL_INPUT_COMMON_INPUT_METHOD_CONTEXT_IMPL_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/common/vector-wrapper.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/object/base-object.h>
#include <functional>
#include <memory>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/input-method-context-integ.h>
#include <dali/integration-api/adaptor-framework/input-method-options.h>
#include <dali/public-api/adaptor-framework/input-method-context.h>

namespace Dali
{
class RenderSurface;

namespace Internal
{
namespace Adaptor
{
class InputMethodContext;
typedef IntrusivePtr<InputMethodContext> InputMethodContextPtr;
using ImfContext = void;

class InputMethodContext : public Dali::BaseObject
{
public:
  using ActivatedSignalType                 = Dali::InputMethodContext::ActivatedSignalType;
  using KeyboardEventSignalType             = Dali::Integration::InputMethodContext::KeyboardEventSignalType;
  using StatusChangedSignalType             = Dali::InputMethodContext::StatusChangedSignalType;
  using KeyboardTypeChangedSignalType       = Dali::InputMethodContext::KeyboardTypeChangedSignalType;
  using KeyboardResizedSignalType           = Dali::InputMethodContext::KeyboardResizedSignalType;
  using LanguageChangedSignalType           = Dali::InputMethodContext::LanguageChangedSignalType;
  using ContentReceivedSignalType           = Dali::Integration::InputMethodContext::ContentReceivedSignalType;
  using PrivateCommandReceivedSignalType    = Dali::InputMethodContext::PrivateCommandReceivedSignalType;
  using LegacyStatusChangedSignalType       = Dali::Integration::InputMethodContext::LegacyStatusChangedSignalType;
  using LegacyKeyboardTypeChangedSignalType = Dali::Integration::InputMethodContext::LegacyKeyboardTypeChangedSignalType;
  using LegacyKeyboardResizedSignalType     = Dali::Integration::InputMethodContext::LegacyKeyboardResizedSignalType;
  using LegacyLanguageChangedSignalType     = Dali::Integration::InputMethodContext::LegacyLanguageChangedSignalType;
  using LegacyContentReceivedSignalType     = Dali::Integration::InputMethodContext::LegacyContentReceivedSignalType;

public:
  /**
   * Create a new input method context instance.
   */
  static InputMethodContextPtr New(Dali::Actor actor);

  /**
   * Initialize the object.
   */
  virtual void Initialize()
  {
  }

  /**
   * Connect Callbacks required for InputMethodContext.
   * If you don't connect InputMethodContext callbacks, you can't get the key events.
   * The events are PreeditChanged, Commit, DeleteSurrounding and PrivateCommand.
   */
  virtual void ConnectCallbacks()
  {
  }

  /**
   * Disconnect Callbacks attached to input method context.
   */
  virtual void DisconnectCallbacks()
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::Finalize()
   */
  virtual void Finalize()
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::Activate()
   */
  virtual void Activate()
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::Deactivate()
   */
  virtual void Deactivate()
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::Reset()
   */
  virtual void Reset()
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::GetContext()
   */
  virtual ImfContext* GetContext()
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::InputMethodContext::RestoreAfterFocusLost()
   */
  virtual bool RestoreAfterFocusLost() const
  {
    return false;
  }

  /**
   * @copydoc Dali::InputMethodContext::SetRestoreAfterFocusLost()
   */
  virtual bool SetRestoreAfterFocusLost(bool toggle)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::PreEditChanged()
   */
  virtual void PreEditChanged(void* data, ImfContext* imfContext, void* eventInfo)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::CommitReceived()
   */
  virtual void CommitReceived(void* data, ImfContext* imfContext, void* eventInfo)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::RetrieveSurrounding()
   */
  /*Eina_Bool*/
  virtual bool RetrieveSurrounding(void* data, ImfContext* imfContext, char** text, int* cursorPosition)
  {
    return false;
  }

  /**
   * @copydoc Dali::InputMethodContext::DeleteSurrounding()
   */
  virtual void DeleteSurrounding(void* data, ImfContext* imfContext, void* eventInfo)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::SendPrivateCommand()
   */
  virtual void SendPrivateCommand(void* data, ImfContext* imfContext, void* eventInfo)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::SendCommitContent()
   */
  virtual void SendCommitContent(void* data, ImfContext* imfContext, void* eventInfo)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::SendSelectionSet()
   */
  virtual void SendSelectionSet(void* data, ImfContext* imfContext, void* eventInfo)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::TransactionStartReceived()
   */
  virtual void TransactionStartReceived(void* data, ImfContext* imfContext, void* eventInfo)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::TransactionEndReceived()
   */
  virtual void TransactionEndReceived(void* data, ImfContext* imfContext, void* eventInfo)
  {
  }

  // Cursor related
  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  virtual void NotifyCursorPosition()
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::SetCursorPosition()
   */
  virtual void SetCursorPosition(unsigned int cursorPosition)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::GetCursorPosition()
   */
  virtual unsigned int GetCursorPosition() const
  {
    return 0;
  }

  /**
   * @copydoc Dali::InputMethodContext::SetSurroundingText()
   */
  virtual void SetSurroundingText(const Dali::String& text)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::GetSurroundingText()
   */
  virtual Dali::String GetSurroundingText() const;

  /**
   * @copydoc Dali::InputMethodContext::NotifyTextInputMultiLine()
   */
  virtual void NotifyTextInputMultiLine(bool multiLine)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::GetTextDirection()
   */
  virtual Dali::Integration::InputMethodContext::TextDirection GetTextDirection()
  {
    return Dali::Integration::InputMethodContext::LEFT_TO_RIGHT;
  }

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelArea()
   */
  virtual Dali::BoundsInteger GetInputPanelArea()
  {
    return Dali::BoundsInteger();
  }

  /**
   * @copydoc Dali::InputMethodContext::ApplyOptions()
   */
  virtual void ApplyOptions(const Dali::Integration::InputMethodOptions& options)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelUserData()
   */
  virtual bool SetInputPanelUserData(const Dali::String& data)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelUserData()
   */
  virtual Dali::String GetInputPanelUserData() const
  {
    return Dali::String();
  }

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelState()
   */
  virtual Dali::InputMethodContext::State GetInputPanelState()
  {
    return Dali::InputMethodContext::State();
  }

  /**
   * @copydoc Dali::InputMethodContext::SetReturnKeyState()
   */
  virtual bool SetReturnKeyState(bool visible)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::IsReturnKeyEnabled()
   */
  virtual bool IsReturnKeyEnabled() const
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::AutoEnableInputPanel()
   */
  virtual bool AutoEnableInputPanel(bool enabled)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::ShowInputPanel()
   */
  virtual bool ShowInputPanel()
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::HideInputPanel()
   */
  virtual bool HideInputPanel()
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::GetKeyboardType()
   */
  virtual Dali::InputMethodContext::KeyboardType GetKeyboardType()
  {
    return Dali::InputMethodContext::KeyboardType();
  }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelLanguageLocale()
   */
  virtual bool SetInputPanelLanguageLocale(const Dali::String& locale)
  {
    return false;
  }

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelLanguageLocale()
   */
  virtual Dali::String GetInputPanelLanguageLocale() const
  {
    return Dali::String();
  }

  /**
   * @copydoc Dali::Integration::InputMethodContext::SetContentMimeTypes()
   */
  virtual void SetContentMimeTypes(const Dali::String& mimeTypes)
  {
  }

  /**
   * @copydoc Dali::InputMethodContext::FilterEventKey()
   */
  virtual bool FilterEventKey(const Dali::KeyEvent& keyEvent)
  {
    return false;
  }

  /**
   * @copydoc Dali::InputMethodContext::AllowTextPrediction()
   */
  virtual bool AllowTextPrediction(bool prediction)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::IsTextPredictionAllowed()
   */
  virtual bool IsTextPredictionAllowed() const
  {
    return false;
  }

  /**
   * @copydoc Dali::InputMethodContext::SetFullScreenMode()
   */
  virtual bool SetFullScreenMode(bool fullScreen)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::IsFullScreenMode()
   */
  virtual bool IsFullScreenMode() const
  {
    return false;
  }

  /**
   * @copydoc Dali::Integration::InputMethodContext::SetInputPanelLanguage()
   */
  virtual bool SetInputPanelLanguage(Dali::Integration::InputMethodContext::InputPanelLanguage language)
  {
    return true;
  }

  /**
   * @copydoc Dali::Integration::InputMethodContext::GetInputPanelLanguage()
   */
  virtual Dali::Integration::InputMethodContext::InputPanelLanguage GetInputPanelLanguage() const
  {
    return Dali::Integration::InputMethodContext::InputPanelLanguage::AUTOMATIC;
  }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelPosition()
   */
  virtual bool SetInputPanelPosition(unsigned int x, unsigned int y)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelPositionAlign()
   */
  virtual bool SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align)
  {
    return false;
  }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelLayout()
   */
  virtual bool SetInputPanelLayout(Dali::InputMethod::PanelLayout layout)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelLayout()
   */
  virtual Dali::InputMethod::PanelLayout GetInputPanelLayout() const
  {
    return Dali::InputMethod::PanelLayout::NORMAL;
  }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelReturnKeyType()
   */
  virtual bool SetInputPanelReturnKeyType(Dali::InputMethod::ReturnKeyType action)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelReturnKeyType()
   */
  virtual Dali::InputMethod::ReturnKeyType GetInputPanelReturnKeyType() const
  {
    return Dali::InputMethod::ReturnKeyType::DEFAULT;
  }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelAutoCapitalType()
   */
  virtual bool SetInputPanelAutoCapitalType(Dali::InputMethod::AutoCapitalType autoCapital)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelAutoCapitalType()
   */
  virtual Dali::InputMethod::AutoCapitalType GetInputPanelAutoCapitalType() const
  {
    return Dali::InputMethod::AutoCapitalType::SENTENCE;
  }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelLayoutVariation()
   */
  virtual bool SetInputPanelLayoutVariation(Dali::InputMethod::PanelLayoutVariation variation)
  {
    return true;
  }

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelLayoutVariation()
   */
  virtual Dali::InputMethod::PanelLayoutVariation GetInputPanelLayoutVariation() const
  {
    return Dali::InputMethod::PanelLayoutVariation::NORMAL_NORMAL;
  }

  /**
   * @copydoc Dali::InputMethodContext::GetPreeditStyle()
   */
  virtual void GetPreeditStyle(Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) const
  {
  }

public: // Signals
  /**
   * @copydoc Dali::InputMethodContext::ActivatedSignal()
   */
  ActivatedSignalType& ActivatedSignal();

  /**
   * @copydoc Dali::Integration::InputMethodContext::EventReceivedSignal()
   */
  KeyboardEventSignalType& EventReceivedSignal();

  /**
   * @copydoc Dali::Integration::InputMethodContext::KeyboardEventReceivedSignal()
   */
  KeyboardEventSignalType& KeyboardEventReceivedSignal();

  /**
   * @copydoc Dali::InputMethodContext::StatusChangedSignal()
   */
  StatusChangedSignalType& StatusChangedSignal();

  /**
   * @copydoc Dali::InputMethodContext::ResizedSignal()
   */
  KeyboardResizedSignalType& ResizedSignal();

  /**
   * @copydoc Dali::InputMethodContext::LanguageChangedSignal()
   */
  LanguageChangedSignalType& LanguageChangedSignal();

  /**
   * @copydoc Dali::InputMethodContext::KeyboardTypeChangedSignal()
   */
  KeyboardTypeChangedSignalType& KeyboardTypeChangedSignal();

  /**
   * @copydoc Dali::Integration::InputMethodContext::ContentReceivedSignal()
   */
  ContentReceivedSignalType& ContentReceivedSignal();

  /**
   * @copydoc Dali::InputMethodContext::PrivateCommandReceivedSignal()
   */
  PrivateCommandReceivedSignalType& PrivateCommandReceivedSignal();

  /**
   * @copydoc Dali::Integration::InputMethodContext::LegacyStatusChangedSignal()
   */
  LegacyStatusChangedSignalType& LegacyStatusChangedSignal();

  /**
   * @copydoc Dali::Integration::InputMethodContext::LegacyResizedSignal()
   */
  LegacyKeyboardResizedSignalType& LegacyResizedSignal();

  /**
   * @copydoc Dali::Integration::InputMethodContext::LegacyLanguageChangedSignal()
   */
  LegacyLanguageChangedSignalType& LegacyLanguageChangedSignal();

  /**
   * @copydoc Dali::Integration::InputMethodContext::LegacyKeyboardTypeChangedSignal()
   */
  LegacyKeyboardTypeChangedSignalType& LegacyKeyboardTypeChangedSignal();

  /**
   * @copydoc Dali::Integration::InputMethodContext::LegacyContentReceivedSignal()
   */
  LegacyContentReceivedSignalType& LegacyContentReceivedSignal();

public:
  /**
   * Constructor
   */
  InputMethodContext();

  /**
   * Destructor
   */
  ~InputMethodContext() override = default;

private:
  InputMethodContext(const InputMethodContext&)      = delete;
  InputMethodContext& operator=(InputMethodContext&) = delete;

protected:
  /**
   * @brief Struct for providing Operation enumeration
   */
  struct Operation
  {
    enum Type
    {
      ALLOW_TEXT_PREDICTION = 0,
      AUTO_ENABLE_INPUT_PANEL,
      NOTIFY_TEXT_INPUT_MULTILINE,
      SET_CONTENT_MIME_TYPES,
      SET_INPUT_PANEL_USER_DATA,
      SET_INPUT_PANEL_LANGUAGE_LOCALE,
      SET_INPUT_PANEL_LANGUAGE,
      SET_INPUT_PANEL_POSITION,
      SET_INPUT_PANEL_POSITION_ALIGN,
      SET_RETURN_KEY_STATE,
      SET_INPUT_PANEL_LAYOUT,
      SET_INPUT_PANEL_RETURN_KEY,
      SET_INPUT_PANEL_AUTO_CAPITAL,
      SET_INPUT_PANEL_LAYOUT_VARIATION,
      FULLSCREEN_MODE,
      MAX_COUNT
    };
  };

  using OperationList = std::vector<std::function<void()> >;

  /**
   * @brief Apply backup operations to the InputMethodContext
   */
  void ApplyBackupOperations();

public:
  /**
   * @brief Emits both public and legacy status signals.
   */
  void EmitStatusChangedSignal(Dali::InputMethodContext::State state);

  /**
   * @brief Emits both public and legacy keyboard resized signals.
   */
  void EmitKeyboardResizedSignal(int value);

  /**
   * @brief Emits both public and legacy language changed signals.
   */
  void EmitLanguageChangedSignal(int value);

  /**
   * @brief Emits both public and legacy keyboard type changed signals.
   */
  void EmitKeyboardTypeChangedSignal(Dali::InputMethodContext::KeyboardType keyboardType);

  /**
   * @brief Emits both public and legacy content received signals.
   */
  void EmitContentReceivedSignal(const Dali::String& content, const Dali::String& description, const Dali::String& mimeTypes);

protected:
  ActivatedSignalType              mActivatedSignal;
  KeyboardEventSignalType          mEventSignal;
  KeyboardEventSignalType          mKeyboardEventSignal;
  StatusChangedSignalType          mKeyboardStatusSignal;
  KeyboardResizedSignalType        mKeyboardResizeSignal;
  LanguageChangedSignalType        mKeyboardLanguageChangedSignal;
  KeyboardTypeChangedSignalType    mKeyboardTypeChangedSignal;
  ContentReceivedSignalType        mContentReceivedSignal;
  PrivateCommandReceivedSignalType mPrivateCommandReceivedSignal;

  // Legacy signals used by bindings for NUI compatibility.
  LegacyStatusChangedSignalType       mLegacyKeyboardStatusSignal;
  LegacyKeyboardResizedSignalType     mLegacyKeyboardResizeSignal;
  LegacyLanguageChangedSignalType     mLegacyKeyboardLanguageChangedSignal;
  LegacyKeyboardTypeChangedSignalType mLegacyKeyboardTypeChangedSignal;
  LegacyContentReceivedSignalType     mLegacyContentReceivedSignal;

  OperationList mBackupOperations;

public:
  inline static Internal::Adaptor::InputMethodContext& GetImplementation(Dali::InputMethodContext& inputMethodContext)
  {
    DALI_ASSERT_ALWAYS(inputMethodContext && "InputMethodContext handle is empty");

    BaseObject& handle = inputMethodContext.GetBaseObject();

    return static_cast<Internal::Adaptor::InputMethodContext&>(handle);
  }

  inline static const Internal::Adaptor::InputMethodContext& GetImplementation(const Dali::InputMethodContext& inputMethodContext)
  {
    DALI_ASSERT_ALWAYS(inputMethodContext && "InputMethodContext handle is empty");

    const BaseObject& handle = inputMethodContext.GetBaseObject();

    return static_cast<const Internal::Adaptor::InputMethodContext&>(handle);
  }
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_INPUT_COMMON_INPUT_METHOD_CONTEXT_IMPL_H
