#include "mio/http_server.hpp"

#include <iostream>

int main() {
    mio::http_server{}.listen(3000, []() {
        std::cout << "hello" << std::endl;
    });
}
