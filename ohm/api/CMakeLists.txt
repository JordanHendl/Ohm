
set(api_headers
    array.h
    allocators.h
    commands.h
    exception.h
    image.h
    memory.h
    system.h
    ohm.h
   )


install(FILES ${api_headers} DESTINATION ${HEADER_INSTALL_DIR}/api COMPONENT devel)