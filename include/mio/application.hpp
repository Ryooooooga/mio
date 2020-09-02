#ifndef INCLUDE_mio_application_hpp
#define INCLUDE_mio_application_hpp

#include "router.hpp"

namespace mio {
    class http_response;
    class http_request;
    class router;

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

        http_response on_request(http_request& req) override;

        http_response on_error(const std::exception& e) noexcept override;
        http_response on_unknown_error() noexcept override;

        router& get_router() noexcept {
            return router_;
        }

        const router& get_router() const noexcept {
            return router_;
        }

    private:
        router router_;

    private:
        // Uncopyable and unmovable
        application_base(const application_base&) = delete;
        application_base(application_base&&) = delete;

        application_base& operator=(const application_base&) = delete;
        application_base& operator=(application_base&&) = delete;
    };
} // namespace mio

#endif // INCLUDE_mio_application_hpp
