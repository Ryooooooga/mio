#include "mio/http1/response.hpp"

#include <cassert>
#include <sstream>

namespace {
    void test_status_code() {
        assert(mio::http1::status_code_string(200) == "OK");
        assert(mio::http1::status_code_string(201) == "Created");
        assert(mio::http1::status_code_string(404) == "Not Found");
    }

    void test_response_writer() {
        mio::http1::response res{};
        res.http_version = "HTTP/1.1";
        res.status_code = 200;

        mio::http1::header headers[] = {
            {"content-type", "text/html; charset=utf-8"},
            {"last-modified", "Sun, 30 Aug 2020 01:23:45 GMT"},
        };

        res.headers = headers;
        res.content = "<p>Hello</p>";

        std::ostringstream oss{};
        mio::http1::write_response(oss, res);

        assert(oss.str() ==
               "HTTP/1.1 200 OK\r\n"
               "content-type: text/html; charset=utf-8\r\n"
               "last-modified: Sun, 30 Aug 2020 01:23:45 GMT\r\n"
               "content-length: 12\r\n"
               "\r\n"
               "<p>Hello</p>");
    }
} // namespace

void test_response() {
    test_status_code();
    test_response_writer();
}
