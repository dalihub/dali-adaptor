#ifndef DALI_INTERNAL_INPUT_METHOD_CONTEXT_IMPL_BRIDGE_H
#define DALI_INTERNAL_INPUT_METHOD_CONTEXT_IMPL_BRIDGE_H

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
#include <cstdint>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/object/weak-handle.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/ime-bridge.h>
#include <dali/internal/input/common/input-method-context-impl.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Input method context that delegates to a registered ImeBridge.
 *
 * Used by the Windows bridge backend, where the IME runs in this process
 * instead of behind the platform's input method service. This class is a forwarding
 * shell: it owns no IME logic, caches only what the application may read
 * back synchronously, and behaves like the generic backend when no bridge is
 * registered.
 */
class InputMethodContextBridge : public Dali::Internal::Adaptor::InputMethodContext,
                                 public Dali::Integration::ImeBridgeHost
{
public:
  /**
   * @brief Creates a new InputMethodContext handle.
   * @param[in] actor The actor that uses the new InputMethodContext instance
   * @return InputMethodContext pointer
   */
  static InputMethodContextPtr New(Dali::Actor actor);

  /**
   * @brief Constructor.
   * @param[in] actor The actor that uses this instance
   */
  explicit InputMethodContextBridge(Dali::Actor actor);

  /**
   * @brief Destructor.
   */
  ~InputMethodContextBridge() override;

  /**
   * @brief Joins the host registry and attaches to the registered bridge.
   *
   * Deliberately *not* called from the constructor. The bridge may call back
   * synchronously (ImeBridgeHost::RetrieveSurrounding() is documented
   * synchronous, and a bridge typically assembles its context on
   * ImeBridge::ShowInputPanel()), and every IME-to-application path here
   * builds a `Dali::InputMethodContext handle(this)`. From a constructor the
   * RefObject count is still zero, so that handle would take it 0 -> 1 and
   * its destructor 1 -> 0, deleting the object inside its own constructor.
   * New() therefore calls this only once an InputMethodContextPtr holds the
   * first reference.
   *
   * AttachHost() and OnBridgeAttached() are issued as a pair, which is the
   * same invariant RegisterImeBridge() maintains.
   */
  void RegisterWithBridge();

  // InputMethodContext lifecycle
  void Initialize() override;
  void Finalize() override;
  void ConnectCallbacks() override;
  void DisconnectCallbacks() override;

  // InputMethodContext: application to IME
  void                                      Activate() override;
  void                                      Deactivate() override;
  void                                      Reset() override;
  bool                                      ShowInputPanel() override;
  bool                                      HideInputPanel() override;
  bool                                      AutoEnableInputPanel(bool enabled) override;
  bool                                      FilterEventKey(const Dali::KeyEvent& keyEvent) override;
  void                                      SetCursorPosition(unsigned int cursorPosition) override;
  unsigned int                              GetCursorPosition() const override;
  void                                      SetSurroundingText(const Dali::String& text) override;
  Dali::String                              GetSurroundingText() const override;
  void                                      NotifyCursorPosition() override;
  bool                                      SetInputPanelLayout(Dali::InputMethod::PanelLayout layout) override;
  Dali::InputMethod::PanelLayout            GetInputPanelLayout() const override;
  bool                                      SetInputPanelLayoutVariation(Dali::InputMethod::PanelLayoutVariation variation) override;
  Dali::InputMethod::PanelLayoutVariation   GetInputPanelLayoutVariation() const override;
  bool                                      SetInputPanelReturnKeyType(Dali::InputMethod::ReturnKeyType action) override;
  Dali::InputMethod::ReturnKeyType          GetInputPanelReturnKeyType() const override;
  bool                                      SetReturnKeyState(bool visible) override;
  bool                                      IsReturnKeyEnabled() const override;
  bool                                      SetInputPanelAutoCapitalType(Dali::InputMethod::AutoCapitalType autoCapital) override;
  Dali::InputMethod::AutoCapitalType        GetInputPanelAutoCapitalType() const override;
  bool                                      AllowTextPrediction(bool prediction) override;
  bool                                      IsTextPredictionAllowed() const override;
  bool                                      SetInputPanelUserData(const Dali::String& data) override;
  Dali::String                              GetInputPanelUserData() const override;
  bool                                      SetInputPanelLanguage(Dali::Integration::InputMethodContext::InputPanelLanguage language) override;
  Dali::Integration::InputMethodContext::InputPanelLanguage GetInputPanelLanguage() const override;
  void                                      SetContentMimeTypes(const Dali::String& mimeTypes) override;
  bool                                      SetInputPanelPosition(unsigned int x, unsigned int y) override;
  bool                                      SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align) override;
  void                                      NotifyTextInputMultiLine(bool multiLine) override;
  void                                      ApplyOptions(const Dali::Integration::InputMethodOptions& options) override;

  // InputMethodContext: cached reads (never reach the bridge)
  Dali::BoundsInteger                 GetInputPanelArea() override;
  Dali::InputMethodContext::State     GetInputPanelState() override;
  Dali::InputMethodContext::KeyboardType GetKeyboardType() override;
  void                                GetPreeditStyle(Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) const override;

  // ImeBridgeHost overrides hide same-named native callbacks with different signatures.
  // Keep both overloads visible to avoid -Woverloaded-virtual errors.
  // TODO: Find a better way to resolve this interface overlap.
  using InputMethodContext::DeleteSurrounding;
  using InputMethodContext::SendPrivateCommand;
  using InputMethodContext::RetrieveSurrounding;

  // ImeBridgeHost: IME to application
  void Commit(const Dali::String& text) override;
  void PreEdit(const Dali::String&                                                        text,
               int32_t                                                                     cursorPosition,
               const Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) override;
  void PreEditStart() override;
  void PreEditEnd() override;
  void DeleteSurrounding(int32_t cursorOffset, int32_t numberOfChars) override;
  void SetSelection(int32_t startIndex, int32_t endIndex) override;
  void SendPrivateCommand(const Dali::String& command) override;
  void CommitContent(const Dali::String& contentUri, const Dali::String& description, const Dali::String& mimeTypes) override;
  void RecaptureString(int32_t                                                                     cursorOffset,
                       int32_t                                                                     numberOfChars,
                       const Dali::String&                                                         preedit,
                       const Dali::String&                                                         commit,
                       const Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) override;
  bool RetrieveSurrounding(Dali::String& text, int32_t& cursorPosition) override;
  void FeedKeyEvent(const Dali::KeyEvent& keyEvent) override;
  void InputPanelGeometryChanged(const Dali::BoundsInteger& area) override;
  void InputPanelStateChanged(Dali::InputMethodContext::State state) override;
  void InputPanelLanguageChanged(Dali::Integration::InputMethodContext::InputPanelLanguage language) override;
  void KeyboardTypeChanged(Dali::InputMethodContext::KeyboardType keyboardType) override;
  void OnBridgeAttached() override;
  void OnBridgeDetached() override;

