#include "mio/http_server.hpp"

int main() {
    mio::http_server{}.listen(3000);
}
