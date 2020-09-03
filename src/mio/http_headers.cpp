#include "mio/http_headers.hpp"

#include <algorithm>
#include <charconv>

namespace mio {
    namespace {
        constexpr char to_ascii_lower(char c) noexcept {
            // Character encoding should follows ASCII.
            return ('A' <= c && c <= 'Z') ? static_cast<unsigned char>(c) | 0x20 : c;
        }

        std::string to_lower(std::string_view s) {
            std::string t{};
            t.reserve(s.size());

            std::ranges::copy(std::ranges::views::transform(s, to_ascii_lower), std::back_inserter(t));
            return t;
        }

        template <std::integral Int>
        std::optional<Int> parse_int(std::string_view s) noexcept {
            Int value;
            if (const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value, 10); ec != std::errc{}) {
                return std::nullopt;
            }
            return value;
        }
    } // namespace

    http_headers::http_headers(std::initializer_list<http_header> headers)
        : entries_()
        , indices_()
        , content_length_(0) {
        for (const http_header& header : headers) {
            append(header.key, header.value);
        }
    }

    template <std::ranges::range Headers>
    http_headers::http_headers(const Headers& headers)
        : entries_()
        , indices_()
        , content_length_(0) {
        for (const http_header& header : headers) {
            append(header.key, header.value);
        }
    }

    std::optional<std::string_view> http_headers::get(std::string_view key) const {
        const auto key_lower = to_lower(key);
        if (const auto it = indices_.find(key_lower); it != std::end(indices_)) {
            return entries_[it->second].value;
        }

        return std::nullopt;
    }

    void http_headers::set(std::string_view key, std::string_view value) {
        auto key_lower = to_lower(key);
        if (const auto it = indices_.find(key_lower); it != std::end(indices_)) {
            entries_[it->second].value = value;
        } else {
            indices_.emplace(key_lower, entries_.size());
            entries_.emplace_back(http_header{std::move(key_lower), std::string{value}});

            if (key_lower == "content-length") {
                const auto content_length = parse_int<std::size_t>(value);
                if (!content_length) {
                    throw std::runtime_error{"invalid request"};
                }

                content_length_ = *content_length;
            }
        }
    }

    void http_headers::append(std::string_view key, std::string_view value) {
        auto key_lower = to_lower(key);
        if (const auto it = indices_.find(key_lower); it != std::end(indices_)) {
            if (key_lower == "content-length") {
                throw std::runtime_error{"invalid request"};
            }

            auto& entry = entries_[it->second];
            entry.value += ", ";
            entry.value += value;
        } else {
            indices_.emplace(key_lower, entries_.size());
            entries_.emplace_back(http_header{std::move(key_lower), std::string{value}});

            if (key_lower == "content-length") {
                const auto content_length = parse_int<std::size_t>(value);
                if (!content_length) {
                    throw std::runtime_error{"invalid request"};
                }

                content_length_ = *content_length;
            }
        }
    }

    void http_headers::remove(std::string_view key) {
        const auto key_lower = to_lower(key);
        if (const auto it = indices_.find(key_lower); it != std::end(indices_)) {
            const auto index = it->second;

            indices_.erase(it);
            entries_.erase(std::begin(entries_) + index);

            // Shift indices.
            for (auto& [key, i] : indices_) {
                if (i > index) {
                    i -= 1;
                }
            }
        }
    }
} // namespace mio
