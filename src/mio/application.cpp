#include "mio/application.hpp"

#include "mio/http_response.hpp"

namespace mio {
    http_response application_base::on_request(http_request& req) {
        auto res = get_router().handle_request(req);
        if (!res) {
            res = on_routing_not_found(req);
        }

        for (const auto& middleware : middlewares_) {
            middleware(req, *res);
        }

        return std::move(*res);
    }

    http_response application_base::on_routing_not_found([[maybe_unused]] http_request& req) {
        return http_response{
            404,
            http_headers{
                {"content-type", "text/html; charset=utf8"},
            },
            "404 not found",
        };
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

    void application_base::use(middleware&& middleware) {
        middlewares_.emplace_back(std::move(middleware));
    }
} // namespace mio
