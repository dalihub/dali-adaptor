#ifndef DALI_INTEGRATION_API_ADAPTOR_FRAMEWORK_IME_BRIDGE_H
#define DALI_INTEGRATION_API_ADAPTOR_FRAMEWORK_IME_BRIDGE_H

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

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/input-method-context-integ.h>
#include <dali/public-api/adaptor-framework/input-method-context.h>
#include <dali/public-api/adaptor-framework/input-method.h>
#include <dali/public-api/common/dali-string.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/math/rect.h>

namespace DALI_NAMESPACE
{
namespace Integration
{

/**
 * @brief Interface an input method context uses to deliver IME output to the application.
 *
 * Implemented by the Adaptor's input method context; the registered ImeBridge
 * calls it. Every call arrives on the main/event thread.
 *
 * @note Not ABI-stable. Every ImeBridge implementation lives outside
 * dali-adaptor, so a method added to this interface is a hard break for all of
 * them: build the bridge against the same dali-adaptor version it runs with.
 */
class DALI_ADAPTOR_API ImeBridgeHost
{
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~ImeBridgeHost() = default;

  /**
   * @brief Delivers a completed input sequence to the application.
   * @param[in] text The committed text
   */
  virtual void Commit(const Dali::String& text) = 0;

  /**
   * @brief Delivers the current pre-edit (composing) string.
   * @param[in] text The pre-edit string
   * @param[in] cursorPosition The cursor position within @a text
   * @param[in] attrs The styling runs for @a text
   */
  virtual void PreEdit(const Dali::String&                                                        text,
                       int32_t                                                                     cursorPosition,
                       const Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) = 0;

  /**
   * @brief Notifies that a pre-edit sequence has started.
   */
  virtual void PreEditStart() = 0;

  /**
   * @brief Notifies that a pre-edit sequence has finished or was cancelled.
   */
  virtual void PreEditEnd() = 0;

  /**
   * @brief Asks the application to delete text around the cursor.
   * @param[in] cursorOffset Offset from the cursor where deletion starts
   * @param[in] numberOfChars How many characters to delete
   */
  virtual void DeleteSurrounding(int32_t cursorOffset, int32_t numberOfChars) = 0;

  /**
   * @brief Asks the application to set its selection range.
   * @param[in] startIndex The first selected character index
   * @param[in] endIndex The one-past-last selected character index
   */
  virtual void SetSelection(int32_t startIndex, int32_t endIndex) = 0;

  /**
   * @brief Forwards an IME-private command to the application.
   * @param[in] command The command string
   */
  virtual void SendPrivateCommand(const Dali::String& command) = 0;

  /**
   * @brief Delivers non-text content, such as an image.
   * @param[in] contentUri The content URI
   * @param[in] description The content description
   * @param[in] mimeTypes The content MIME types
   */
  virtual void CommitContent(const Dali::String& contentUri,
                             const Dali::String& description,
                             const Dali::String& mimeTypes) = 0;

  /**
   * @brief Asks the application to replace a range of its text in one step.
   *
   * The IME uses this to revise text it has already committed: the range
   * starting @a cursorOffset characters from the cursor is replaced by
   * @a commit, and @a preedit becomes the new composition. It is one
   * operation, not a delete followed by a commit -- an implementation that
   * decomposes it would let the application observe the intermediate state.
   *
   * @param[in] cursorOffset Offset from the cursor where the replaced range starts
   * @param[in] numberOfChars How many characters the range covers
   * @param[in] preedit The pre-edit string to leave in place, possibly empty
   * @param[in] commit The text replacing the range, possibly empty
   * @param[in] attrs The styling runs for @a preedit
   */
  virtual void RecaptureString(int32_t                                                                     cursorOffset,
                               int32_t                                                                     numberOfChars,
                               const Dali::String&                                                         preedit,
                               const Dali::String&                                                         commit,
                               const Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) = 0;

  /**
   * @brief Reads the text surrounding the cursor.
   *
   * Synchronous: the IME waits on the result. Reading has no side effects on
   * application state, so this does not go through the deferred queue.
   *
   * @param[out] text The surrounding text
   * @param[out] cursorPosition The cursor position within @a text
   * @return Whether the application supplied a result
   */
  virtual bool RetrieveSurrounding(Dali::String& text, int32_t& cursorPosition) = 0;

