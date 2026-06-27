#ifndef DALI_INTEGRATION_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_INTEG_H
#define DALI_INTEGRATION_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_INTEG_H

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

#include <dali/integration-api/adaptor-framework/accessibility/accessibility-bitset.h>
#include <cstdint>

namespace Dali
{
namespace Integration
{
namespace Accessibility
{

/**
 * @brief Enumeration describing states of the Accessibility object.
 *
 * Object can be in many states at the same time.
 *
 * This is the AT-SPI/export state vocabulary used by adaptor integration,
 * toolkit compatibility code, and legacy language bindings. Application and
 * component code should prefer toolkit-level settable accessibility states.
 */
enum class State : uint32_t
{
  INVALID,
  ACTIVE,
  ARMED,
  BUSY,
  CHECKED,
  COLLAPSED,
  DEFUNCT,
  EDITABLE,
  ENABLED,
  EXPANDABLE,
  EXPANDED,
  FOCUSABLE,
  FOCUSED,
  HAS_TOOLTIP,
  HORIZONTAL,
  ICONIFIED,
  MODAL,
  MULTI_LINE,
  MULTI_SELECTABLE,
  OPAQUE,
  PRESSED,
  RESIZEABLE,
  SELECTABLE,
  SELECTED,
  SENSITIVE,
  SHOWING,
  SINGLE_LINE,
  STALE,
  TRANSIENT,
  VERTICAL,
  VISIBLE,
  MANAGES_DESCENDANTS,
  INDETERMINATE,
  REQUIRED,
  TRUNCATED,
  ANIMATED,
  INVALID_ENTRY,
  SUPPORTS_AUTOCOMPLETION,
  SELECTABLE_TEXT,
  IS_DEFAULT,
  VISITED,
  CHECKABLE,
  HAS_POPUP,
  READ_ONLY,
  HIGHLIGHTED,
  HIGHLIGHTABLE,
  MAX_COUNT
};

/**
 * @brief Enumeration describing role of the Accessibility object.
 *
 * This is the AT-SPI/export role vocabulary used by adaptor integration,
 * toolkit compatibility code, and legacy language bindings. Application and
 * component code should prefer toolkit-level accessibility roles.
 */
enum class Role : uint32_t
{
  INVALID,
  ACCELERATOR_LABEL,
  ALERT,
  ANIMATION,
  ARROW,
  CALENDAR,
  CANVAS,
  CHECK_BOX,
  CHECK_MENU_ITEM,
  COLOR_CHOOSER,
  COLUMN_HEADER,
  COMBO_BOX,
  DATE_EDITOR,
  DESKTOP_ICON,
  DESKTOP_FRAME,
  DIAL,
  DIALOG,
  DIRECTORY_PANE,
  DRAWING_AREA,
  FILE_CHOOSER,
  FILLER,
  FOCUS_TRAVERSABLE,
  FONT_CHOOSER,
  FRAME,
  GLASS_PANE,
  HTML_CONTAINER,
  ICON,
  IMAGE,
  INTERNAL_FRAME,
  LABEL,
  LAYERED_PANE,
  LIST,
  LIST_ITEM,
  MENU,
  MENU_BAR,
  MENU_ITEM,
  OPTION_PANE,
  PAGE_TAB,
  PAGE_TAB_LIST,
  PANEL,
  PASSWORD_TEXT,
  POPUP_MENU,
  PROGRESS_BAR,
  PUSH_BUTTON,
  RADIO_BUTTON,
  RADIO_MENU_ITEM,
  ROOT_PANE,
  ROW_HEADER,
  SCROLL_BAR,
  SCROLL_PANE,
  SEPARATOR,
  SLIDER,
  SPIN_BUTTON,
  SPLIT_PANE,
  STATUS_BAR,
  TABLE,
  TABLE_CELL,
  TABLE_COLUMN_HEADER,
  TABLE_ROW_HEADER,
  TEAROFF_MENU_ITEM,
  TERMINAL,
  TEXT,
  TOGGLE_BUTTON,
  TOOL_BAR,
  TOOL_TIP,
  TREE,
  TREE_TABLE,
  UNKNOWN,
  VIEWPORT,
  WINDOW,
  EXTENDED,
  HEADER,
  FOOTER,
  PARAGRAPH,
  RULER,
  APPLICATION,
  AUTOCOMPLETE,
  EDITBAR,
  EMBEDDED,
  ENTRY,
  CHART,
  CAPTION,
  DOCUMENT_FRAME,
  HEADING,
  PAGE,
  SECTION,
  REDUNDANT_OBJECT,
  FORM,
  LINK,
  INPUT_METHOD_WINDOW,
  TABLE_ROW,
  TREE_ITEM,
  DOCUMENT_SPREADSHEET,
  DOCUMENT_PRESENTATION,
  DOCUMENT_TEXT,
  DOCUMENT_WEB,
  DOCUMENT_EMAIL,
  COMMENT,
  LIST_BOX,
  GROUPING,
  IMAGE_MAP,
  NOTIFICATION,
  INFO_BAR,
  LEVEL_BAR,
  TITLE_BAR,
  BLOCK_QUOTE,
  AUDIO,
  VIDEO,
  DEFINITION,
  ARTICLE,
  LANDMARK,
  LOG,
  MARQUEE,
  MATH,
  RATING,
  TIMER,
  STATIC,
  MATH_FRACTION,
  MATH_ROOT,
  SUBSCRIPT,
  SUPERSCRIPT,
  MAX_COUNT
};

/**
 * @brief Enumeration describing a relation between accessible objects.
 *
 * This is the AT-SPI/export relation vocabulary used by adaptor integration.
 * UI-facing APIs should use their own toolkit-level relation vocabulary and
 * convert at the adaptor boundary.
 */
enum class RelationType : uint32_t
{
  NULL_OF,
  LABEL_FOR,
  LABELLED_BY,
  CONTROLLER_FOR,
  CONTROLLED_BY,
  MEMBER_OF,
  TOOLTIP_FOR,
  NODE_CHILD_OF,
  NODE_PARENT_OF,
  EXTENDED,
  FLOWS_TO,
  FLOWS_FROM,
  SUBWINDOW_OF,
  EMBEDS,
  EMBEDDED_BY,
  POPUP_FOR,
  PARENT_WINDOW_OF,
  DESCRIPTION_FOR,
  DESCRIBED_BY,
  DETAILS,
  DETAILS_FOR,
  ERROR_MESSAGE,
  ERROR_FOR,
  MAX_COUNT
};

/**
 * @brief Enumeration describing which information is read for an accessible object.
 */
enum class ReadingInfoType
{
  NAME,
  ROLE,
  DESCRIPTION,
  STATE,
  MAX_COUNT
};

/**
 * @brief Enumeration of accessibility interfaces an accessible object can expose.
 */
enum class AccessibilityInterface
{
  ACCESSIBLE,
  ACTION,
  APPLICATION,
  CACHE,
  COLLECTION,
  COMPONENT,
  DEVICE_EVENT_CONTROLLER,
  DEVICE_EVENT_LISTENER,
  DOCUMENT,
  EDITABLE_TEXT,
  EVENT_DOCUMENT,
  EVENT_FOCUS,
  EVENT_KEYBOARD,
  EVENT_MOUSE,
  EVENT_OBJECT,
  EVENT_TERMINAL,
  EVENT_WINDOW,
  HYPERLINK,
  HYPERTEXT,
  IMAGE,
  REGISTRY,
  SELECTION,
  SOCKET,
  TABLE,
  TABLE_CELL,
  TEXT,
  VALUE,
  MAX_COUNT
};

/**
 * @brief Enumeration of accessibility events that can be suppressed.
 */
enum class AccessibilityEvent
{
  PROPERTY_CHANGED,
  BOUNDS_CHANGED,
  LINK_SELECTED,
  STATE_CHANGED,
  CHILDREN_CHANGED,
  VISIBLE_DATA_CHANGED,
  SELECTION_CHANGED,
  MODEL_CHANGED,
  ACTIVE_DESCENDANT_CHANGED,
  ROW_INSERTED,
  ROW_REORDERED,
  ROW_DELETED,
  COLUMN_INSERTED,
  COLUMN_REORDERED,
  COLUMN_DELETED,
  TEXT_BOUNDS_CHANGED,
  TEXT_SELECTION_CHANGED,
  TEXT_CHANGED,
  TEXT_ATTRIBUTES_CHANGED,
  TEXT_CARET_MOVED,
  ATTRIBUTES_CHANGED,
  MOVED_OUT,
  WINDOW_CHANGED,
  SCROLL_STARTED,
  SCROLL_FINISHED,
  MAX_COUNT
};

using AccessibilityInterfaces = EnumBitSet<AccessibilityInterface, AccessibilityInterface::MAX_COUNT>;
using AccessibilityEvents     = EnumBitSet<AccessibilityEvent, AccessibilityEvent::MAX_COUNT>;
using ReadingInfoTypes        = EnumBitSet<ReadingInfoType, ReadingInfoType::MAX_COUNT>;
using States                  = EnumBitSet<State, State::MAX_COUNT>;
using RoleSet                 = EnumBitSet<Role, Role::MAX_COUNT>;

/*
 * Accessibility interfaces exposed as native C++ types should specialize this like so:
 *
 * template<>
 * struct AccessibilityInterfaceTypeHelper<AccessibilityInterface::ACCESSIBLE>
 * {
 *   using Type = Dali::Accessibility::Accessible;
 * };
 */
template<AccessibilityInterface I>
struct AccessibilityInterfaceTypeHelper; // no default definition

/**
 * @brief Resolves to the native C++ type that represents the given accessibility interface.
 *
 * For example, @code AccessibilityInterfaceType<AccessibilityInterface::ACCESSIBLE> @endcode is the same as
 * @code Dali::Accessibility::Accessible @endcode. Not all accessibility interfaces have native C++
 * representations (in which case, such an expression will not compile).
 *
 * @tparam I Enumeration value indicating the requested accessibility interface.
 */
template<AccessibilityInterface I>
using AccessibilityInterfaceType = typename AccessibilityInterfaceTypeHelper<I>::Type;

} // namespace Accessibility
} // namespace Integration
} // namespace Dali

#endif // DALI_INTEGRATION_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_INTEG_H
