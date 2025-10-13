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

// CLASS HEADER
#include <dali/internal/window-system/common/window-render-surface.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>
#include <unordered_set>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/adaptor-framework/thread-synchronization-interface.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/system-factory.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-system.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const int   MINIMUM_DIMENSION_CHANGE(1); ///< Minimum change for window to be considered to have moved
const float FULL_UPDATE_RATIO(0.8f);     ///< Force full update when the dirty area is larget than this ratio

constexpr int MERGE_RECTS_LOGIC_THRESHOLD = 50; ///< Threshold of the number of dirty rects to switch between the legacy O(n^2) rectangle merging logic and O(n log n) interval-based approach.

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_WINDOW_RENDER_SURFACE");
#endif

void InsertRects(WindowRenderSurface::DamagedRectsContainer& damagedRectsList, const Rect<int>& damagedRects)
{
  damagedRectsList.insert(damagedRectsList.begin(), damagedRects);
  if(damagedRectsList.size() > 4) // past triple buffers + current
  {
    damagedRectsList.pop_back();
  }
}

Rect<int32_t> RecalculateRect0(Rect<int32_t>& rect, const Rect<int32_t>& surfaceSize)
{
  return rect;
}

Rect<int32_t> RecalculateRect90(Rect<int32_t>& rect, const Rect<int32_t>& surfaceSize)
{
  Rect<int32_t> newRect;
  newRect.x      = surfaceSize.height - (rect.y + rect.height);
  newRect.y      = rect.x;
  newRect.width  = rect.height;
  newRect.height = rect.width;
  return newRect;
}

Rect<int32_t> RecalculateRect180(Rect<int32_t>& rect, const Rect<int32_t>& surfaceSize)
{
  Rect<int32_t> newRect;
  newRect.x      = surfaceSize.width - (rect.x + rect.width);
  newRect.y      = surfaceSize.height - (rect.y + rect.height);
  newRect.width  = rect.width;
  newRect.height = rect.height;
  return newRect;
}

Rect<int32_t> RecalculateRect270(Rect<int32_t>& rect, const Rect<int32_t>& surfaceSize)
{
  Rect<int32_t> newRect;
  newRect.x      = rect.y;
  newRect.y      = surfaceSize.width - (rect.x + rect.width);
  newRect.width  = rect.height;
  newRect.height = rect.width;
  return newRect;
}

using RecalculateRectFunction = Rect<int32_t> (*)(Rect<int32_t>&, const Rect<int32_t>&);

RecalculateRectFunction RecalculateRect[4] = {RecalculateRect0, RecalculateRect90, RecalculateRect180, RecalculateRect270};

namespace DamagedRectUtils
{
/**
 * @brief IntervalMarker enum to mark the beginning and end of a interval.
 */
enum IntervalMarker
{
  INTERVAL_BEGIN = 0, ///< Marker for the beginning of a interval
  INTERVAL_END   = 1, ///< Marker for the end of a interval

