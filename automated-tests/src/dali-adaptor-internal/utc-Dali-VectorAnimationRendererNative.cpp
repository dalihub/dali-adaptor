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

#include <dali-test-suite-utils.h>
#include <dali/internal/vector-animation/common/vector-animation-renderer-native.h>
#include <dali/public-api/object/property-array.h>
#include <thorvg.h>
#include <fstream>
#include <string>

using namespace Dali;
using Dali::Internal::Adaptor::VectorAnimationRendererNative;

namespace
{
// fr 30, ip 0, op 90
// markers: idle {tm 0, dr 30}, activate:start {tm 30, dr 30}, activate:end {tm 60, dr 30}
// layers : idle_layer [0, 30], activate_start [30, 60], activate_end [60, 90]
const char* const LOTTIE_MARKERS_FILE = TEST_RESOURCE_DIR "/lottie-markers.json";

// 96x96 RGBA PNG: what a remote "lottie" URL actually returned in the field.
const char* const NOT_A_LOTTIE_FILE = TEST_RESOURCE_DIR "/not-a-lottie.png";

/**
 * Minimal concrete renderer for exercising the ThorVG-backed metadata paths.
 * It intentionally does not call Initialize(), so neither an Adaptor nor the
 * VectorAnimationRendererEventManager is required.
 */
class TestVectorAnimationRenderer : public VectorAnimationRendererNative
{
public:
  TestVectorAnimationRenderer()           = default;
  ~TestVectorAnimationRenderer() override = default;

  bool Render(uint32_t frameNumber) override
  {
    return false;
  }

protected:
  std::shared_ptr<RenderingData> CreateRenderingData() override
  {
    return std::make_shared<RenderingData>();
  }

  void PrepareTarget(std::shared_ptr<RenderingData> renderingData) override
  {
  }

  bool IsTargetPrepared() override
  {
    return false;
  }

  Dali::Texture GetTargetTexture() override
  {
    return Dali::Texture();
  }

  void OnFinalize() override
  {
  }

  void OnNotify() override
  {
  }
};

bool ReadFile(const char* path, Dali::Vector<uint8_t>& data)
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if(!file.is_open())
  {
    return false;
  }

  const std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  data.Resize(static_cast<Dali::Vector<uint8_t>::SizeType>(size));
  return static_cast<bool>(file.read(reinterpret_cast<char*>(data.Begin()), size));
}

void CheckMarker(const VectorAnimationRendererNative& renderer, const char* name, uint32_t expectedStart, uint32_t expectedEnd, const char* location)
{
  uint32_t startFrame = 0xFFFFFFFFu;
  uint32_t endFrame   = 0xFFFFFFFFu;

  DALI_TEST_EQUALS(renderer.GetMarkerInfo(name, startFrame, endFrame), true, location);
  DALI_TEST_EQUALS(startFrame, expectedStart, location);
  DALI_TEST_EQUALS(endFrame, expectedEnd, location);
}

void CheckRange(const Property::Map& map, const char* name, int expectedStart, int expectedEnd, const char* location)
{
  const Property::Value* value = map.Find(name);
  DALI_TEST_CHECK(value != nullptr);

  const Property::Array* range = value->GetArray();
  DALI_TEST_CHECK(range != nullptr);
  DALI_TEST_CHECK(range->Count() == 2u);

  DALI_TEST_EQUALS(range->GetElementAt(0).Get<int>(), expectedStart, location);
  DALI_TEST_EQUALS(range->GetElementAt(1).Get<int>(), expectedEnd, location);
}

void CheckMarkersOfLottieMarkersFile(const VectorAnimationRendererNative& renderer)
{
  DALI_TEST_EQUALS(renderer.GetTotalFrameNumber(), 90u, TEST_LOCATION);
  DALI_TEST_EQUALS(renderer.GetFrameRate(), 30.0f, 0.01f, TEST_LOCATION);

  // Marker tm/dr are frame numbers, not seconds: they must not be multiplied by the frame rate.
  CheckMarker(renderer, "idle", 0u, 30u, TEST_LOCATION);
  CheckMarker(renderer, "activate:start", 30u, 60u, TEST_LOCATION);
  CheckMarker(renderer, "activate:end", 60u, 90u, TEST_LOCATION);

  uint32_t startFrame = 0u;
  uint32_t endFrame   = 0u;
  DALI_TEST_EQUALS(renderer.GetMarkerInfo("does-not-exist", startFrame, endFrame), false, TEST_LOCATION);

  Property::Map markers;
  renderer.GetMarkerInfo(markers);
  DALI_TEST_CHECK(markers.Count() == 3u);
  CheckRange(markers, "idle", 0, 30, TEST_LOCATION);
  CheckRange(markers, "activate:start", 30, 60, TEST_LOCATION);
  CheckRange(markers, "activate:end", 60, 90, TEST_LOCATION);
}

} // namespace

void utc_dali_internal_vector_animation_renderer_native_startup(void)
{
  test_return_value = TET_UNDEF;
  // One worker thread, as VectorAnimationRendererEventManager initialises ThorVG in production.
  tvg::Initializer::init(1);
}

void utc_dali_internal_vector_animation_renderer_native_cleanup(void)
{
  tvg::Initializer::term();
  test_return_value = TET_PASS;
}

