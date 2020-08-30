add_executable(test_mio
    test.cpp
    http1/test_request.cpp
    http1/test_response.cpp
    test_http_headers.cpp
)

target_link_libraries(test_mio
    mio
)

add_test(NAME tests::mio
    COMMAND $<TARGET_FILE:test_mio>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
