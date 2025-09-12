#ifndef DALI_WINDOW_DATA_H
#define DALI_WINDOW_DATA_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/math/rect.h>
#include <memory>
#include <string>

namespace Dali
{
/**
 * The WindowData class is used as a parameter for the constructors of the Application class.
 * The data from the WindowData object is used to customize the default window created by the Application class.
 *
 * The default values are below:
 * PositionSize : x:0, y:0, w:0, h:0 (full-screen window)
 * Transparency : true (Window is created with 32-bit color depth)
 * WindowType : NORMAL
 * mIsFrontBufferRendering : false
 *
 * If you want to customize the window, you can modify the values of the WindowData object as needed.
 * @SINCE_2_2.23
 */
class DALI_ADAPTOR_API WindowData
{
public:
  /**
   * @brief Creates a WindowData object.
   */
  WindowData();

  /**
   * @brief Destructor.
   */
  ~WindowData();

  /**
   * @brief Sets the position and size
   *
   * @SINCE_2_2.23
   * @param[in] positionSize Position and Size
   */
  void SetPositionSize(Dali::Rect<int>& positionSize);

  /**
   * @brief Gets the PositionSize
   *
   * @SINCE_2_2.23
   * @return The position and size
   */
  Dali::Rect<int> GetPositionSize() const;

  /**
   * @brief Sets the transparency
   *
   * @SINCE_2_2.23
   * @param[in] transparent transparency
   */
  void SetTransparency(bool transparent);

  /**
   * @brief Gets the transparency
   *
   * @SINCE_2_2.23
   * @return whether transparency
   */
  bool GetTransparency() const;

  /**
   * @brief Sets the window type
   *
   * @SINCE_2_2.23
   * @param[in] type the window type
   */
  void SetWindowType(WindowType type);

  /**
   * @brief Gets the window type
   *
   * @SINCE_2_2.23
   * @return the window type
   */
  WindowType GetWindowType() const;

  /**
   * @brief Sets the front buffer rendering
   *
   * @SINCE_2_2.51
   * @param[in] enable whether to use front buffer rendering
   */
  void SetFrontBufferRendering(bool enable);

  /**
   * @brief Gets the front buffer rendering
   *
   * @SINCE_2_2.51
   * @return whether front buffer rendering is enabled
   */
  bool GetFrontBufferRendering() const;

  /**
   * @brief Sets the screen for the window
   * It is base information from DevelWindowSystem::GetAvailableScreens
   *
   * This method sets the window to the specified screen.
   * The window will be moved to the specified screen.
   *
   * @SINCE_2_4.35
   * @param[in] screen The name of the screen to set
   */
  void SetScreen(const std::string& screen);

  /**
   * @brief Gets the screen
   *
   * @SINCE_2_4.35
   * @return screen name
   */
  std::string GetScreen() const;

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

} // namespace Dali

#endif // DALI_WINDOW_DATA_H