int UtcDaliVectorAnimationRendererNativeMarkerInfoNotLoaded(void)
{
  tet_infoline("Marker queries before Load() report nothing");

  TestVectorAnimationRenderer renderer;

  uint32_t startFrame = 0u;
  uint32_t endFrame   = 0u;
  DALI_TEST_EQUALS(renderer.GetMarkerInfo("idle", startFrame, endFrame), false, TEST_LOCATION);

  Property::Map markers;
  renderer.GetMarkerInfo(markers);
  DALI_TEST_CHECK(markers.Count() == 0u);

  renderer.Finalize();

  END_TEST;
}

int UtcDaliVectorAnimationRendererNativeMarkerInfoFromUrl(void)
{
  tet_infoline("Marker frame ranges come from ThorVG when loading from a file path");

  TestVectorAnimationRenderer renderer;
  DALI_TEST_EQUALS(renderer.Load(std::string(LOTTIE_MARKERS_FILE)), true, TEST_LOCATION);

  CheckMarkersOfLottieMarkersFile(renderer);

  renderer.Finalize();

  END_TEST;
}

int UtcDaliVectorAnimationRendererNativeMarkerInfoFromData(void)
{
  tet_infoline("Marker frame ranges come from ThorVG when loading from a memory buffer");

  Dali::Vector<uint8_t> data;
  DALI_TEST_CHECK(ReadFile(LOTTIE_MARKERS_FILE, data));

  TestVectorAnimationRenderer renderer;
  DALI_TEST_EQUALS(renderer.Load(data), true, TEST_LOCATION);

  CheckMarkersOfLottieMarkersFile(renderer);

  renderer.Finalize();

  END_TEST;
}

int UtcDaliVectorAnimationRendererNativeMarkerInfoReload(void)
{
  tet_infoline("Reloading rebuilds the marker cache instead of appending to it");

  Dali::Vector<uint8_t> data;
  DALI_TEST_CHECK(ReadFile(LOTTIE_MARKERS_FILE, data));

  TestVectorAnimationRenderer renderer;
  DALI_TEST_EQUALS(renderer.Load(data), true, TEST_LOCATION);
  DALI_TEST_EQUALS(renderer.Load(std::string(LOTTIE_MARKERS_FILE)), true, TEST_LOCATION);

  CheckMarkersOfLottieMarkersFile(renderer);

  renderer.Finalize();

  END_TEST;
}

int UtcDaliVectorAnimationRendererNativeNonLottieDataSkipsMarkerLookup(void)
{
  tet_infoline("A non-Lottie payload (PNG) still loads as a zero-frame picture; the marker query is skipped instead of aborting");

  Dali::Vector<uint8_t> png;
  DALI_TEST_CHECK(ReadFile(NOT_A_LOTTIE_FILE, png));

  TestVectorAnimationRenderer renderer;

  // ThorVG sniffs the content and picks the PNG loader despite the "lottie" hint. Calling the
  // LottieAnimation marker API on that loader aborts or hangs, so Load() must simply return.
  DALI_TEST_EQUALS(renderer.Load(png), true, TEST_LOCATION);
  DALI_TEST_EQUALS(renderer.GetTotalFrameNumber(), 0u, TEST_LOCATION);

  uint32_t startFrame = 0u;
  uint32_t endFrame   = 0u;
  DALI_TEST_EQUALS(renderer.GetMarkerInfo("activate:end", startFrame, endFrame), false, TEST_LOCATION);

  Property::Map markers;
  renderer.GetMarkerInfo(markers);
  DALI_TEST_CHECK(markers.Count() == 0u);

  // The same renderer must still load a real Lottie afterwards (remote PNG first, local file next).
  DALI_TEST_EQUALS(renderer.Load(std::string(LOTTIE_MARKERS_FILE)), true, TEST_LOCATION);
  CheckMarkersOfLottieMarkersFile(renderer);

  renderer.Finalize();

  END_TEST;
}

int UtcDaliVectorAnimationRendererNativeNonLottieUrlSkipsMarkerLookup(void)
{
  tet_infoline("A non-Lottie file path loads as a zero-frame picture without querying markers");

  TestVectorAnimationRenderer renderer;
  DALI_TEST_EQUALS(renderer.Load(std::string(NOT_A_LOTTIE_FILE)), true, TEST_LOCATION);
  DALI_TEST_EQUALS(renderer.GetTotalFrameNumber(), 0u, TEST_LOCATION);

  Property::Map markers;
  renderer.GetMarkerInfo(markers);
  DALI_TEST_CHECK(markers.Count() == 0u);

  renderer.Finalize();

  END_TEST;
}

int UtcDaliVectorAnimationRendererNativeLayerInfo(void)
{
  tet_infoline("Layer frame ranges are still parsed from the Lottie JSON");

  TestVectorAnimationRenderer renderer;
  DALI_TEST_EQUALS(renderer.Load(std::string(LOTTIE_MARKERS_FILE)), true, TEST_LOCATION);

  Property::Map layers;
  renderer.GetLayerInfo(layers);
  DALI_TEST_CHECK(layers.Count() == 3u);
  CheckRange(layers, "idle_layer", 0, 30, TEST_LOCATION);
  CheckRange(layers, "activate_start", 30, 60, TEST_LOCATION);
  CheckRange(layers, "activate_end", 60, 90, TEST_LOCATION);

  renderer.Finalize();

  END_TEST;
}
