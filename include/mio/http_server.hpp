#ifndef INCLUDE_mio_http_server_hpp
#define INCLUDE_mio_http_server_hpp

#include <cstdint>
#include <functional>
#include <memory>
#include "http_request.hpp"
#include "http_response.hpp"

namespace mio {
    class application;

    namespace sockets {
        class socket;
    } // namespace sockets

    class http_server {
    public:
        http_server(std::unique_ptr<application>&& app);
        ~http_server() noexcept;

        void listen(std::uint16_t port);

    private:
        static void on_client_accepted(http_server* self, sockets::socket client_socket) noexcept;

    private:
        std::unique_ptr<application> app_;

    private:
        // Uncopyable and unmovable
        http_server(const http_server&) = delete;
        http_server(http_server&&) = delete;

        http_server& operator=(const http_server&) = delete;
        http_server& operator=(http_server&&) = delete;
    };
} // namespace mio

#endif // INCLUDE_mio_http_server_hpp
