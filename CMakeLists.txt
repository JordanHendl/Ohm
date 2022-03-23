cmake_minimum_required( VERSION 3.8.0 ) 

list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
project( ohm CXX C )

option( Build_Benchmarks "Whether or not to build benchmarks for this project." ON )
option( Build_Release "Whether or not to build the release version of this project." ON )
option( Build_Tests "Whether or not to build tests for this project." ON )
option( Run_Tests "Whether or not running of tests should occur as a part of the build." ON )

if( Run_Tests )
  enable_testing()
endif()

# Set build config.
set( Architecture "64bit" CACHE STRING "The system architecture."                        )
set( CXX_Standard "17"    CACHE STRING "The C++ standard to use for building."           )
set( Major        "0"     CACHE STRING "The major version of this build."                )
set( Minor        "1"     CACHE STRING "The minor version of this build."                )
set( Branch       "0"     CACHE STRING "The branch version of this build."               )
set( Revision     "0"     CACHE STRING "The revision of this build."                     )
set( Generator    "DEB"   CACHE STRING "The Package Generator to use for this build."    )
set( Sanitizer    "None"  CACHE STRING "The sanitizer to use for this project's build." )

set( PROJECT_VERSION "${Major}.${Minor}.${Branch}.${Revision}" )

set(CMAKE_CXX_STANDARD          ${CXX_Standard} )
set(CMAKE_CXX_STANDARD_REQUIRED ON              )


if( Build_Release )
  set(CMAKE_BUILD_TYPE Release)
  if( MSVC )
    set( CXX_COMPILE_OPTIONS /W2 /O2 ) 
    set( LINKER_OPTIONS "" ) 
  elseif( UNIX )
    set( C_COMPILE_OPTIONS -Wall -Werror -fPIC -O2 ) 
    set( CXX_COMPILE_OPTIONS -Wall -Werror -fPIC -O2 -fno-exceptions -fno-rtti ) 
    set( LINKER_OPTIONS "" ) 
  endif()
else()
  if( MSVC )
    set( CXX_COMPILE_OPTIONS /W2 /DEBUG ) 
    set( LINKER_OPTIONS "" ) 
  elseif( UNIX )
    set( CXX_COMPILE_OPTIONS -Wall -Werror -fno-omit-frame-pointer -fPIC -g ) 

    if(${Sanitizer} MATCHES "Address")
      set( LINKER_OPTIONS -fsanitize=address ) 
      set( CXX_COMPILE_OPTIONS ${CXX_COMPILE_OPTIONS} -fsanitize=address )
    endif()

    if(${Sanitizer} MATCHES "Thread")
      set( LINKER_OPTIONS -fsanitize=thread ) 
      set( CXX_COMPILE_OPTIONS ${CXX_COMPILE_OPTIONS} -fsanitize=thread )
    endif()

    set(Safe_Sanitizers -fsanitize=null -fsanitize=signed-integer-overflow -fsanitize=bounds -fsanitize-address-use-after-scope
                        -fsanitize=undefined )

    if( NOT ${Sanitizer} MATCHES "None" )
      set( LINKER_OPTIONS ${LINKER_OPTIONS} ${Safe_Sanitizers}) 
      set( CXX_COMPILE_OPTIONS ${CXX_COMPILE_OPTIONS} ${Safe_Sanitizers})
    endif()
  endif()
endif()

add_compile_options( ${CXX_COMPILE_OPTIONS} )
add_link_options( ${LINKER_OPTIONS} )

set( EXPORT_BIN_DIR     bin     )
set( EXPORT_INCLUDE_DIR include )
set( EXPORT_LIB_DIR     lib     )
set( EXPORT_DOC_DIR     doc     )

# Set output directories.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_LIB_DIR} )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_LIB_DIR} )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_BIN_DIR} )
set(DOCUMENTATION_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_DOC_DIR} )

set( CONFIG_DIR         ${CMAKE_PROJECT_NAME}                       )
set( DOXYGEN_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/doc             )
set( BUILD_DIR          ${CMAKE_BINARY_DIR}                         )
set( HEADER_INSTALL_DIR ${EXPORT_INCLUDE_DIR}/${CMAKE_PROJECT_NAME} )

message("Ohm Prebuild" )
message("-- Build Release: ${Build_Release}")
message("-- Build Benchmarks: ${Build_Benchmarks}")
message("-- Build Tests: ${Build_Tests}")
message("-- Package: ${Generator}")
message("-- Sanitizer: ${Sanitizer}")
message("-- Compile options: ${CXX_COMPILE_OPTIONS}")
message("")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libshaderc/include/)


set( CMAKE_SHARED_LIBRARY_PREFIX "o${CMAKE_SHARED_LIBRARY_PREFIX}" )

include         ( Package  )
add_subdirectory( external )
add_subdirectory( ohm      )

install( EXPORT ohm FILE ohm-config.cmake 
                    DESTINATION cmake 
                    NAMESPACE   ohm::
                    COMPONENT   devel )
