find_package(Vulkan)
find_package(SDL2 REQUIRED)
if(Vulkan_FOUND)

add_subdirectory(impl)

set(vulkan_hdrs
    vulkan_impl.h
   )
set(vulkan_srcs 
    vulkan_impl.cpp
   )

set(vulkan_includes
     ${SDL2_INCLUDE_DIRS}
     ${Vulkan_INCLUDE_DIRS}
   )
set(vulkan_libs
     vulkan_impl
     ${SDL2_LIBRARIES}
     ${shaderc_libraries}
     io
   )

add_library               (vulkan SHARED  ${vulkan_srcs})
target_link_libraries     (vulkan PRIVATE  ${vulkan_libs})
target_include_directories(vulkan PRIVATE ${vulkan_includes})

install( FILES  ${vulkan_hdrs} DESTINATION ${HEADER_INSTALL_DIR}/vulkan COMPONENT devel )
install( TARGETS vulkan EXPORT ohm COMPONENT release  
         ARCHIVE  DESTINATION ${EXPORT_LIB_DIR}
         RUNTIME  DESTINATION ${EXPORT_LIB_DIR}
         LIBRARY  DESTINATION ${EXPORT_LIB_DIR}
         INCLUDES DESTINATION ${EXPORT_INCLUDE_DIR} )
else()
message("-- Couldn't find vulkan development libraries, skipping vulkan implementation build.")
endif()

