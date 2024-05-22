Intended directory layout
-------------------------

To match EGL/GLES implementation in latest DALi:

vulkan/  Top level graphics interfaces, graphics-interface implementations, surface factory & impls.
         Enough code to get a vulkan surface and swapchain (and skeleton controller?)

vulkan-impl/
  - Graphics-API implementations, controller implementation, other impl( e.g. descriptor sets, pipeline cache)
  - Currently contains OLD layout:
      vulkan-impl contains vulkan-graphics.cpp
      vulkan-impl/api contains handles & controller
      vulkan-impl/api/internal contains descriptor sets, pipeline cache
      vulkan-impl/internal contains api implementations

TODO:
  Remove handles.
  Remove factories.
  Refactor vulkan-graphics & vulkan-api-controller.
  Refactor Dali::Graphics::VulkanAPI namespace to Dali::Graphics::Vulkan

