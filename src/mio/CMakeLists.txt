add_library(mio STATIC
    bodies/x_www_form_url_encoded.cpp
    http1/request.cpp
    http1/response.cpp
    sockets/socket.cpp
    middlewares/static.cpp
    application.cpp
    http_headers.cpp
    http_server.cpp
    router.cpp
)

target_include_directories(mio
    PUBLIC "../../include"
)

target_link_libraries(mio
    Threads::Threads
)
