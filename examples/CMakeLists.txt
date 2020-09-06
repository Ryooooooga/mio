set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples")

add_executable(simple_http simple_http.cpp)
target_link_libraries(simple_http mio)
