
set(api_headers
    allocators.h
    array.h
    commands.h
    descriptor.h
    event.h
    exception.h
    image.h
    memory.h
    ohm.h
    pipeline.h
    render_pass.h
    system.h
    window.h
   )


install(FILES ${api_headers} DESTINATION ${HEADER_INSTALL_DIR}/api COMPONENT devel)