#ifndef INCLUDE_mio_uri_hpp
#define INCLUDE_mio_uri_hpp

#include <optional>
#include <string>
#include <string_view>

namespace mio {
    namespace detail {
        constexpr bool is_hex_digit(char c) noexcept {
            return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
        }
    } // namespace detail

    inline std::optional<std::string> decode_uri(std::string_view s, bool replace_plus) {
        std::string text{};
        text.reserve(s.size());

        for (std::size_t i = 0; i < s.size();) {
            if (s[i] == '%') {
                if (i + 2 >= s.size() || !detail::is_hex_digit(s[i + 1]) || !detail::is_hex_digit(s[i + 2])) {
                    return std::nullopt;
                }

                const std::uint8_t hi = (s[i + 1] <= '9') ? s[i + 1] - '0' : (s[i + 1] <= 'F') ? s[i + 1] - 'A' + 10 : s[i + 1] - 'a' + 10;
                const std::uint8_t lo = (s[i + 2] <= '9') ? s[i + 2] - '0' : (s[i + 2] <= 'F') ? s[i + 2] - 'A' + 10 : s[i + 2] - 'a' + 10;
                const std::uint8_t byte = (hi << 4) | lo;

                text += static_cast<char>(byte);
                i += 3;
            } else if (s[i] == '+' && replace_plus) {
                text += ' ';
                i++;
            } else {
                text += s[i];
                i++;
            }
        }

        return text;
    }
} // namespace mio

#endif // INCLUDE_mio_uri_hpp
