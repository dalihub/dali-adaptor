#ifndef __DALI_PLATFORM_FONT_CONTROLLER_H__
#define __DALI_PLATFORM_FONT_CONTROLLER_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// INTERNAL INCLUDES (for TextArray)
#include <dali/integration-api/platform-abstraction.h>

// EXTERNAL INCLUDES
#include <string>

namespace Dali
{

namespace Platform
{

/**
 * Abstract interface for the font controller.
 * Hides platform specific code for managing fonts.
 */
class FontController
{
public:

  /**
   * The mode for GetFontList()
   */
  enum FontListMode
  {
    LIST_SYSTEM_FONTS,                   ///< List system fonts
    LIST_APPLICATION_FONTS,              ///< List application fonts
    LIST_ALL_FONTS                       ///< List all fonts
  };

  /**
   * Defines a font family with style.
   */
  typedef std::pair<std::string/*font family*/,std::string/*font style*/> StyledFontFamily;

  /**
   * Font list typedef.
   */
  typedef std::vector<StyledFontFamily> FontList;

  /**
   * Create a new font controller.
   */
  static FontController* New();


  /**
   * Destructor.
   */
  virtual ~FontController()
  {
  }

  /**
   * Given a font family return a font path.
   * @param[in] styledFontFamily The name of the font's family and the font's style.
   * @return font file name.
   */
  virtual std::string GetFontPath( const StyledFontFamily& styledFontFamily ) = 0;

  /**
   * Gets a list of fonts installed on the system.
   * @param fontListMode which fonts to include in the list
   * @return a list of styled font family names.
   */
  virtual FontList GetFontList( FontListMode fontListMode ) = 0;

  /**
   * Checks whether fontName is a valid font family name.
   * closestMatch is always set to the best matching font or the system default font if no near match is detected.
   * @param[in]  styledFontFamily The name of the font's family and the font's style.
   * @param[out] isDefaultSystemFont Whether this font has been created with a default system font.
   * @param[out] closestStyledFontFamilyMatch The name of the font's family and the font's style found based on the given input.
   * @return     true if styledFontFamily is valid, false otherwise.
   */
  virtual bool ValidateFontFamilyName(const StyledFontFamily& styledFontFamily, bool& isDefaultSystemFont, StyledFontFamily& closestStyledFontFamilyMatch) = 0;

  /**
   * Returns font family name for displayed text.
   * If possible, the returned font name should be able to display all characters in text.
   * Otherwise returns closest match.
   * @param[in] charsRequested displayed text.
   */
  virtual StyledFontFamily GetFontFamilyForChars(const TextArray& charsRequested) = 0;

  /**
   * Checks whether all characters of text could be displayed with specified font family.
   * @param[in] styledFontFamily The name of the font's family and the font's style.
   * @param[in] text displayed text.
   */
  virtual bool AllGlyphsSupported(const StyledFontFamily& styledFontFamily, const TextArray& text) = 0;

  /**
   * Set the default font family and its style that should be used by the font controller.
   * @param[in] styledFontFamily The default name of the font's family and the default font's style.
   */
  virtual void SetDefaultFontFamily( const StyledFontFamily& styledFontFamily ) = 0;

protected:

  /**
   * Constructor
   */
  FontController()
  {
  }

private:

  // Undefined copy constructor.
  FontController( const FontController& );

  // Undefined assignment operator.
  FontController& operator=( const FontController& );
};

} // namespace Platform

} // namespace Dali

#endif // __DALI_PLATFORM_FONT_CONTROLLER_H__
