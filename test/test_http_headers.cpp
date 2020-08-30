#include "mio/http_headers.hpp"

#include <cassert>

void test_http_headers() {
    mio::http_headers headers{
        {"Content-Type", "text/plain"},
        {"Content-Encoding", "gzip"},
        {"Server", "Mio"},
    };

    assert(headers.entries().size() == 3);
    assert(headers.entries()[0].key == "content-type");
    assert(headers.entries()[0].value == "text/plain");
    assert(headers.entries()[1].key == "content-encoding");
    assert(headers.entries()[1].value == "gzip");
    assert(headers.entries()[2].key == "server");
    assert(headers.entries()[2].value == "Mio");

    assert(headers.get("server") == "Mio");
    assert(headers.get("SERVER") == "Mio");
    assert(headers.get("Expires") == std::nullopt);

    headers.append("Vary", "User-Agent");
    headers.append("Vary", "*");

    assert(headers.get("Vary") == "User-Agent, *");

    headers.set("Vary", "User-Agent");
    headers.set("Vary", "*");

    assert(headers.get("Vary") == "*");

    headers.remove("content-encoding");

    assert(headers.get("Content-Encoding") == std::nullopt);

    assert(headers.entries().size() == 3);
    assert(headers.entries()[0].key == "content-type");
    assert(headers.entries()[0].value == "text/plain");
    assert(headers.entries()[1].key == "server");
    assert(headers.entries()[1].value == "Mio");
    assert(headers.entries()[2].key == "vary");
    assert(headers.entries()[2].value == "*");
}