  INTERVAL_MARKER_MASK = 0x1 /// Mask to extract the interval marker from the position value
};

static_assert(IntervalMarker::INTERVAL_BEGIN < IntervalMarker::INTERVAL_END, "INTERVAL_BEGIN marker must be less than INTERVAL_END");

/**
 * @brief Helper function to get the interval is begin or end.
 */
constexpr IntervalMarker GetIntervalMarker(int positionInfo)
{
  return static_cast<IntervalMarker>(positionInfo & INTERVAL_MARKER_MASK);
}

/**
 * @brief Helper function to set the interval as begin or end.
 * Ensures interval endpoints are even for pixel alignment and to avoid rounding bias in repeated merges.
 */
constexpr int ClampAndEven(int x, int min, int max, bool ceil)
{
  Dali::ClampInPlace<int>(x, min, max);
  if(x & 1)
  {
    x += ceil ? 1 : -1;
  }
  return x;
}

constexpr int MarkAsBegin(int positionInfo, int min, int max)
{
  return ClampAndEven(positionInfo, min, max, false) | INTERVAL_BEGIN;
}

constexpr int MarkAsEnd(int positionInfo, int min, int max)
{
  return ClampAndEven(positionInfo, min, max, true) | INTERVAL_END;
}

/**
 * @brief Retrieve the marked intervals from the position infos which are not overlapping.
 * The position infos should be sorted by IntervalMarker.
 *
 * @param[in] positionInfos The vector of position infos with IntervalMarker.
 * @param[out] intervals The output vector for intervals.
 */
void RetrieveMarkedInterval(const std::vector<int>& positionInfos, std::vector<int>& intervals)
{
  int xStart = 0;
  int xEnd   = 0;
  int xCount = 0;
  for(const auto& positionInfo : positionInfos)
  {
    if(GetIntervalMarker(positionInfo) == IntervalMarker::INTERVAL_BEGIN)
    {
      if(++xCount == 1)
      {
        xStart = positionInfo;
      }
    }
    else
    {
      if(--xCount == 0)
      {
        xEnd = positionInfo;

        // Check overflow happened
        if(DALI_LIKELY(xStart < xEnd))
        {
          intervals.emplace_back(xStart);
          intervals.emplace_back(xEnd);
        }
        else
        {
          DALI_LOG_ERROR("Integer overflow happend! (begin was INT_MIN, or end was INT_MAX). Just reset intervals and full-swap\n");
          intervals.clear();
        }
      }
    }
  }
}

/**
 * @brief Get the damaged ranges interval from the damaged rectangles.
 * It will be used to get the dirty rects more efficiently when the number of damaged rects is large.
 *
 * The algorithm works as follows:
 * 1. Clamp each intervals end point as even number.
 *    For example, if the interval is [1, 3], it will be clamped to [0, 4]. [-7, -3] will be clamped to [-8, -2].
 * 2. Use last bit of the position to mark the beginning and end of the interval.
 *    After that we can know that even number is beginning of the interval, and odd number is the end of the interval.
 *    For example, if the interval is [0, 4], it will be marked as [0, 5]. [-8, -2] will be marked as [-8, -1].
 * 3. Sort the positions.
 * 4. Now we can get the list of intervals that each others are not overlapping.
 *
 * @param[in] damagedRects The list of damaged rectangles.
 * @param[in] surfaceRect The surface rectangle to which the damaged rectangles belong.
 * @param[out] xIntervals The output vector for x axis intervals.
 * @param[out] yIntervals The output vector for y axis intervals.
 */
void GetDamagedRangesInterval(const std::vector<Rect<int>>& damagedRects, const Rect<int32_t>& surfaceRect, std::vector<int>& xIntervals, std::vector<int>& yIntervals)
{
  static std::vector<int> rectXPositionInfos;
  static std::vector<int> rectYPositionInfos;

  const uint32_t n = damagedRects.size();

  rectXPositionInfos.clear();
  rectYPositionInfos.clear();
  rectXPositionInfos.reserve(n * 2);
  rectYPositionInfos.reserve(n * 2);

  for(uint32_t i = 0; i < n; i++)
  {
    if(DALI_UNLIKELY(damagedRects[i].IsEmpty()))
    {
      continue;
    }

    // Encode each rect position with INTERVAL_BEGIN and INTERVAL_END (to reduce the sort time)
    rectXPositionInfos.emplace_back(MarkAsBegin(damagedRects[i].x, surfaceRect.x, surfaceRect.x + surfaceRect.width));
    rectXPositionInfos.emplace_back(MarkAsEnd(damagedRects[i].x + damagedRects[i].width, surfaceRect.x, surfaceRect.x + surfaceRect.width));

    rectYPositionInfos.emplace_back(MarkAsBegin(damagedRects[i].y, surfaceRect.y, surfaceRect.y + surfaceRect.height));
    rectYPositionInfos.emplace_back(MarkAsEnd(damagedRects[i].y + damagedRects[i].height, surfaceRect.y, surfaceRect.y + surfaceRect.height));
  }

  // Sort the positions with IntervalMarker
  std::sort(rectXPositionInfos.begin(), rectXPositionInfos.end());
  std::sort(rectYPositionInfos.begin(), rectYPositionInfos.end());

  // Now we have the positions with IntervalMarker, We can narrow down the candidates damage rects by interval.
  // Even index is the start of the interval, odd index is the end of the interval. (To reduce the upper_bound compare time)
  RetrieveMarkedInterval(rectXPositionInfos, xIntervals);
  RetrieveMarkedInterval(rectYPositionInfos, yIntervals);
}

/**
 * @brief Merges intersecting rectangles and rotates them for large number of damaged rectscases.
 * Time complexity is O(n log n) for sort, O(n * HashSet) for merging. Space complexity is O(n).
 *
 * The algorithm works as follows:
 * 1. Collect all x and y intervals which could cover all damaged rectangles.
 * 2. Create a new set of damaged rectangles by the intervals. Subset of this new set will contains original damaged rectangles.
 * 3. Collect the new rectangles who are actually contains at least one of the original damaged rectangles.
 *
 * +--+  +---+     +-+ -- yIntervals[0]
 * |  |  |*  |     |*|
 * |  |  |  *|     | |
 * +--+  +---+     +-+ -- yIntervals[1]
 *
 * +--+  +---+     +-+ -- yIntervals[2]
 * | *|  | * |     | |
 * |* |  |   |     | |
 * +--+  +---+     +-+ -- yIntervals[3]
 * |  |  |   |     | |
 * |  xIntervals[1]| xIntervals[5]
 * xIntervals[0]   xIntervals[4]
 *       |   |
 *       |   xIntervals[3]
 *       xIntervals[2]
 *
 * Now the rectangles who actually contains the all input damaged rectangles collected only.
 */
void MergeIntersectingRectsAndRotateForLargeCase(Rect<int>& mergingRect, std::vector<Rect<int>>& damagedRects, int orientation, const Rect<int32_t>& surfaceRect)
{
  /**
   * @brief Helper class to clean the damaged rects as surfaceRect on exit.
   */
  class DamagedRectsCleaner
  {
  public:
    explicit DamagedRectsCleaner(Rect<int>& mergingRect, std::vector<Rect<int>>& damagedRects, const int orientation, const Rect<int>& surfaceRect)
    : mMergingRect(mergingRect),
      mDamagedRects(damagedRects),
      mSurfaceRect(surfaceRect),
      mOrientation(orientation),
      mCleanOnReturn(true)
    {
    }

    void CompleteWithoutError()
    {
      mCleanOnReturn = false;
    }

    ~DamagedRectsCleaner()
    {
      if(mCleanOnReturn)
      {
        mMergingRect = mSurfaceRect;
        mDamagedRects.clear();
        mDamagedRects.push_back(RecalculateRect[mOrientation](mMergingRect, mSurfaceRect));
      }
    }

  private:
    Rect<int>&              mMergingRect;
    std::vector<Rect<int>>& mDamagedRects;
    const Rect<int>         mSurfaceRect;
    const int               mOrientation;
    bool                    mCleanOnReturn;
  };

  DamagedRectsCleaner damagedRectCleaner(mergingRect, damagedRects, orientation, surfaceRect);

  static std::vector<int> xIntervals;
  static std::vector<int> yIntervals;

  const uint32_t n = damagedRects.size();
  xIntervals.clear();
  yIntervals.clear();
  xIntervals.reserve(2 * n);
  yIntervals.reserve(2 * n);

  // Collect intervals for each axis
  GetDamagedRangesInterval(damagedRects, surfaceRect, xIntervals, yIntervals);

  // The number of Intervals should be even, because we have pairs of begin and end intervals.
  // If the number of intervals is zero or odd, it means that there is an error in the input data.
  if(DALI_UNLIKELY(xIntervals.size() == 0 || (xIntervals.size() & 1) || yIntervals.size() == 0 || (yIntervals.size() & 1)))
  {
    // Should never happen
    DALI_LOG_ERROR("No intervals found, something is wrong!!\n");
    return;
  }

  if(DALI_UNLIKELY(xIntervals.size() > 0xFFFF || yIntervals.size() > 0xFFFF))
  {
    // If the number of intervals is too large, just full swap.
    return;
  }

  // Now we have the intervals, we can create the new damaged rects by the intervals
  const uint16_t xIntervalCount = static_cast<uint16_t>(xIntervals.size());
  const uint16_t yIntervalCount = static_cast<uint16_t>(yIntervals.size());

  mergingRect.x      = xIntervals[0];
  mergingRect.y      = yIntervals[0];
  mergingRect.width  = xIntervals[xIntervalCount - 1u] - mergingRect.x;
  mergingRect.height = yIntervals[yIntervalCount - 1u] - mergingRect.y;

  // Special case for 2x2 intervals
  if(xIntervalCount == 2 && yIntervalCount == 2)
  {
    // Only one rect
    damagedRects.clear();
    damagedRects.push_back(RecalculateRect[orientation](mergingRect, surfaceRect));

    damagedRectCleaner.CompleteWithoutError();
    return;
  }

  // Check if we already have the maximum number of damaged rects.
  // Note that we need to guard integer overflow.
  const uint32_t maximumDamagedRectsCount = std::min(static_cast<uint32_t>(n), static_cast<uint32_t>(xIntervalCount / 2) * static_cast<uint32_t>(yIntervalCount / 2));

  uint32_t newDamagedRectsCount = 0;

  std::unordered_set<uint32_t> uniqueRectsIndexPairs;

  for(uint32_t i = 0; i < n && newDamagedRectsCount < maximumDamagedRectsCount; i++)
  {
    if(DALI_UNLIKELY(damagedRects[i].IsEmpty()))
    {
      continue;
    }

    // Found given rect is in the intervals
    // Upper bound will return the end of interval.
    const auto xIntervalIt = std::upper_bound(xIntervals.cbegin(), xIntervals.cend(), damagedRects[i].x);
    const auto yIntervalIt = std::upper_bound(yIntervals.cbegin(), yIntervals.cend(), damagedRects[i].y);

    if(DALI_UNLIKELY(xIntervalIt == xIntervals.cbegin() ||
                     xIntervalIt == xIntervals.cend() ||
                     yIntervalIt == yIntervals.cbegin() ||
                     yIntervalIt == yIntervals.cend() ||
                     GetIntervalMarker(xIntervalIt - xIntervals.cbegin()) != IntervalMarker::INTERVAL_END ||
                     GetIntervalMarker(yIntervalIt - yIntervals.cbegin()) != IntervalMarker::INTERVAL_END))
    {
      // Should never happen
      DALI_LOG_ERROR("No intervals found, something is wrong!!\n");
      return;
    }

    // Packing x and y intervals indices into a single 32-bit integer to use as a key for uniqueness check.
    // Note that the number of xIntervals and yIntervals is limited to 0xFFFF, so we can safely use 32-bit integer.
    uint32_t indexPair = (static_cast<uint32_t>(xIntervalIt - xIntervals.cbegin()) << 16) | (yIntervalIt - yIntervals.cbegin());

    if(uniqueRectsIndexPairs.find(indexPair) == uniqueRectsIndexPairs.cend())
    {
      // Found new rect
      uniqueRectsIndexPairs.insert(indexPair);

      auto& rect = damagedRects[newDamagedRectsCount];

      rect.x      = *(xIntervalIt - 1);
      rect.y      = *(yIntervalIt - 1);
      rect.width  = (*(xIntervalIt)) - rect.x;
      rect.height = (*(yIntervalIt)) - rect.y;

      damagedRects[newDamagedRectsCount++] = RecalculateRect[orientation](rect, surfaceRect);
    }
  }

  if(DALI_LIKELY(newDamagedRectsCount > 0))
  {
    // Everything is merged well.
    damagedRects.resize(newDamagedRectsCount);

    damagedRectCleaner.CompleteWithoutError();
  }
}

/**
 * @brief Merges intersecting rectangles and rotates them for small number of damaged rectscases.
 * Time complexity is O(n^2) for merge. Space complexity is O(1) (no extra space used).
 */
void MergeIntersectingRectsAndRotateForSmallCase(Rect<int>& mergingRect, std::vector<Rect<int>>& damagedRects, int orientation, const Rect<int32_t>& surfaceRect)
{
  uint32_t n = damagedRects.size();
  for(uint32_t i = 0; i < n - 1; i++)
  {
    if(damagedRects[i].IsEmpty())
    {
      continue;
    }

    for(uint32_t j = i + 1; j < n; j++)
    {
      if(damagedRects[j].IsEmpty())
      {
        continue;
      }

      if(damagedRects[i].Intersects(damagedRects[j]))
      {
        damagedRects[i].Merge(damagedRects[j]);
        std::swap(damagedRects[j], damagedRects[n - 1]); // Move the last rect to the current position
        damagedRects.pop_back();                         // Remove the last rect

        --n; // Reduce the number of damaged rects
        --j; // Stay at the same position to check the new rect
      }
    }
  }

  uint32_t j = 0;
  for(uint32_t i = 0; i < n; i++)
  {
    if(!damagedRects[i].IsEmpty())
    {
      // Merge rects before rotate
      if(mergingRect.IsEmpty())
      {
        mergingRect = damagedRects[i];
      }
      else
      {
        mergingRect.Merge(damagedRects[i]);
      }

      damagedRects[j++] = RecalculateRect[orientation](damagedRects[i], surfaceRect);
    }
  }

  if(DALI_LIKELY(j != 0))
  {
    damagedRects.resize(j);
  }
}
} // namespace DamagedRectUtils

