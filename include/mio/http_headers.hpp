#ifndef INCLUDE_mio_http_header_hpp
#define INCLUDE_mio_http_header_hpp

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mio {
    struct http_header {
        std::string key;
        std::string value;
    };

    class http_headers {
    public:
        http_headers() = default;
        ~http_headers() noexcept = default;

        // Uncopyable and movable
        http_headers(const http_headers&) = delete;
        http_headers(http_headers&&) = default;

        http_headers& operator=(const http_headers&) = delete;
        http_headers& operator=(http_headers&&) = default;

        [[nodiscard]] std::optional<std::string> get(std::string_view key) const;
        void set(std::string_view key, std::string_view value);
        void append(std::string_view key, std::string_view value);
        void remove(std::string_view key);

        [[nodiscard]] std::span<http_header> entries() noexcept {
            return entries_;
        }

        [[nodiscard]] std::span<const http_header> entries() const noexcept {
            return entries_;
        }

    private:
        std::vector<http_header> entries_;
        std::unordered_map<std::string, std::size_t> indices_;
    };
} // namespace mio

#endif // INCLUDE_mio_http_header_hpp
