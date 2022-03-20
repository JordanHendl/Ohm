set( io_sources
     dlloader.cpp
   )

set( io_headers
     dlloader.h
   )

set( io_includes
   )

set( io_libraries
    ${CMAKE_DL_LIBS}
   )

add_library               ( io STATIC  ${io_sources} ${io_headers} )
target_link_libraries     ( io PUBLIC  ${io_libraries}             )
target_include_directories( io PRIVATE ${io_includes}              )