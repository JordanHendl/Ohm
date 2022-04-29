find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

set(vulkan_impl_srcs
     instance.cpp
     system.cpp
     device.cpp
     memory.cpp
     buffer.cpp
     image.cpp
     command_buffer.cpp
     shader.cpp
     pipeline.cpp
     descriptor.cpp
     render_pass.cpp
     swapchain.cpp
     event.cpp
     window.cpp
   )

set(vulkan_impl_includes
     ${Vulkan_INCLUDE_DIRS}
     ${SDL2_INCLUDE_DIRS}
   )

set(vulkan_impl_libs
     ${SDL2_LIBRARIES}
     io
   )

add_library               (vulkan_impl STATIC  ${vulkan_impl_srcs})
target_link_libraries     (vulkan_impl PUBLIC  ${vulkan_impl_libs})
target_include_directories(vulkan_impl PRIVATE ${vulkan_impl_includes})
