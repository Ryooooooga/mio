#include "mio/http_server.hpp"

#include <iostream>

int main() {
    mio::http_server{}.listen(3000, []([[maybe_unused]] const mio::http_request& req) {
        return mio::http_response{
            200,
            mio::http_headers{
                {"Content-Type", "text/html; charset=utf8"},
            },
            "<html>Hello, world!</html>",
        };
    });
}