/**
 * @brief Reduce the number of damaged rectangles by merging intersecting rectangles and rotating them by orientation.
 * We separate the logic for small and large number of damaged rectangles to optimize performance.
 *
 * @note MERGE_RECTS_LOGIC_THRESHOLD determines when to switch between the legacy O(n^2) rectangle merging logic and the newer, more efficient O(n log n) interval-based approach.
 *
 * For small numbers of rectangles (n <= MERGE_RECTS_LOGIC_THRESHOLD), the legacy approach is simple and incurs minimal overhead.
 * For larger sets, the legacy approach becomes inefficient due to quadratic time complexity, so the interval-based method is used instead.
 */
void MergeIntersectingRectsAndRotate(Rect<int>& mergingRect, std::vector<Rect<int>>& damagedRects, int orientation, const Rect<int32_t>& surfaceRect)
{
  if(damagedRects.size() <= MERGE_RECTS_LOGIC_THRESHOLD)
  {
    DamagedRectUtils::MergeIntersectingRectsAndRotateForSmallCase(mergingRect, damagedRects, orientation, surfaceRect);
  }
  else
  {
    DamagedRectUtils::MergeIntersectingRectsAndRotateForLargeCase(mergingRect, damagedRects, orientation, surfaceRect);
  }
}

} // unnamed namespace

