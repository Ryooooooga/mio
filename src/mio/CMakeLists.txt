add_library(mio STATIC
    http1/request_parser.cpp
)

target_include_directories(mio
    PUBLIC "../../include"
)
