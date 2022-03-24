# SPIRV-Reflect: https://github.com/KhronosGroup/SPIRV-Reflect
# Used for doing SPIRV reflection.
# This is untarred externally and built from there.
execute_process( 
  COMMAND tar -xf ${CMAKE_CURRENT_SOURCE_DIR}/spirv-reflect.tar.gz
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set(spirv-reflect_include_dirs ${CMAKE_CURRENT_BINARY_DIR}/spirv-reflect/ CACHE INTERNAL "spirv-reflect external include path.")
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/spirv-reflect ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})