WindowRenderSurface::WindowRenderSurface(Dali::PositionSize positionSize, Any surface, bool isTransparent)
: mDisplayConnection(nullptr),
  mPositionSize(positionSize),
  mWindowBase(),
  mThreadSynchronization(nullptr),
  mRenderNotification(nullptr),
  mPostRenderTrigger(),
  mFrameRenderedTrigger(),
  mGraphics(nullptr),
  mColorDepth(isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24),
  mOutputTransformedSignal(),
  mWindowRotationFinishedSignal(),
  mFrameCallbackInfoContainer(),
  mBufferDamagedRects(),
  mMutex(),
  mWindowRotationAngle(0),
  mScreenRotationAngle(0),
  mDpiHorizontal(0),
  mDpiVertical(0),
  mOwnSurface(false),
  mIsImeWindowSurface(false),
  mNeedWindowRotationAcknowledgement(false),
  mIsWindowOrientationChanging(false),
  mIsFrontBufferRendering(false),
  mIsFrontBufferRenderingChanged(false)
{
  DALI_LOG_INFO(gWindowRenderSurfaceLogFilter, Debug::Verbose, "Creating Window\n");
  Initialize(surface);
}

WindowRenderSurface::~WindowRenderSurface()
{
}

void WindowRenderSurface::Initialize(Any surface)
{
  // If width or height are zero, go full screen.
  if((mPositionSize.width == 0) || (mPositionSize.height == 0))
  {
    // Default window size == screen size
    mPositionSize.x = 0;
    mPositionSize.y = 0;
    WindowSystem::GetScreenSize(mPositionSize.width, mPositionSize.height);
  }

  // Create a window base
  auto windowFactory = Dali::Internal::Adaptor::GetWindowFactory();
  mWindowBase        = windowFactory->CreateWindowBase(mPositionSize, surface, (mColorDepth == COLOR_DEPTH_32 ? true : false));

  // Connect signals
  mWindowBase->OutputTransformedSignal().Connect(this, &WindowRenderSurface::OutputTransformed);

  // Create frame rendered trigger.
  mFrameRenderedTrigger = std::move(TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &WindowRenderSurface::ProcessFrameCallback),
                                                                            TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
  DALI_LOG_DEBUG_INFO("mFrameRenderedTrigger Trigger Id(%u)\n", mFrameRenderedTrigger->GetId());

  // Check screen rotation
  int screenRotationAngle = mWindowBase->GetScreenRotationAngle(true);
  if(screenRotationAngle != 0)
  {
    DALI_LOG_RELEASE_INFO("WindowRenderSurface::Initialize, screen rotation is enabled, screen rotation angle:[%d]\n", screenRotationAngle);
    OutputTransformed(screenRotationAngle);
  }
}

Any WindowRenderSurface::GetNativeWindow()
{
  return mWindowBase->GetNativeWindow();
}

int WindowRenderSurface::GetNativeWindowId()
{
  return mWindowBase->GetNativeWindowId();
}

void WindowRenderSurface::Map()
{
  mWindowBase->Show();
}

void WindowRenderSurface::SetRenderNotification(TriggerEventInterface* renderNotification)
{
  mRenderNotification = renderNotification;
}

void WindowRenderSurface::SetTransparency(bool transparent)
{
  mWindowBase->SetTransparency(transparent);
}

void WindowRenderSurface::RequestRotation(int angle, PositionSize positionSize)
{
  if(!mPostRenderTrigger)
  {
    mPostRenderTrigger = std::move(TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &WindowRenderSurface::ProcessPostRender),
                                                                           TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
    DALI_LOG_DEBUG_INFO("mPostRenderTrigger Trigger Id(%u)\n", mPostRenderTrigger->GetId());
  }

  mPositionSize.x = positionSize.x;
  mPositionSize.y = positionSize.y;

  mWindowBase->SetWindowRotationAngle(angle);

  DALI_LOG_RELEASE_INFO("start window rotation angle = %d screen rotation = %d\n", angle, mWindowBase->GetScreenRotationAngle(false));
}

