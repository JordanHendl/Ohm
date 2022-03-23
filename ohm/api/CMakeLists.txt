
set(api_headers
    array.h
    commands.h
    exception.h
    image.h
    memory.h
    system.h
    ohm.h
   )

set(api_ext_headers
    bits/pool_allocator.h
   )

install(FILES ${api_headers} DESTINATION ${HEADER_INSTALL_DIR}/api COMPONENT devel)
install(FILES ${api_ext_headers} DESTINATION ${HEADER_INSTALL_DIR}/api/bits COMPONENT devel)