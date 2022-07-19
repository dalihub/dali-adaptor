#ifndef DALI_WEB_ENGINE_CONTEXT_MENU_ITEM_H
#define DALI_WEB_ENGINE_CONTEXT_MENU_ITEM_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <memory>
#include <string>

namespace Dali
{
class WebEngineContextMenu;

/**
 * @brief A class WebEngineContextMenuItem for context menu item of web engine.
 */
class WebEngineContextMenuItem
{
public:
  /**
   * @brief Enumeration that provides the tags of items for the context menu.
   */
  enum class ItemTag
  {
    NO_ACTION = 0,                  ///< No action
    OPEN_LINK_IN_NEW_WINDOW,        ///< Open link in new window
    DOWNLOAD_LINK_TO_DISK,          ///< Download link to disk
    COPY_LINK_TO_CLIPBOARD,         ///< Copy link to clipboard
    OPEN_IMAGE_IN_NEW_WINDOW,       ///< Open image in new window
    OPEN_IMAGE_IN_CURRENT_WINDOW,   ///< Open image in current window
    DOWNLOAD_IMAGE_TO_DISK,         ///< Download image to disk
    COPY_IMAGE_TO_CLIPBOARD,        ///< Copy image to clipboard
    OPEN_FRAME_IN_NEW_WINDOW,       ///< Open frame in new window
    COPY,                           ///< Copy
    GO_BACK,                        ///< Go back
    GO_FORWARD,                     ///< Go forward
    STOP,                           ///< Stop
    SHARE,                          ///< Share
    RELOAD,                         ///< Reload
    CUT,                            ///< Cut
    PASTE,                          ///< Paste
    SPELLING_GUESS,                 ///< Spelling guess
    NO_GUESSES_FOUND,               ///< Guess found
    IGNORE_SPELLING,                ///< Ignore spelling
    LEARN_SPELLING,                 ///< Learn spelling
    OTHER,                          ///< Other
    SEARCH_IN_SPOTLIGHT,            ///< Search in spotlight
    SEARCH_WEB,                     ///< Search web
    LOOK_UP_IN_DICTIONARY,          ///< Look up in dictionary
    OPEN_WITH_DEFAULT_APPLICATION,  ///< Open with default application
    PDF_ACTUAL_SIZE,                ///< PDF actual size
    PDF_ZOOM_IN,                    ///< PDF zoom in
    PDF_ZOOM_OUT,                   ///< PDF zoom out
    PDF_AUTO_SIZE,                  ///< PDF auto size
    PDF_SINGLE_PAGE,                ///< PDF single page
    PDF_FACTING_PAGES,              ///< PDF facting page
    PDF_CONTINUOUS,                 ///< PDF continuous
    PDF_NEXT_PAGE,                  ///< PDF next page
    PDF_PREVIOUS_PAGE,              ///< PDF previous page
    OPEN_LINK,                      ///< Open link
    IGNORE_GRAMMAR,                 ///< Ignore grammar
    SPELLING_MENU,                  ///< Spelling menu
    SHOW_SPELLING_PANEL,            ///< Show spelling panel
    CHECK_SPELLING,                 ///< Check spelling
    CHECK_SPELLING_WHILE_TYPING,    ///< Check spelling white typing
    CHECK_GRAMMAR_WITH_SPELLING,    ///< Check grammar with spelling
    FONT_MENU,                      ///< Font menu
    SHOW_FONTS,                     ///< Show fonts
    BOLD,                           ///< Bold
    ITALIC,                         ///< Italic
    UNDERLINE,                      ///< Underline
    OUTLINE,                        ///< Outline
    STYLES,                         ///< Style
    SHOW_COLORS,                    ///< Show colors
    SPEECH_MENU,                    ///< Speech menu
    START_SPEAKING,                 ///< Start speaking
    STOP_SPEAKING,                  ///< Stop speaking
    WRITING_DIRECTION_MENU,         ///< Writing direction menu
    DEFAULT_DIRECTION,              ///< Default direction
    LEFT_TO_RIGHT,                  ///< Left to right
    RIGHT_TO_LEFT,                  ///< Right to left
    PDF_SINGLE_PAGE_SCROLLING,      ///< PDF single page scrolling
    PDF_FACING_PAGES_SCROLLING,     ///< PDF facing page scrolling
    INSPECT_ELEMENT,                ///< Inspect element
    TEXT_DIRECTION_MENU,            ///< Text direction menu
    TEXT_DIRECTION_DEFAULT,         ///< Text direction default
    TEXT_DIRECTION_LEFT_TO_RIGHT,   ///< Text direction left to right
    TEXT_DIRECTION_RIGHT_TO_LEFT,   ///< Text direction right to left
    CORRECT_SPELLING_AUTOMATICALLY, ///< Correct spelling automatically
    SUBSTITUTIONS_MENU,             ///< Substitutions menu
    SHOW_SUBSTITUTIONS,             ///< Show substitutions
    SMART_COPY_PASTE,               ///< Smart copy paste
    SMART_QUOTES,                   ///< Smart quotes
    SMART_DASHES,                   ///< Smart dashes
    SMART_LINKS,                    ///< Smart links
    TEXT_REPLACEMENT,               ///< Text replacement
    TRANSFORMATIONS_MENU,           ///< Transformation menu
    MAKE_UPPER_CASE,                ///< Make upper case
    MAKE_LOWER_CASE,                ///< Make lower case
    CAPITALIZE,                     ///< Capitalize
    CHANGE_BACK,                    ///< Change back
    OPEN_MEDIA_IN_NEW_WINDOW,       ///< Open media in new window
    COPY_MEDIA_LINK_TO_CLIPBOARD,   ///< Copy media link to clipboard
    TOGGLE_MEDIA_CONTROLS,          ///< Toggle media controls
    TOGGLE_MEDIA_LOOP,              ///< Toggle media loop
    ENTER_VIDEO_FULLSCREEN,         ///< Enter video fullscreen
    MEDIA_PLAY_PAUSE,               ///< Media play pause
    MEDIA_MUTE,                     ///< Media mute
    DICTATION_ALTERNATIVE,          ///< Dictation alternative
    SELECT_ALL,                     ///< Select all
    SELECT_WORD,                    ///< Select word
    TEXT_SELECTION_MODE,            ///< Text selection mode
    CLIPBOARD,                      ///< Clipboard
    DRAG,                           ///< Drag
    TRANSLATE,                      ///< Translate
    COPY_LINK_DATA,                 ///< Copy link data
  };

