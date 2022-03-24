execute_process( 
  COMMAND tar -xf ${CMAKE_CURRENT_SOURCE_DIR}/spirv-reflect.tar.gz
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/spirv-reflect ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})