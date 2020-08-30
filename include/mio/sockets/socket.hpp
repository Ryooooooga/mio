#ifndef INCLUDE_mio_sockets_socket_hpp
#define INCLUDE_mio_sockets_socket_hpp

#include <cstddef>
#include <cstdint>

#include <sys/socket.h>

namespace mio::sockets {
    enum class address_family : int {
        inet = AF_INET,
    };

    enum class socket_type : int {
        stream = SOCK_STREAM,
    };

    class socket {
    private:
        explicit socket(int fd) noexcept;

    public:
        socket(address_family family, socket_type type);
        ~socket() noexcept;

        // Movable
        socket(socket&& socket) noexcept;
        socket& operator=(socket&& socket);

        int set_reuse_addr(bool value);

        void listen(std::uint16_t port, int backlog = 10);
        socket accept();

        std::size_t receive(void* buffer, std::size_t size_bytes);
        std::size_t send(const void* data, std::size_t size_bytes);

        [[nodiscard]] int descriptor() const noexcept {
            return fd_;
        }

    private:
        int fd_;

    private:
        // Uncopyable
        socket(const socket&) = delete;
        socket& operator=(const socket&) = delete;
    };
} // namespace mio::sockets

#endif // INCLUDE_mio_sockets_socket_hpp
