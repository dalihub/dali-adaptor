#include <dali-test-suite-utils.h>

#include <dali/graphics-api/graphics-controller.h>
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <test-graphics-egl-application.h>

//#include <dali/graphics-api/graphics-api-buffer.h>

int UtcDaliGraphicsCreateGeometry(void)
{
  // Initialize actual egl graphics controller (without initializing egl!)
  TestGraphicsApplication app;

  struct Vertex
  {
    float x;
    float y;
  };

  std::vector<Vertex>   someData(100);
  Graphics::Controller& graphicsController = app.GetGraphicsController();

  Graphics::BufferCreateInfo createInfo;
  createInfo
    .SetUsage(0 | Graphics::BufferUsage::VERTEX_BUFFER)
    .SetSize(someData.size());

  auto                    buffer = graphicsController.CreateBuffer(createInfo, nullptr);
  Graphics::MapBufferInfo info;
  info.buffer = buffer.get();
  info.usage  = 0 | Graphics::MemoryUsageFlagBits::WRITE;
  info.offset = 0;
  info.size   = someData.size();

  auto  memory = graphicsController.MapBufferRange(info);
  void* ptr    = memory->LockRegion(0, someData.size());
  DALI_TEST_CHECK(ptr != nullptr);
  memory->Unlock(true);
  graphicsController.UnmapMemory(std::move(memory));

  // Test that data has been uploaded to GL, e.g. test that GPU buffer has been created
  auto& gl              = app.GetGlAbstraction();
  auto& bufferDataCalls = gl.GetBufferDataCalls();
  DALI_TEST_EQUALS(bufferDataCalls.size(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(bufferDataCalls[0], someData.size(), TEST_LOCATION);
  DALI_TEST_CHECK(1);
  END_TEST;
}
