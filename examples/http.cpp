#include "mio/http_server.hpp"

#include <iostream>

int main() {
    mio::http_server{}.listen(3000, [](const mio::http_request& req) {
        std::cout << req.method() << " " << req.request_uri() << " " << req.http_version() << std::endl;
        for (const auto& [key, value] : req.headers().entries()) {
            std::cout << key << ": " << value << std::endl;
        }
        std::cout << std::endl;
    });
}
