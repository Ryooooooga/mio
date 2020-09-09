#include "mio/bodies/x_www_form_url_encoded.hpp"

#include "mio/http_request.hpp"
#include "mio/uri.hpp"

namespace mio::bodies {
    void parse_x_www_form_url_encoded(http_request& req) {
        const auto body = req.body_as_text();

        std::size_t pos = 0;
        std::size_t amp;
        do {
            amp = body.find('&', pos);
            const auto expr = body.substr(pos, amp);
            const auto sep = expr.find('=');

            const auto key = expr.substr(0, sep);
            const auto value = sep != std::string_view::npos ? expr.substr(sep + 1) : std::string_view{};

            auto decoded_key = decode_uri(key);
            auto decoded_value = decode_uri(value);

            if (!decoded_key || !decoded_value) {
                throw std::runtime_error{"bad request"};
            }

            req.set_form(std::move(*decoded_key), std::move(*decoded_value));

            pos = amp + 1;
        } while (amp != std::string_view::npos);
    }
} // namespace mio::bodies
