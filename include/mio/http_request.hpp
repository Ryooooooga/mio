#ifndef INCLUDE_mio_http_request_hpp
#define INCLUDE_mio_http_request_hpp

#include "http_headers.hpp"

namespace mio {
    class http_request {
    public:
        http_request(std::string_view method, std::string_view request_uri, std::string_view http_version, http_headers&& headers, std::vector<std::byte>&& body = {})
            : method_(method)
            , request_uri_(request_uri)
            , query_index_(request_uri_.find('?'))
            , http_version_(http_version)
            , headers_(std::move(headers))
            , body_(std::move(body))
            , params_()
            , form_() {
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

        [[nodiscard]] std::string_view request_uri() const noexcept {
            return request_uri_;
        }

        [[nodiscard]] std::string_view path() const noexcept {
            return request_uri().substr(0, query_index_);
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

        [[nodiscard]] std::span<const std::byte> body() const noexcept {
            return body_;
        }

        [[nodiscard]] std::string_view body_as_text() const noexcept {
            return std::string_view{reinterpret_cast<const char*>(body_.data()), body_.size()};
        }

        void set_param(std::string&& key, std::string&& value) {
            params_.emplace(std::move(key), std::move(value));
        }

        std::optional<std::string_view> param(const std::string& key) const {
            if (const auto it = params_.find(key); it != std::end(params_)) {
                return it->second;
            }
            return std::nullopt;
        }

        void set_form(std::string&& key, std::string&& value) {
            if (const auto it = form_.find(key); it != std::end(form_)) {
                it->second.emplace_back(std::move(value));
            } else {
                form_.emplace(std::move(key), std::vector<std::string>{std::move(value)});
            }
        }

        std::optional<std::string_view> form(const std::string& key) const {
            if (const auto p = form_values(key); p && !p->empty()) {
                return p->front();
            }
            return std::nullopt;
        }

        const std::vector<std::string>* form_values(const std::string& key) const {
            if (const auto it = form_.find(key); it != std::end(form_)) {
                return &it->second;
            }
            return nullptr;
        }

    private:
        std::string method_;
        std::string request_uri_;
        std::size_t query_index_;
        std::string http_version_;
        http_headers headers_;
        std::vector<std::byte> body_;
        std::unordered_map<std::string, std::string> params_;
        std::unordered_map<std::string, std::vector<std::string>> form_;
    };
} // namespace mio

#endif // INCLUDE_mio_http_request_hpp
