#ifndef INCLUDE_mio_http_response_hpp
#define INCLUDE_mio_http_response_hpp

#include <cassert>
#include "http_headers.hpp"

namespace mio {
    class http_response {
    public:
        explicit http_response(std::int32_t status_code)
            : status_code_(status_code)
            , headers_()
            , body_() {
        }

        http_response(std::int32_t status_code, std::span<const std::byte> body)
            : status_code_(status_code)
            , headers_()
            , body_(std::begin(body), std::end(body)) {
        }

        http_response(std::int32_t status_code, std::string_view body)
            : http_response(status_code, std::as_bytes(std::span{body})) {
        }

        http_response(std::int32_t status_code, http_headers&& headers, std::span<const std::byte> body)
            : status_code_(status_code)
            , headers_(std::move(headers))
            , body_(std::begin(body), std::end(body)) {
            assert(!headers_.get("content-length"));
        }

        http_response(std::int32_t status_code, http_headers&& headers, std::string_view body)
            : http_response(status_code, std::move(headers), std::as_bytes(std::span{body})) {
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

        [[nodiscard]] std::span<const std::byte> body() const noexcept {
            return body_;
        }

        [[nodiscard]] std::string_view body_as_text() const noexcept {
            return std::string_view{reinterpret_cast<const char*>(body_.data()), body_.size()};
        }

        [[nodiscard]] std::size_t content_length() const noexcept {
            return body_.size();
        }

        void body(std::span<const std::byte> bytes) {
            body_.clear();
            write(bytes);
        }

        void body(std::string_view text) {
            body(std::as_bytes(std::span{text}));
        }

        void write(std::span<const std::byte> bytes) {
            body_.insert(std::end(body_), std::begin(bytes), std::end(bytes));
        }

        void write(std::string_view text) {
            write(std::as_bytes(std::span{text}));
        }

        static http_response html(std::int32_t status_code, std::string_view body) {
            return http_response{
                status_code,
                http_headers{
                    {"content-type", "text/html; charset=utf8"},
                },
                body,
            };
        }

        static http_response json(std::int32_t status_code, std::string_view body) {
            return http_response{
                status_code,
                http_headers{
                    {"content-type", "application/json; charset=utf8"},
                },
                body,
            };
        }

    private:
        std::int32_t status_code_;
        http_headers headers_;
        std::vector<std::byte> body_;
    };
} // namespace mio

#endif // INCLUDE_mio_http_response_hpp
