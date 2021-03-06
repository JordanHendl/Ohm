if( WIN32 )
 set( CPACK_GENRATOR                  "WIX"                 )
 set( CPACK_PACKAGING_INSTALL_PREFIX  ""                    )
 set( CPACK_PACKAGE_INSTALL_DIRECTORY ${CMAKE_PROJECT_NAME} )
endif() 

if( UNIX )
  set( CPACK_GENERATOR                       ${Generator}                                  )
  set( CPACK_PACKAGING_INSTALL_PREFIX        ${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME} )
  set( CPACK_PACKAGE_INSTALL_DIRECTORY       ${CMAKE_PROJECT_NAME}                         )

  set( CPACK_RPM_PACKAGE_AUTOREQ             OFF                                           )
  set( CPACK_RPM_PACKAGE_AUTOPROV            ON                                            )
  set( CPACK_RPM_SPEC_MORE_DEFINE            "%define _build_id_links none"                )
  set( CPACK_RPM_COMPONENT_INSTALL           ON                                            )

  set( CPACK_DEBIAN_PACKAGE_DEBUG            OFF                                           )
  set( CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON                                            )
  set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS        ON                                            )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER       "N/A"                                         )
  set( CPACK_DEB_COMPONENT_INSTALL           ON                                            )
endif() 

set( CMAKE_INSTALL_RPATH   ${CPACK_PACKAGING_INSTALL_PREFIX}/${EXPORT_LIB_DIR} )
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH ON                                      )
set( CPACK_PACKAGE_VERSION ${PROJECT_VERSION}                                  )
set( CPACK_PACKAGE_NAME    ${CMAKE_PROJECT_NAME}                               )

set( CPACK_COMPONENTS_ALL                 release devel                                                                  )
set( CPACK_COMPONENT_DEVEL_DISPLAY_NAME   "${CMAKE_PROJECT_NAME}-devel"                                                  )
set( CPACK_COMPONENT_RELEASE_DISPLAY_NAME "${CMAKE_PROJECT_NAME}-release"                                                )
set( CPACK_COMPONENT_DEVEL_DESCRIPTION    "Development Headers & Runtime Libraries for the ${CMAKE_PROJECT_NAME} library")
set( CPACK_COMPONENT_RELEASE_DESCRIPTION  "Runtime Libraries for the ${CMAKE_PROJECT_NAME} library"                      )
set( CPACK_COMPONENT_DEVEL_DEPENDS        release                                                                        )
set( CPACK_COMPONENT_DEVEL_REQUIRED       release                                                                        )
set( CPACK_RPM_DEVEL_PACKAGE_REQUIRES     ohm-release ) 
set( CPACK_RPM_TOOLS_PACKAGE_REQUIRES     ohm-release ) 

include (CMakePackageConfigHelpers)
write_basic_package_version_file (
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
         DESTINATION cmake COMPONENT devel )

include( CPack )

