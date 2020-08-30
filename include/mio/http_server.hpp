#ifndef INCLUDE_mio_http_server_hpp
#define INCLUDE_mio_http_server_hpp

#include <cstdint>
#include <functional>

namespace mio {
    class http_server {
    public:
        http_server() = default;
        ~http_server() noexcept = default;

        void listen(std::uint16_t port, std::function<void()> callback);

    private:
        // Uncopyable and unmovable
        http_server(const http_server&) = delete;
        http_server(http_server&&) = delete;

        http_server& operator=(const http_server&) = delete;
        http_server& operator=(http_server&&) = delete;
    };
} // namespace mio

#endif // INCLUDE_mio_http_server_hpp
