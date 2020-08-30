#include "mio/sockets/socket.hpp"

#include <system_error>
#include <utility>

#include <netinet/in.h>
#include <unistd.h>

namespace mio::sockets {
    socket::socket(int fd) noexcept
        : fd_(fd) {
    }

    socket::socket(address_family family, socket_type type)
        : fd_(-1) {
        if ((fd_ = ::socket(static_cast<int>(family), static_cast<int>(type), 0)) < 0) {
            throw std::system_error{errno, std::generic_category()};
        }
    }

    socket::socket(socket&& socket) noexcept
        : fd_(std::exchange(socket.fd_, -1)) {
    }

    socket::~socket() {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    socket& socket::operator=(socket&& socket) {
        if (fd_ >= 0) {
            ::close(fd_);
        }

        fd_ = std::exchange(socket.fd_, -1);
        return *this;
    }

    int socket::set_reuse_addr(bool value) {
        const int reuseaddr = static_cast<int>(value);
        return ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));
    }

    void socket::listen(std::uint16_t port, int backlog) {
        struct sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        if (::bind(fd_, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)) < 0) {
            throw std::system_error{errno, std::generic_category()};
        }

        if (::listen(fd_, backlog) < 0) {
            throw std::system_error{errno, std::generic_category()};
        }
    }

    socket socket::accept() {
        const auto client_socket = ::accept(fd_, nullptr, nullptr);
        if (client_socket < 0) {
            throw std::system_error{errno, std::generic_category()};
        }

        return socket{client_socket};
    }

    std::size_t socket::receive(void* buffer, std::size_t size_bytes) {
        ::ssize_t size_recv;
        do {
            size_recv = ::recv(fd_, buffer, size_bytes, 0);
        } while (size_recv < 0 && errno == EINTR);

        return static_cast<std::size_t>(size_recv);
    }

    std::size_t socket::send(const void* data, std::size_t size_bytes) {
        ::ssize_t size_sent;
        do {
            size_sent = ::send(fd_, data, size_bytes, 0);
        } while (size_sent < 0 && errno == EINTR);

        return static_cast<std::size_t>(size_sent);
    }
} // namespace mio::sockets