WindowBase* WindowRenderSurface::GetWindowBase()
{
  return mWindowBase.get();
}

WindowBase::OutputSignalType& WindowRenderSurface::OutputTransformedSignal()
{
  return mOutputTransformedSignal;
}

WindowRenderSurface::RotationFinishedSignalType& WindowRenderSurface::RotationFinishedSignal()
{
  return mWindowRotationFinishedSignal;
}

PositionSize WindowRenderSurface::GetPositionSize() const
{
  return mPositionSize;
}

void WindowRenderSurface::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  if(mDpiHorizontal == 0 || mDpiVertical == 0)
  {
    const char* environmentDpiHorizontal = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_DPI_HORIZONTAL);
    mDpiHorizontal                       = environmentDpiHorizontal ? std::atoi(environmentDpiHorizontal) : 0;

    const char* environmentDpiVertical = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_DPI_VERTICAL);
    mDpiVertical                       = environmentDpiVertical ? std::atoi(environmentDpiVertical) : 0;

    if(mDpiHorizontal == 0 || mDpiVertical == 0)
    {
      mWindowBase->GetDpi(mDpiHorizontal, mDpiVertical);
    }
  }

  dpiHorizontal = mDpiHorizontal;
  dpiVertical   = mDpiVertical;
}

int WindowRenderSurface::GetSurfaceOrientation() const
{
  return mWindowBase->GetWindowRotationAngle();
}

int WindowRenderSurface::GetScreenOrientation() const
{
  return mWindowBase->GetScreenRotationAngle(false);
}

void WindowRenderSurface::InitializeGraphics()
{
  if(mSurfaceId == Graphics::INVALID_SURFACE_ID)
  {
    mGraphics = &mAdaptor->GetGraphicsInterface();
    DALI_ASSERT_ALWAYS(mGraphics && "Graphics interface is not created");

    CreateSurface();
  }
}

void WindowRenderSurface::CreateSurface()
{
  DALI_LOG_TRACE_METHOD(gWindowRenderSurfaceLogFilter);

  int width, height;
  if(mScreenRotationAngle == 0 || mScreenRotationAngle == 180)
  {
    width  = mPositionSize.width;
    height = mPositionSize.height;
  }
  else
  {
    width  = mPositionSize.height;
    height = mPositionSize.width;
  }

  std::unique_ptr<Graphics::SurfaceFactory> surfaceFactory = Graphics::SurfaceFactory::New(*this);

  mSurfaceId = mGraphics->CreateSurface(surfaceFactory.get(), mWindowBase.get(), mColorDepth, width, height);

  if(mWindowBase->GetType() == WindowType::IME)
  {
    InitializeImeSurface();
  }

  DALI_LOG_RELEASE_INFO("WindowRenderSurface::CreateSurface: SurfaceId(%d) WinId (%d), w = %d h = %d angle = %d screen rotation = %d\n",
                        mSurfaceId,
                        mWindowBase->GetNativeWindowId(),
                        mPositionSize.width,
                        mPositionSize.height,
                        mWindowRotationAngle,
                        mScreenRotationAngle);
}

void WindowRenderSurface::DestroySurface()
{
  DALI_LOG_TRACE_METHOD(gWindowRenderSurfaceLogFilter);

  if(DALI_LIKELY(mGraphics))
  {
    DALI_LOG_RELEASE_INFO("WindowRenderSurface::DestroySurface: SurfaceId(%d) WinId (%d)\n",
                          mSurfaceId,
                          mWindowBase->GetNativeWindowId());
    mGraphics->DestroySurface(mSurfaceId);
    mSurfaceId = Graphics::INVALID_SURFACE_ID;
  }
}

bool WindowRenderSurface::ReplaceGraphicsSurface()
{
  DALI_LOG_TRACE_METHOD(gWindowRenderSurfaceLogFilter);

  int width, height;
  if(mScreenRotationAngle == 0 || mScreenRotationAngle == 180)
  {
    width  = mPositionSize.width;
    height = mPositionSize.height;
  }
  else
  {
    width  = mPositionSize.height;
    height = mPositionSize.width;
  }

  if(DALI_LIKELY(mGraphics))
  {
    DALI_LOG_RELEASE_INFO("WindowRenderSurface::ReplaceGraphicsSurface: SurfaceId(%d) WinId (%d), width(%d) height(%d)\n",
                          mSurfaceId,
                          mWindowBase->GetNativeWindowId(),
                          width,
                          height);
    return mGraphics->ReplaceSurface(mSurfaceId, width, height);
  }
  else
  {
    DALI_LOG_ERROR("Graphics interface is not initialized yet.");
    return false;
  }
}

void WindowRenderSurface::UpdatePositionSize(Dali::PositionSize positionSize)
{
  // Check moving
  if((fabs(positionSize.x - mPositionSize.x) >= MINIMUM_DIMENSION_CHANGE) ||
     (fabs(positionSize.y - mPositionSize.y) >= MINIMUM_DIMENSION_CHANGE))
  {
    mPositionSize.x = positionSize.x;
    mPositionSize.y = positionSize.y;

    DALI_LOG_RELEASE_INFO("Update Position by server SurfaceId(%d) (%d, %d)\n", mSurfaceId, mPositionSize.x, mPositionSize.y);
  }
}

void WindowRenderSurface::Move(Dali::PositionSize positionSize)
{
  mPositionSize.x = positionSize.x;
  mPositionSize.y = positionSize.y;

  DALI_LOG_RELEASE_INFO("Update Position by client SurfaceId(%d) (%d, %d)\n", mSurfaceId, positionSize.x, positionSize.y);

  mWindowBase->Move(positionSize);
}

