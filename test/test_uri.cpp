#include "mio/uri.hpp"

#include <cassert>

void test_uri() {
    assert(mio::decode_uri("a%20b") == "a b");
    assert(mio::decode_uri("%3Fx%3dtest") == "?x=test");
}
