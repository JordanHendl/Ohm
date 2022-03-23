if(NOT Build_Release)
  add_definitions(-DOhm_Debug)
endif()

add_subdirectory(api)
add_subdirectory(io)
add_subdirectory(vulkan)
add_subdirectory(test)
add_subdirectory(benchmark)
