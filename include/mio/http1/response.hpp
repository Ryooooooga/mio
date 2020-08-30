#ifndef INCLUDE_mio_http1_response_hpp
#define INCLUDE_mio_http1_response_hpp

#include <cstdint>
#include <ostream>
#include <span>
#include <unordered_map>
#include "header.hpp"

namespace mio::http1 {
    struct response {
        std::string_view http_version;
        std::int32_t status_code;
        std::span<header> headers;
        std::string_view content;
    };

    extern const std::unordered_map<std::int32_t, std::string_view> status_codes;

    inline std::string_view status_code_string(std::int32_t status_code) noexcept {
        if (const auto it = status_codes.find(status_code); it != std::end(status_codes)) {
            return it->second;
        }
        return "Unknown";
    }

    void write_response(std::ostream& ostream, const response& res);
} // namespace mio::http1

#endif // INCLUDE_mio_http1_response_hpp