private:
  // Undefined copy constructor
  InputMethodContextBridge(const InputMethodContextBridge&) = delete;

  // Undefined assignment operator
  InputMethodContextBridge& operator=(const InputMethodContextBridge&) = delete;

  /**
   * @brief Returns the registered bridge, or nullptr when none is registered.
   */
  Dali::Integration::ImeBridge* Bridge() const;

  /**
   * @brief Replays a ShowInputPanel() that arrived while no bridge was registered.
   */
  void FlushPendingShow();

  /**
   * @brief Replays the cached application-side configuration to a newly attached bridge.
   *
   * Runs on every attach (RegisterWithBridge() and late RegisterImeBridge()
   * alike), before FlushPendingShow().
   */
  void FlushPendingState();

private:
  Dali::Integration::InputMethodOptions                                mOptions;
  Dali::Integration::InputMethodContext::PreEditAttributeDataContainer mPreeditAttrs;

  Dali::WeakHandle<Dali::Actor> mActor; ///< Weak: a strong Actor handle would cycle back to this context.

  Dali::String mSurroundingText;
  Dali::String mUserData;
  Dali::String mContentMimeTypes;

  Dali::BoundsInteger                    mInputPanelArea{0, 0, 0, 0};
  Dali::InputMethodContext::State        mInputPanelState{Dali::InputMethodContext::State::HIDE};
  Dali::InputMethodContext::KeyboardType mKeyboardType{Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD};

  Dali::InputMethod::PanelLayout          mPanelLayout{Dali::InputMethod::PanelLayout::NORMAL};
  Dali::InputMethod::PanelLayoutVariation mPanelLayoutVariation{Dali::InputMethod::PanelLayoutVariation::NORMAL_NORMAL};
  Dali::InputMethod::ReturnKeyType        mReturnKeyType{Dali::InputMethod::ReturnKeyType::DEFAULT};
  Dali::InputMethod::AutoCapitalType      mAutoCapitalType{Dali::InputMethod::AutoCapitalType::SENTENCE};

  Dali::Integration::InputMethodContext::InputPanelLanguage mInputPanelLanguage{
    Dali::Integration::InputMethodContext::InputPanelLanguage::AUTOMATIC};

  int32_t mCursorPosition{0};
  int32_t mPanelPositionX{0};
  int32_t mPanelPositionY{0};

  bool mReturnKeyEnabled : 1;
  bool mTextPredictionAllowed : 1;
  bool mMultiLine : 1;
  bool mPanelPositionSet : 1; ///< SetInputPanelPosition() was called at least once
  bool mPanelShowPending : 1; ///< ShowInputPanel() arrived with no bridge registered
  bool mInputPanelAutoShow : 1; ///< Show on Activate() and hide on Deactivate(); see AutoEnableInputPanel()
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_INPUT_METHOD_CONTEXT_IMPL_BRIDGE_H
