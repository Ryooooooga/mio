#ifndef INCLUDE_mio_http1_request_parser_hpp
#define INCLUDE_mio_http1_request_parser_hpp

#include <span>
#include <string_view>

namespace mio::http1 {
    struct header {
        std::string_view key;
        std::string_view value;
    };

    struct request {
        std::string_view method;
        std::string_view request_uri;
        std::string_view http_version;
        std::span<header> headers;
    };

    enum class [[nodiscard]] parse_result{
        done,
        in_progress,
        invalid,
        too_many_headers,
    };

    parse_result parse_request(request& req, std::span<header> headers, std::string_view input);
} // namespace mio::http1

#endif // INCLUDE_mio_http1_request_parser_hpp