void WindowRenderSurface::MoveResize(Dali::PositionSize positionSize)
{
  mPositionSize.x = positionSize.x;
  mPositionSize.y = positionSize.y;

  DALI_LOG_RELEASE_INFO("Update Position by client SurfaceId(%d) (%d, %d)\n", mSurfaceId, positionSize.x, positionSize.y);

  mWindowBase->MoveResize(positionSize);
}

void WindowRenderSurface::Resize(Uint16Pair size)
{
  Dali::PositionSize positionSize;

  // Some native resize API (e.g. wl_egl_window_resize) have the input parameters of x, y, width and height.
  // So, position data should be set as well.
  positionSize.x      = mPositionSize.x;
  positionSize.y      = mPositionSize.y;
  positionSize.width  = size.GetWidth();
  positionSize.height = size.GetHeight();

  mWindowBase->ResizeWindow(positionSize);
}

void WindowRenderSurface::StartRender()
{
}

bool WindowRenderSurface::PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect)
{
  InitializeGraphics();

  Dali::Integration::Scene::FrameCallbackContainer callbacks;

  Dali::Integration::Scene scene = mScene.GetHandle();

  if(scene)
  {
    bool needFrameRenderedTrigger = false;

    //@todo These callbacks are specifically for wayland EGL. Are there equivalent wayland vulkan callbacks?

    scene.GetFrameRenderedCallback(callbacks);
    if(!callbacks.empty())
    {
      int frameRenderedSync = mWindowBase->CreateFrameRenderedSyncFence();
      if(frameRenderedSync != -1)
      {
        Dali::Mutex::ScopedLock lock(mMutex);

        DALI_LOG_RELEASE_INFO("WindowRenderSurface::PreRender: CreateFrameRenderedSyncFence [%d]\n", frameRenderedSync);

        mFrameCallbackInfoContainer.push_back(std::unique_ptr<FrameCallbackInfo>(new FrameCallbackInfo(callbacks, frameRenderedSync)));

        needFrameRenderedTrigger = true;
      }
      else
      {
        DALI_LOG_ERROR("WindowRenderSurface::PreRender: CreateFrameRenderedSyncFence is failed\n");
      }

      // Clear callbacks
      callbacks.clear();
    }

    scene.GetFramePresentedCallback(callbacks);
    if(!callbacks.empty())
    {
      int framePresentedSync = mWindowBase->CreateFramePresentedSyncFence();
      if(framePresentedSync != -1)
      {
        Dali::Mutex::ScopedLock lock(mMutex);

        DALI_LOG_RELEASE_INFO("WindowRenderSurface::PreRender: CreateFramePresentedSyncFence [%d]\n", framePresentedSync);

        mFrameCallbackInfoContainer.push_back(std::unique_ptr<FrameCallbackInfo>(new FrameCallbackInfo(callbacks, framePresentedSync)));

        needFrameRenderedTrigger = true;
      }
      else
      {
        DALI_LOG_ERROR("WindowRenderSurface::PreRender: CreateFramePresentedSyncFence is failed\n");
      }

      // Clear callbacks
      callbacks.clear();
    }

    if(needFrameRenderedTrigger)
    {
      mFrameRenderedTrigger->Trigger();
    }
  }

  /*
   * wl_egl_window_tizen_set_rotation(SetWindowRotation)                -> PreRotation
   * wl_egl_window_tizen_set_buffer_transform(SetWindowBufferTransform) -> Screen Rotation
   * wl_egl_window_tizen_set_window_transform(SetWindowTransform)       -> Window Rotation
   * These function should be called before calling first drawing gl Function.
   * Notice : PreRotation is not used in the latest tizen,
   *          because output transform event should be occured before egl window is not created.
   */
  if(scene && resizingSurface)
  {
    int  totalAngle                  = 0;
    bool isScreenOrientationChanging = false;

    if(mWindowRotationAngle != scene.GetCurrentSurfaceOrientation())
    {
      mWindowRotationAngle         = scene.GetCurrentSurfaceOrientation();
      mIsWindowOrientationChanging = true;
    }

    if(mScreenRotationAngle != scene.GetCurrentScreenOrientation())
    {
      mScreenRotationAngle        = scene.GetCurrentScreenOrientation();
      isScreenOrientationChanging = true;
    }
    totalAngle = (mWindowRotationAngle + mScreenRotationAngle) % 360;

    DALI_LOG_RELEASE_INFO("Window/Screen orientation are changed, WinOrientation[%d],flag[%d], ScreenOrientation[%d],flag[%d], total[%d]\n", mWindowRotationAngle, mIsWindowOrientationChanging, mScreenRotationAngle, isScreenOrientationChanging, totalAngle);

    Rect<int> surfaceSize = scene.GetCurrentSurfaceRect();
    // update surface size
    mPositionSize.width  = surfaceSize.width;
    mPositionSize.height = surfaceSize.height;

    DALI_LOG_RELEASE_INFO("Window is resizing, SurfaceId(%d) (%d, %d), [%d x %d], IMEWindow [%d]\n", mSurfaceId, mPositionSize.x, mPositionSize.y, mPositionSize.width, mPositionSize.height, mIsImeWindowSurface);

    // Window rotate or screen rotate
    if(mIsWindowOrientationChanging || isScreenOrientationChanging)
    {
      mWindowBase->SetWindowBufferTransform(totalAngle);
    }

    // Only window rotate
    if(mIsWindowOrientationChanging)
    {
      mWindowBase->SetWindowTransform(mWindowRotationAngle);
    }

    // Resize case
    Uint16Pair size;

    if(totalAngle == 0 || totalAngle == 180)
    {
      size.SetWidth(mPositionSize.width);
      size.SetHeight(mPositionSize.height);
    }
    else
    {
      size.SetWidth(mPositionSize.height);
      size.SetHeight(mPositionSize.width);
    }

    mGraphics->Resize(this, size);

    SetFullSwapNextFrame();
  }

  // When mIsFrontBufferRendering is not equal to mWindowBase's
  if(mIsFrontBufferRenderingChanged)
  {
    mIsFrontBufferRenderingChanged = false;
    mWindowBase->SetWindowFrontBufferMode(mIsFrontBufferRendering);
    SetFullSwapNextFrame();
  }

  SetBufferDamagedRects(damagedRects, clippingRect);

  if(scene)
  {
    Rect<int> surfaceRect = scene.GetCurrentSurfaceRect();
    if(clippingRect == surfaceRect)
    {
      int32_t totalAngle = scene.GetCurrentSurfaceOrientation() + scene.GetCurrentScreenOrientation();
      if(totalAngle >= 360)
      {
        totalAngle -= 360;
      }
      mDamagedRects.assign(1, RecalculateRect[std::min(totalAngle / 90, 3)](surfaceRect, surfaceRect));
    }
  }

  return true;
}

