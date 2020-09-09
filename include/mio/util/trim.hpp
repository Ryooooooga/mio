#ifndef INCLUDE_mio_util_trim_hpp
#define INCLUDE_mio_util_trim_hpp

#include <string_view>

namespace mio::util {
    constexpr std::string_view trim_start(std::string_view s) noexcept {
        while (!s.empty()) {
            switch (s.front()) {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    s.remove_prefix(1);
                    break;

                default:
                    return s;
            }
        }
        return s;
    }

    constexpr std::string_view trim_end(std::string_view s) noexcept {
        while (!s.empty()) {
            switch (s.back()) {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    s.remove_suffix(1);
                    break;

                default:
                    return s;
            }
        }
        return s;
    }

    constexpr std::string_view trim(std::string_view s) noexcept {
        return trim_end(trim_start(s));
    }
} // namespace mio::util

#endif // INCLUDE_mio_util_trim_hpp
