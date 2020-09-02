#include "mio/http_headers.hpp"

#include <algorithm>

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
    } // namespace

    std::optional<std::string> http_headers::get(std::string_view key) const {
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
        }
    }

    void http_headers::append(std::string_view key, std::string_view value) {
        auto key_lower = to_lower(key);
        if (const auto it = indices_.find(key_lower); it != std::end(indices_)) {
            auto& entry = entries_[it->second];
            entry.value += ", ";
            entry.value += value;
        } else {
            indices_.emplace(key_lower, entries_.size());
            entries_.emplace_back(http_header{std::move(key_lower), std::string{value}});
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
