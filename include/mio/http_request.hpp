#ifndef INCLUDE_mio_http_request_hpp
#define INCLUDE_mio_http_request_hpp

#include "http_headers.hpp"

namespace mio {
    class http_request {
    public:
        http_request(std::string_view method, std::string_view request_uri, std::string_view http_version, http_headers&& headers)
            : method_(method)
            , request_uri_(request_uri)
            , http_version_(http_version)
            , headers_(std::move(headers)) {
        }

        ~http_request() noexcept = default;

        // Uncopyable and movable
        http_request(const http_request&) = delete;
        http_request(http_request&&) = default;

        http_request& operator=(const http_request&) = delete;
        http_request& operator=(http_request&&) = default;

        [[nodiscard]] const std::string& method() const noexcept {
            return method_;
        }

        [[nodiscard]] const std::string& request_uri() const noexcept {
            return request_uri_;
        }

        [[nodiscard]] const std::string& http_version() const noexcept {
            return http_version_;
        }

        [[nodiscard]] http_headers& headers() noexcept {
            return headers_;
        }

        [[nodiscard]] const http_headers& headers() const noexcept {
            return headers_;
        }

    private:
        std::string method_;
        std::string request_uri_;
        std::string http_version_;
        http_headers headers_;
    };
} // namespace mio

#endif // INCLUDE_mio_http_request_hpp