  /**
   * @brief Injects a key event as if it came from a keyboard.
   *
   * Used for keys the IME synthesises itself, such as Backspace or Enter,
   * which must reach the application through the normal key path rather than
   * as committed text.
   *
   * @param[in] keyEvent The key event to inject
   */
  virtual void FeedKeyEvent(const Dali::KeyEvent& keyEvent) = 0;

  /**
   * @brief Notifies that the input panel's geometry changed.
   * @param[in] area The new input panel area
   */
  virtual void InputPanelGeometryChanged(const Dali::BoundsInteger& area) = 0;

  /**
   * @brief Notifies that the input panel's visibility state changed.
   * @param[in] state The new state
   */
  virtual void InputPanelStateChanged(Dali::InputMethodContext::State state) = 0;

  /**
   * @brief Notifies that the input panel's language changed.
   *
   * The IME reports what it actually switched to, which is not necessarily
   * what the application last asked for through ImeBridge::SetLanguage():
   * the user may change the language from the panel itself. The host updates
   * what InputMethodContext::GetInputPanelLanguage() reports and raises
   * LanguageChangedSignal().
   *
   * @param[in] language The language the input panel switched to
   */
  virtual void InputPanelLanguageChanged(Dali::Integration::InputMethodContext::InputPanelLanguage language) = 0;

  /**
   * @brief Notifies that the active keyboard type changed.
   *
   * Sent when the IME starts or stops taking its input from a physical
   * keyboard. The host updates what InputMethodContext::GetKeyboardType()
   * reports and raises KeyboardTypeChangedSignal().
   *
   * @param[in] keyboardType The keyboard type now in use
   */
  virtual void KeyboardTypeChanged(Dali::InputMethodContext::KeyboardType keyboardType) = 0;

  /**
   * @brief Notifies that a bridge has just attached this host.
   *
   * Called by RegisterImeBridge() right after ImeBridge::AttachHost(). A host
   * that buffered a request while no bridge was registered replays it here.
   */
  virtual void OnBridgeAttached() = 0;

  /**
   * @brief Notifies that the bridge holding this host has just detached it.
   *
   * Called by UnregisterImeBridge(), and by RegisterImeBridge() when it
   * replaces one bridge with another, right after ImeBridge::DetachHost().
   * There is no input panel any more, so the host drops the panel state it
   * was caching on the IME's behalf and tells the application: otherwise a
   * context whose panel was showing keeps reporting SHOW, with panel
   * geometry, for a panel that no longer exists.
   */
  virtual void OnBridgeDetached() = 0;
};

/**
 * @brief Interface for an in-process IME, registered with the Adaptor.
 *
 * Implemented by the layer that owns the IME and registered here so that the
 * Adaptor's input method context can drive it without a reverse dependency on
 * that layer. Every method takes the calling ImeBridgeHost so the
 * implementation can tell input contexts apart; an application may have many.
 *
 * @note Threading: every method is called on the main/event thread only, and
 * an implementation must only call back into ImeBridgeHost from that thread.
 * Neither the process-wide bridge pointer nor the Adaptor's host list is
 * synchronised.
 *
 * @note Re-entrancy: a host call made from inside a bridge call can re-enter
 * the bridge synchronously. The mandatory case is composition: the Adaptor's
 * ImeBridgeHost::PreEdit() implementation calls Reset() on this bridge when
 * the application asks for the pre-edit to be dropped, so a bridge that
 * delivers a pre-edit by calling host->PreEdit() will see Reset() arrive
 * before its own call returns. An implementation must therefore guard its own
 * state against re-entry -- typically by finishing every state change before
 * calling into a host -- rather than assuming a call it makes runs to
 * completion first.
 *
 * @note Not ABI-stable. Every implementation lives outside dali-adaptor, so a
 * method added to this interface is a hard break for all of them: build the
 * bridge against the same dali-adaptor version it runs with.
 */
class DALI_ADAPTOR_API ImeBridge
{
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~ImeBridge() = default;

