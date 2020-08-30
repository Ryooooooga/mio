#ifndef INCLUDE_mio_http1_header_hpp
#define INCLUDE_mio_http1_header_hpp

#include <string_view>

namespace mio::http1 {
    struct header {
        std::string_view key;
        std::string_view value;
    };
} // namespace mio::http1

#endif // INCLUDE_mio_http1_header_hpp
