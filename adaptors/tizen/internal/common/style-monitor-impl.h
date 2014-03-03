#ifndef __DALI_INTERNAL_STYLE_MONITOR_H__
#define __DALI_INTERNAL_STYLE_MONITOR_H__

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/adaptor-framework/common/style-monitor.h>
#include <dali/integration-api/platform-abstraction.h>

// INTERNAL INCLUDES

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * This holds the platform's style information.
 * It provides a signal when any aspect of the default style changes on the device.
 */
class StyleMonitor : public BaseObject
{
public:

  // Creation & Destruction

  /**
   * Constructor.
   * @param[in]  platformAbstraction  The platform abstraction.
   */
  StyleMonitor(Integration::PlatformAbstraction& platformAbstraction);

  /**
   * Retrieve the initialized instance of the StyleMonitor.
   * @return Handle to StyleMonitor.
   */
  static Dali::StyleMonitor Get();

  // Style Change Notifications

  /**
   * Informs the Style Monitor that the style has changed.
   * @param[in]  styleChange  The details of the change.
   */
  void StyleChanged(StyleChange styleChange);

  // Style Information

  /**
   * @copydoc Dali::StyleMonitor::GetDefaultFontFamily() const
   */
  std::string GetDefaultFontFamily() const;

  /**
   * @copydoc Dali::StyleMonitor::GetDefaultFontSize() const
   */
  float GetDefaultFontSize() const;

  /**
   * @copydoc Dali::StyleMonitor::GetTheme() const
   */
  const std::string& GetTheme() const;

  /**
   * @copydoc Dali::StyleMonitor::SetTheme()
   */
  void SetTheme(const std::string& themeFilePath);

  // Signals

  /**
   * @copydoc Dali::StyleMonitor::StyleChangeSignal()
   */
  Dali::StyleMonitor::StyleChangeSignalV2& StyleChangeSignal();

protected:

  /**
   * Virtual Destructor.
   */
  virtual ~StyleMonitor();

private:

  /**
   * Emit the style change signal.
   * @param[in]  styleChange  The details of the style change
   */
  inline void EmitStyleChangeSignal(StyleChange styleChange);

private:

  Dali::StyleMonitor::StyleChangeSignalV2      mStyleChangeSignalV2; ///< Emitted when the style changes

  Integration::PlatformAbstraction&            mPlatformAbstraction; ///< Reference to the PlatformAbstraction (for retrieving defaults)
  std::string                                  mUserDefinedThemeFilePath;///< String containing the user defined theme file path

};

} // namespace Adaptor

} // namespace Internal

// Additional Helpers for public-api forwarding methods

inline Internal::Adaptor::StyleMonitor& GetImplementation(Dali::StyleMonitor& monitor)
{
  DALI_ASSERT_ALWAYS(monitor && "Monitor handle is empty");
  BaseObject& handle = monitor.GetBaseObject();
  return static_cast<Internal::Adaptor::StyleMonitor&>(handle);
}

inline const Internal::Adaptor::StyleMonitor& GetImplementation(const Dali::StyleMonitor& monitor)
{
  DALI_ASSERT_ALWAYS(monitor && "Monitor handle is empty");
  const BaseObject& handle = monitor.GetBaseObject();
  return static_cast<const Internal::Adaptor::StyleMonitor&>(handle);
}

} // namespace Dali

#endif // __DALI_INTERNAL_STYLE_MONITOR_H__