  /**
   * @brief Registers an input method context with this bridge.
   *
   * The pointer is owned by the Adaptor and is valid only from this call
   * until the matching DetachHost(). The bridge must not retain it beyond
   * that: the input method context is destroyed when the control using it
   * goes away, and DetachHost() is the only notice given.
   *
   * @param[in] host The host to attach
   */
  virtual void AttachHost(ImeBridgeHost* host) = 0;

  /**
   * @brief Unregisters an input method context from this bridge.
   *
   * After this returns, @a host must no longer be called or stored: it may be
   * destroyed immediately afterwards.
   *
   * @param[in] host The host to detach
   */
  virtual void DetachHost(ImeBridgeHost* host) = 0;

  /**
   * @brief Notifies that @a host gained keyboard focus.
   * @param[in] host The calling host
   */
  virtual void Activate(ImeBridgeHost* host) = 0;

  /**
   * @brief Notifies that @a host lost keyboard focus.
   * @param[in] host The calling host
   */
  virtual void Deactivate(ImeBridgeHost* host) = 0;

  /**
   * @brief Asks the IME to abandon any in-progress composition.
   * @param[in] host The calling host
   */
  virtual void Reset(ImeBridgeHost* host) = 0;

  /**
   * @brief Asks the IME to show its input panel.
   * @param[in] host The calling host
   * @return Whether the request was accepted
   */
  virtual bool ShowInputPanel(ImeBridgeHost* host) = 0;

  /**
   * @brief Asks the IME to hide its input panel.
   * @param[in] host The calling host
   * @return Whether the request was accepted
   */
  virtual bool HideInputPanel(ImeBridgeHost* host) = 0;

  /**
   * @brief Offers a key event to the IME.
   *
   * Synchronous: the return value decides whether the application consumes the
   * key, so this cannot be deferred.
   *
   * @param[in] host The calling host
   * @param[in] keyEvent The key event to offer
   * @return Whether the IME consumed the key
   */
  virtual bool FilterEventKey(ImeBridgeHost* host, const Dali::KeyEvent& keyEvent) = 0;

  /**
   * @brief Notifies the IME of the application's cursor position.
   * @param[in] host The calling host
   * @param[in] position The cursor position
   */
  virtual void SetCursorPosition(ImeBridgeHost* host, uint32_t position) = 0;

  /**
   * @brief Gives the IME the text surrounding the cursor.
   * @param[in] host The calling host
   * @param[in] text The surrounding text
   */
  virtual void SetSurroundingText(ImeBridgeHost* host, const Dali::String& text) = 0;

  /**
   * @brief Sets the requested input panel layout.
   * @param[in] host The calling host
   * @param[in] layout The panel layout
   * @param[in] variation The layout variation
   */
  virtual void SetPanelLayout(ImeBridgeHost*                          host,
                              Dali::InputMethod::PanelLayout          layout,
                              Dali::InputMethod::PanelLayoutVariation variation) = 0;

  /**
   * @brief Sets the return key's appearance and whether it is enabled.
   * @param[in] host The calling host
   * @param[in] type The return key type
   * @param[in] enabled Whether the return key is enabled
   */
  virtual void SetReturnKey(ImeBridgeHost* host, Dali::InputMethod::ReturnKeyType type, bool enabled) = 0;

  /**
   * @brief Sets the auto-capitalisation behaviour.
   * @param[in] host The calling host
   * @param[in] type The auto-capitalisation type
   */
  virtual void SetAutoCapital(ImeBridgeHost* host, Dali::InputMethod::AutoCapitalType type) = 0;

  /**
   * @brief Sets whether the IME may offer word predictions.
   * @param[in] host The calling host
   * @param[in] allowed Whether prediction is allowed
   */
  virtual void SetTextPrediction(ImeBridgeHost* host, bool allowed) = 0;

  /**
   * @brief Passes application-defined data through to the IME.
   * @param[in] host The calling host
   * @param[in] data The application-specific data
   */
  virtual void SetUserData(ImeBridgeHost* host, const Dali::String& data) = 0;

