find_package(Vulkan)
find_package(SDL2 REQUIRED)
if(Vulkan_FOUND)

add_subdirectory(impl)

set(vulkan_srcs 
    vulkan_impl.cpp
   )

set(vulkan_libs
     vulkan_impl
   )

add_library               (vulkan SHARED  ${vulkan_srcs})
target_link_libraries     (vulkan PUBLIC  ${vulkan_libs})
target_include_directories(vulkan PRIVATE ${vulkan_includes})

endif()
