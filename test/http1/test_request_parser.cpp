#include "mio/http1/request_parser.hpp"

#include <cassert>

void test_request_parser() {
    mio::http1::request req;
    mio::http1::header buffer[100];

    const std::string_view input =
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Accept:*/*\r\n"
        "\r\n";

    const mio::http1::parse_result result = mio::http1::parse_request(req, buffer, input);

    assert(result == mio::http1::parse_result::done);
    assert(req.method == "GET");
    assert(req.request_uri == "/index.html");
    assert(req.http_version == "HTTP/1.1");
    assert(req.headers.size() == 2);
    assert(req.headers[0].key == "Host");
    assert(req.headers[0].value == "example.com");
    assert(req.headers[1].key == "Accept");
    assert(req.headers[1].value == "*/*");
}
