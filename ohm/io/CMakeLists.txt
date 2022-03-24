set( io_sources
     dlloader.cpp
     shader.cpp
   )

set( io_headers
     dlloader.h
     shader.h
   )

set( io_includes
      ${spirv-reflect_include_dirs}
      ${shaderc_include_dirs}
   )

set( io_libraries
    shaderc_combined  
    ${CMAKE_DL_LIBS}
    spirv_reflect
   )

add_library               ( io SHARED  ${io_sources} ${io_headers} )
target_link_libraries     ( io PRIVATE ${io_libraries}             )
target_include_directories( io PRIVATE ${io_includes}              )
install( FILES  ${io_headers} DESTINATION ${HEADER_INSTALL_DIR}/io COMPONENT devel )
install( TARGETS io EXPORT ohm COMPONENT release  
         ARCHIVE  DESTINATION ${EXPORT_LIB_DIR}
         RUNTIME  DESTINATION ${EXPORT_LIB_DIR}
         LIBRARY  DESTINATION ${EXPORT_LIB_DIR}
         INCLUDES DESTINATION ${EXPORT_INCLUDE_DIR} )