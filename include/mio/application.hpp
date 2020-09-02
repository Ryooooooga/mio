#ifndef INCLUDE_mio_application_hpp
#define INCLUDE_mio_application_hpp

#include "router.hpp"

namespace mio {
    class http_response;
    class http_request;
    class router;

    using middleware = std::function<void(http_request&, http_response&)>;

    class application {
    public:
        application() = default;
        virtual ~application() noexcept = default;

        virtual http_response on_request(http_request& req) = 0;

        virtual http_response on_error(const std::exception& e) noexcept = 0;
        virtual http_response on_unknown_error() noexcept = 0;
    };

    class application_base : public application {
    public:
        application_base() = default;
        virtual ~application_base() noexcept = default;

        virtual http_response on_request(http_request& req) override;
        virtual http_response on_routing_not_found(http_request& req);

        virtual http_response on_error(const std::exception& e) noexcept override;
        virtual http_response on_unknown_error() noexcept override;

        router& get_router() noexcept {
            return router_;
        }

        const router& get_router() const noexcept {
            return router_;
        }

        void use(middleware&& middleware);

    private:
        router router_;
        std::vector<middleware> middlewares_;

    private:
        // Uncopyable and unmovable
        application_base(const application_base&) = delete;
        application_base(application_base&&) = delete;

        application_base& operator=(const application_base&) = delete;
        application_base& operator=(application_base&&) = delete;
    };
} // namespace mio

#endif // INCLUDE_mio_application_hpp
