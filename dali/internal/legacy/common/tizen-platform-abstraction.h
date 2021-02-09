#ifndef DALI_TIZEN_PLATFORM_ABSTRACTION_H
#define DALI_TIZEN_PLATFORM_ABSTRACTION_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/dali-adaptor-common.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/platform-abstraction.h>
#include <cstdint>
#include <memory>
#include <string>

namespace Dali
{
class CallbackBase;

namespace TizenPlatform
{
class ResourceLoader;

/**
 * Concrete implementation of the platform abstraction class.
 */
class TizenPlatformAbstraction : public Integration::PlatformAbstraction
{
public: // Construction & Destruction
  /**
   * Constructor
   */
  TizenPlatformAbstraction();

  /**
   * Destructor
   */
  ~TizenPlatformAbstraction() override;

public: // PlatformAbstraction overrides
  /**
   * @copydoc PlatformAbstraction::GetClosestImageSize()
   */
  ImageDimensions GetClosestImageSize(const std::string& filename,
                                      ImageDimensions    size,
                                      FittingMode::Type  fittingMode,
                                      SamplingMode::Type samplingMode,
                                      bool               orientationCorrection) override;

  /**
   * @copydoc PlatformAbstraction::GetClosestImageSize()
   */
  ImageDimensions GetClosestImageSize(Integration::ResourcePointer resourceBuffer,
                                      ImageDimensions              size,
                                      FittingMode::Type            fittingMode,
                                      SamplingMode::Type           samplingMode,
                                      bool                         orientationCorrection) override;

  /**
   * @copydoc PlatformAbstraction::LoadImageSynchronously()
   */
  Integration::ResourcePointer LoadImageSynchronously(const Integration::BitmapResourceType& resource, const std::string& resourcePath) override;

  /**
   * @copydoc PlatformAbstraction::DecodeBuffer()
   */
  Integration::BitmapPtr DecodeBuffer(const Integration::BitmapResourceType& resource, uint8_t* buffer, size_t size) override;

  /**
   * @copydoc PlatformAbstraction::LoadShaderBinaryFile()
   */
  bool LoadShaderBinaryFile(const std::string& filename, Dali::Vector<unsigned char>& buffer) const override;

  /**
   * @copydoc PlatformAbstraction::SaveShaderBinaryFile()
   */
  bool SaveShaderBinaryFile(const std::string& filename, const unsigned char* buffer, unsigned int numBytes) const override;

  /**
   * @copydoc PlatformAbstraction::StartTimer()
   */
  uint32_t StartTimer(uint32_t milliseconds, CallbackBase* callback) override;

  /**
   * @copydoc PlatformAbstraction::CancelTimer()
   */
  void CancelTimer(uint32_t timerId) override;

  /**
   * Sets path for data/resource storage.
   * @param[in] path data/resource storage path
   */
  void SetDataStoragePath(const std::string& path);

  /**
   * Clears the timers that have completed
   */
  void CleanupTimers();

private:
  struct TimerCallback;

  /*
   * Executes callback function and cleans up timer
   */
  void RunTimerFunction(TimerCallback& timerPtr);

  TizenPlatformAbstraction(const TizenPlatformAbstraction&);            ///< Undefined
  TizenPlatformAbstraction& operator=(const TizenPlatformAbstraction&); ///< Undefined

  std::string mDataStoragePath;

  std::vector<std::unique_ptr<TimerCallback> > mTimerPairsWaiting;
  std::vector<std::unique_ptr<TimerCallback> > mTimerPairsSpent;
};

/**
 * Construct a platform abstraction and return it.
 * @return TizenPlatformAbstraction instance
 */
TizenPlatformAbstraction* CreatePlatformAbstraction();

/**
 * Save a file to disk
 * @param filename to create
 * @param buffer to store
 * @param numBytes to store
 * @return true if successful, false otherwise
 */
bool SaveFile(const std::string& filename, const unsigned char* buffer, unsigned int numBytes);

} // namespace TizenPlatform

} // namespace Dali

#endif // DALI_TIZEN_PLATFORM_ABSTRACTION_H