void WindowRenderSurface::PostRender()
{
  // Inform the graphics backend that rendering has finished before informing the surface
  mGraphics->PostRenderDebug();

  bool needWindowRotationCompleted = false;

  if(mIsWindowOrientationChanging)
  {
    if(mNeedWindowRotationAcknowledgement)
    {
      Dali::Integration::Scene scene = mScene.GetHandle();
      if(scene)
      {
        if(scene.IsRotationCompletedAcknowledgementSet())
        {
          needWindowRotationCompleted = true;
        }
      }
    }
    else
    {
      needWindowRotationCompleted = true;
    }
  }

  if(needWindowRotationCompleted || mIsImeWindowSurface)
  {
    if(mThreadSynchronization)
    {
      // Enable PostRender flag
      mThreadSynchronization->PostRenderStarted();
    }

    if(mIsWindowOrientationChanging || mIsImeWindowSurface)
    {
      mPostRenderTrigger->Trigger();
    }

    if(mThreadSynchronization)
    {
      // Wait until the event-thread complete the rotation event processing
      mThreadSynchronization->PostRenderWaitForCompletion();
    }
  }

  SwapBuffers(mDamagedRects);

  if(mRenderNotification)
  {
    mRenderNotification->Trigger();
  }
}

void WindowRenderSurface::StopRender()
{
}

