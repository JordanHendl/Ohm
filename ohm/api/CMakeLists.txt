
file(GLOB api_headers ${CMAKE_CURRENT_SOURCE_DIR})
install(FILES ${api_headers} DESTINATION ${HEADER_INSTALL_DIR}/api COMPONENT devel)