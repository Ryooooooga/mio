#ifndef INCLUDE_mio_http_response_hpp
#define INCLUDE_mio_http_response_hpp

#include "mio/http_headers.hpp"

namespace mio {
    class http_response {
    public:
        explicit http_response(std::int32_t status_code, std::string_view content = {})
            : status_code_(status_code)
            , headers_()
            , content_(content) {
        }

        http_response(std::int32_t status_code, http_headers&& headers, std::string_view content = {})
            : status_code_(status_code)
            , headers_(std::move(headers))
            , content_(content) {
        }

        ~http_response() noexcept = default;

        // Uncopyable and movable
        http_response(const http_response&) = delete;
        http_response(http_response&&) = default;

        http_response& operator=(const http_response&) = delete;
        http_response& operator=(http_response&&) = default;

        [[nodiscard]] std::int32_t status_code() const noexcept {
            return status_code_;
        }

        void set_status_code(std::int32_t status_code) noexcept {
            status_code_ = status_code;
        }

        [[nodiscard]] http_headers& headers() noexcept {
            return headers_;
        }

        [[nodiscard]] const http_headers& headers() const noexcept {
            return headers_;
        }

        [[nodiscard]] const std::string& content() const noexcept {
            return content_;
        }

        [[nodiscard]] std::size_t content_length() const noexcept {
            return content_.size();
        }

        void body(std::string_view s) {
            content_ = s;
        }

        void write(std::string_view s) {
            content_ += s;
        }

    private:
        std::int32_t status_code_;
        http_headers headers_;
        std::string content_;
    };
} // namespace mio

#endif // INCLUDE_mio_http_response_hpp