void WindowRenderSurface::SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization)
{
  DALI_LOG_INFO(gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::SetThreadSynchronization: called\n");

  mThreadSynchronization = &threadSynchronization;
}

void WindowRenderSurface::ReleaseLock()
{
  // Nothing to do.
}

Dali::Integration::RenderSurfaceInterface::Type WindowRenderSurface::GetSurfaceType()
{
  return Dali::Integration::RenderSurfaceInterface::WINDOW_RENDER_SURFACE;
}

void WindowRenderSurface::MakeContextCurrent()
{
  mGraphics->MakeContextCurrent(mSurfaceId);
}

Integration::DepthBufferAvailable WindowRenderSurface::GetDepthBufferRequired()
{
  return mGraphics ? mGraphics->GetDepthBufferRequired() : Integration::DepthBufferAvailable::FALSE;
}

Integration::StencilBufferAvailable WindowRenderSurface::GetStencilBufferRequired()
{
  return mGraphics ? mGraphics->GetStencilBufferRequired() : Integration::StencilBufferAvailable::FALSE;
}

void WindowRenderSurface::InitializeImeSurface()
{
  if(!mIsImeWindowSurface)
  {
    mIsImeWindowSurface = true;
    if(!mPostRenderTrigger)
    {
      mPostRenderTrigger = std::move(TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &WindowRenderSurface::ProcessPostRender),
                                                                             TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
      DALI_LOG_DEBUG_INFO("mPostRenderTrigger Trigger Id(%u)\n", mPostRenderTrigger->GetId());
    }
  }
}

void WindowRenderSurface::SetNeedsRotationCompletedAcknowledgement(bool needAcknowledgement)
{
  mNeedWindowRotationAcknowledgement = needAcknowledgement;
}

void WindowRenderSurface::OutputTransformed(int screenRotationAngle)
{
  DALI_LOG_RELEASE_INFO("Emit screen rotation signal to new screen angle = %d\n", screenRotationAngle);
  mOutputTransformedSignal.Emit(screenRotationAngle);
}

void WindowRenderSurface::ProcessPostRender()
{
  if(mIsWindowOrientationChanging)
  {
    mWindowRotationFinishedSignal.Emit();
    mWindowBase->WindowRotationCompleted(mWindowRotationAngle, mPositionSize.width, mPositionSize.height);
    mIsWindowOrientationChanging = false;
    DALI_LOG_RELEASE_INFO("WindowRenderSurface::ProcessPostRender: Rotation Done, flag = %d\n", mIsWindowOrientationChanging);
  }

  if(mIsImeWindowSurface)
  {
    mWindowBase->ImeWindowReadyToRender();
  }

  if(mThreadSynchronization)
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

void WindowRenderSurface::ProcessFrameCallback()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  for(auto&& iter : mFrameCallbackInfoContainer)
  {
    if(!iter->fileDescriptorMonitor)
    {
      iter->fileDescriptorMonitor = Dali::Internal::Adaptor::GetSystemFactory()->CreateFileDescriptorMonitor(iter->fileDescriptor, MakeCallback(this, &WindowRenderSurface::OnFileDescriptorEventDispatched), FileDescriptorMonitor::FD_READABLE);

      DALI_LOG_RELEASE_INFO("WindowRenderSurface::ProcessFrameCallback: Add handler [%d]\n", iter->fileDescriptor);
    }
  }
}

void WindowRenderSurface::OnFileDescriptorEventDispatched(FileDescriptorMonitor::EventType eventBitMask, int fileDescriptor)
{
  DALI_LOG_RELEASE_INFO("WindowRenderSurface::OnFileDescriptorEventDispatched: Frame rendered [%d]\n", fileDescriptor);

  std::unique_ptr<FrameCallbackInfo> callbackInfo;
  {
    Dali::Mutex::ScopedLock lock(mMutex);

    auto frameCallbackInfo = std::find_if(mFrameCallbackInfoContainer.begin(), mFrameCallbackInfoContainer.end(), [fileDescriptor](std::unique_ptr<FrameCallbackInfo>& callbackInfo)
    { return callbackInfo->fileDescriptor == fileDescriptor; });
    if(frameCallbackInfo != mFrameCallbackInfoContainer.end())
    {
      callbackInfo = std::move(*frameCallbackInfo);

      mFrameCallbackInfoContainer.erase(frameCallbackInfo);
    }
  }

  // Call the connected callback
  if(callbackInfo && (eventBitMask & FileDescriptorMonitor::FD_READABLE))
  {
    for(auto&& iter : (callbackInfo)->callbacks)
    {
      CallbackBase::Execute(*(iter.first), iter.second);
    }
  }
}

void WindowRenderSurface::SetBufferDamagedRects(const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect)
{
  // If scene is not exist, just use stored mPositionSize.
  Rect<int> surfaceRect(0, 0, mPositionSize.width, mPositionSize.height);
  int32_t   orientation = 0;

  Dali::Integration::Scene scene = mScene.GetHandle();
  if(scene)
  {
    surfaceRect        = scene.GetCurrentSurfaceRect();
    int32_t totalAngle = scene.GetCurrentSurfaceOrientation() + scene.GetCurrentScreenOrientation();
    if(totalAngle >= 360)
    {
      totalAngle -= 360;
    }
    orientation = std::min(totalAngle / 90, 3);
  }

  if(Integration::PartialUpdateAvailable::FALSE == mGraphics->GetPartialUpdateRequired() ||
     mFullSwapNextFrame)
  {
    InsertRects(mBufferDamagedRects, surfaceRect);
    clippingRect = surfaceRect;
    return;
  }

  if(damagedRects.empty())
  {
    // Empty damaged rect. We don't need rendering
    clippingRect = Rect<int>();
    // Clean up current damanged rects.
    mDamagedRects.clear();
    return;
  }

  mGraphics->ActivateSurfaceContext(this);

  auto bufferAge = mGraphics->GetBufferAge(mSurfaceId);

  // Buffer age 0 means the back buffer in invalid and requires full swap
  if(bufferAge == 0)
  {
    InsertRects(mBufferDamagedRects, surfaceRect);
    clippingRect = surfaceRect;
    return;
  }

  mDamagedRects.assign(damagedRects.begin(), damagedRects.end());

  // Merge intersecting rects, form an array of non intersecting rects to help driver a bit
  // Could be optional and can be removed, needs to be checked with and without on platform
  // And then, Make one clipping rect, and rotate rects by orientation.
  MergeIntersectingRectsAndRotate(clippingRect, mDamagedRects, orientation, surfaceRect);

  // We push current frame damaged rects here, zero index for current frame
  InsertRects(mBufferDamagedRects, clippingRect);

  // Merge damaged rects into clipping rect
  if(bufferAge <= static_cast<int>(mBufferDamagedRects.size()))
  {
    // clippingRect is already the current frame's damaged rect. Merge from the second
    for(int i = 1; i < bufferAge; i++)
    {
      clippingRect.Merge(mBufferDamagedRects[i]);
    }
  }
  else
  {
    // The buffer age is too old. Need full update.
    clippingRect = surfaceRect;
    return;
  }

  if(!clippingRect.Intersect(surfaceRect) || clippingRect.Area() > surfaceRect.Area() * FULL_UPDATE_RATIO)
  {
    // clipping area too big or doesn't intersect surface rect
    clippingRect = surfaceRect;
    return;
  }

  if(!clippingRect.IsEmpty())
  {
    std::vector<Rect<int>> damagedRegion;
    if(scene)
    {
      damagedRegion.push_back(RecalculateRect[orientation](clippingRect, surfaceRect));
    }
    else
    {
      damagedRegion.push_back(clippingRect);
    }

    mGraphics->SetDamageRegion(mSurfaceId, damagedRegion);
  }
}

void WindowRenderSurface::SwapBuffers(const std::vector<Rect<int>>& damagedRects)
{
  if(Integration::PartialUpdateAvailable::FALSE == mGraphics->GetPartialUpdateRequired() ||
     mFullSwapNextFrame)
  {
    mFullSwapNextFrame = false;
    mGraphics->SwapBuffers(mSurfaceId);
    return;
  }

  mFullSwapNextFrame = false;

  Rect<int32_t>            surfaceRect;
  Dali::Integration::Scene scene = mScene.GetHandle();
  if(scene)
  {
    surfaceRect = scene.GetCurrentSurfaceRect();
  }

  if(!damagedRects.size() || (damagedRects[0].Area() > surfaceRect.Area() * FULL_UPDATE_RATIO))
  {
    // In normal cases, WindowRenderSurface::SwapBuffers() will not be called if mergedRects.size() is 0.
    // For exceptional cases, swap full area.
    mGraphics->SwapBuffers(mSurfaceId);
  }
  else
  {
    mGraphics->SwapBuffers(mSurfaceId, damagedRects);
  }
}

void WindowRenderSurface::SetFrontBufferRendering(bool enable)
{
  if(mIsFrontBufferRendering != enable)
  {
    mIsFrontBufferRendering        = enable;
    mIsFrontBufferRenderingChanged = !mIsFrontBufferRenderingChanged;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
