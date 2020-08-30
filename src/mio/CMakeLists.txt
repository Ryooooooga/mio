add_library(mio STATIC
    http1/request_parser.cpp
    http_headers.cpp
    http_server.cpp
)

target_include_directories(mio
    PUBLIC "../../include"
)

target_link_libraries(mio
    Threads::Threads
)