  /**
   * @brief Enumeration that defines the types of the items for the context menu.
   */
  enum class ItemType
  {
    ACTION,           ///< Action
    CHECKABLE_ACTION, ///< Checkable action
    SEPARATOR,        ///< Separator
    SUBMENU,          ///< Sub-menu
  };

  /**
   * @brief Constructor.
   */
  WebEngineContextMenuItem() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineContextMenuItem() = default;

  /**
   * @brief Return the tag of context menu item.
   *
   * @return The tag of context menu item
   */
  virtual ItemTag GetTag() const = 0;

  /**
   * @brief Return the type of context menu item.
   *
   * @return The type of context menu item
   */
  virtual ItemType GetType() const = 0;

  /**
   * @brief Check if the item is enabled or not.
   *
   * @return true if it is enabled, false otherwise
   */
  virtual bool IsEnabled() const = 0;

  /**
   * @brief Return the link url of context menu item.
   *
   * @return The current link url if succeeded, empty string otherwise
   */
  virtual std::string GetLinkUrl() const = 0;

  /**
   * @brief Return the image url of context menu item.
   *
   * @return The current image url if succeeded, empty string otherwise
   */
  virtual std::string GetImageUrl() const = 0;

  /**
   * @brief Get a title of the item.
   *
   * @return a title of the item if succeeded, empty string otherwise
   */
  virtual std::string GetTitle() const = 0;

  /**
   * @brief Get the parent menu for the item.
   *
   * @return parent menu if succeeded, 0 otherwise
   */
  virtual std::unique_ptr<Dali::WebEngineContextMenu> GetParentMenu() const = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_CONTEXT_MENU_ITEM_H
