#include "mio/application.hpp"

#include "mio/http_response.hpp"

namespace mio {
    http_response application_base::on_request(http_request& req) {
        return get_router().handle_request(req);
    }

    http_response application_base::on_error(const std::exception& e) noexcept {
        return http_response{
            500,
            http_headers{
                {"content-type", "text/html; charset=utf8"},
            },
            e.what(),
        };
    }

    http_response application_base::on_unknown_error() noexcept {
        return http_response{
            500,
            http_headers{
                {"content-type", "text/html; charset=utf8"},
            },
            "500 Internal Server Error",
        };
    }
} // namespace mio
