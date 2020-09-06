#ifndef INCLUDE_mio_http1_request_hpp
#define INCLUDE_mio_http1_request_hpp

#include <span>
#include "header.hpp"

namespace mio::http1 {
    struct request {
        std::string_view method;
        std::string_view request_uri;
        std::string_view http_version;
        std::span<header> headers;
    };

    enum class [[nodiscard]] parse_result{
        completed,
        in_progress,
        invalid,
        too_many_headers,
    };

    parse_result parse_request(request& req, std::span<header> headers, std::string_view input, std::size_t& header_size);
} // namespace mio::http1

#endif // INCLUDE_mio_http1_request_hpp
