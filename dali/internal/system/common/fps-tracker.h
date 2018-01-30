#ifndef __DALI_INTERNAL_FPS_TRACKER_H__
#define __DALI_INTERNAL_FPS_TRACKER_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class EnvironmentOptions;

/**
 * Tracks the frames per second.
 *
 * Can also output the FPS to a file if required.
 */
class FpsTracker
{
public:

  /**
   * Create the FPS Tracker.
   * @param[in] environmentOptions environment options
   */
  FpsTracker( const EnvironmentOptions& environmentOptions );

  /**
   * Non-virtual destructor; UpdateThread is not suitable as a base class.
   */
  ~FpsTracker();

  /**
   * When DALI_FPS_TRACKING is enabled, this method calculates the frame rates for the specified time period
   *
   * @param[in] secondsFromLastFrame The time (in seconds) that has elapsed since the last frame.
   */
  void Track(float secondsFromLastFrame);

  /**
   * @return Whether FPS tracking is enabled.
   */
  bool Enabled() const;

private:

  /**
   * Output the FPS information
   * when the FSP tracking is enabled,
   * it is called when the specified tracking period is elapsed or in the destructor when the process finished beforehand
   */
  void OutputFPSRecord();

private: // Data

  float mFpsTrackingSeconds;  ///< fps tracking time length in seconds
  float mFrameCount;          ///< how many frames occurred during tracking period
  float mElapsedTime;         ///< time elapsed from previous fps tracking output
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_FPS_TRACKER_H__