  /**
   * @brief Reads the application-defined data the IME currently holds.
   *
   * Synchronous: the application waits on the result, and reading has no
   * side effects on IME state.
   *
   * The answer is the IME's, not an echo of the last SetUserData(): an IME
   * may revise the data or never take it at all. Return an empty string when
   * the IME does not expose it, which is what the caller treats as "no answer".
   *
   * @param[in] host The calling host
   * @return The IME's current user data, or an empty string
   */
  virtual Dali::String GetUserData(ImeBridgeHost* host) = 0;

  /**
   * @brief Sets the requested input panel language.
   * @param[in] host The calling host
   * @param[in] language The panel language
   */
  virtual void SetLanguage(ImeBridgeHost* host, Dali::Integration::InputMethodContext::InputPanelLanguage language) = 0;

  /**
   * @brief Sets the MIME types the application accepts as committed content.
   * @param[in] host The calling host
   * @param[in] mimeTypes The accepted MIME types
   */
  virtual void SetContentMimeTypes(ImeBridgeHost* host, const Dali::String& mimeTypes) = 0;

  /**
   * @brief Sets the input panel position.
   * @param[in] host The calling host
   * @param[in] x The x coordinate
   * @param[in] y The y coordinate
   */
  virtual void SetPanelPosition(ImeBridgeHost* host, int32_t x, int32_t y) = 0;

  /**
   * @brief Sets the input panel position and its alignment anchor.
   * @param[in] host The calling host
   * @param[in] x The x coordinate
   * @param[in] y The y coordinate
   * @param[in] align The alignment anchor
   */
  virtual void SetPanelPositionAlign(ImeBridgeHost*                            host,
                                     int32_t                                   x,
                                     int32_t                                   y,
                                     Dali::InputMethodContext::InputPanelAlign align) = 0;

  /**
   * @brief Sets whether the edited text may span more than one line.
   *
   * Decides what the return key does: insert a newline, or act on the
   * ImeBridge::SetReturnKey() action.
   *
   * @param[in] host The calling host
   * @param[in] multiLine Whether the text is multi-line
   */
  virtual void SetMultiLine(ImeBridgeHost* host, bool multiLine) = 0;
};

/**
 * @brief Registers the process-wide IME bridge.
 *
 * The caller retains ownership; the pointer must outlive its registration.
 * Every ImeBridgeHost alive at this point is attached to @a bridge and told
 * ImeBridgeHost::OnBridgeAttached(). If a different bridge was registered, its
 * hosts are detached and told ImeBridgeHost::OnBridgeDetached() first.
 * Registering the bridge that is already registered does nothing.
 *
 * @note Main/event thread only. Neither the bridge pointer nor the host list
 * is synchronised.
 *
 * @note The per-host notifications are issued over a snapshot of the host
 * list, and each host's liveness is re-checked immediately before its own
 * notification. A host destroyed while an earlier host in the same pass was
 * being notified -- OnBridgeDetached() raises application-facing signals, and
 * a handler may destroy a text control -- is skipped rather than called. Do
 * not rely on receiving exactly one notification per host that was alive when
 * this was called.
 *
 * @param[in] bridge The bridge implementation to register
 */
DALI_ADAPTOR_API void RegisterImeBridge(ImeBridge* bridge);

/**
 * @brief Unregisters the IME bridge if it is currently registered.
 *
 * Every ImeBridgeHost is detached from @a bridge and told
 * ImeBridgeHost::OnBridgeDetached(). Passing a bridge that is not the
 * registered one does nothing.
 *
 * @note Main/event thread only. Neither the bridge pointer nor the host list
 * is synchronised.
 *
 * @note Same skip-if-destroyed rule as RegisterImeBridge(): a host destroyed
 * by an application handler that ran during an earlier host's
 * OnBridgeDetached() in this same pass is not called.
 *
 * @param[in] bridge The bridge implementation to unregister
 */
DALI_ADAPTOR_API void UnregisterImeBridge(ImeBridge* bridge);

/**
 * @brief Returns the process-wide IME bridge.
 *
 * @note Main/event thread only. Neither the bridge pointer nor the host list
 * is synchronised.
 *
 * @return The registered bridge, or nullptr when none is registered
 */
DALI_ADAPTOR_API ImeBridge* GetImeBridge();

} // namespace Integration
} // namespace Dali

#endif // DALI_INTEGRATION_API_ADAPTOR_FRAMEWORK_IME_BRIDGE_H
