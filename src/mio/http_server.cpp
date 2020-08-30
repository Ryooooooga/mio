#include "mio/http_server.hpp"

#include <system_error>
#include <thread>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "mio/http1/request_parser.hpp"

namespace mio {
    namespace {
        constexpr std::size_t max_header_size = 4096;
        constexpr std::size_t max_header_lines = 100;

        http_request construct_request(const http1::request& from) {
            http_headers headers{};
            for (const auto& header : from.headers) {
                headers.append(header.key, header.value);
            }

            return http_request{from.method, from.request_uri, from.http_version, std::move(headers)};
        }

        void on_client_accepted(int client_socket, std::function<void(const http_request& req)> callback) noexcept {
            try {
                char buffer[max_header_size];
                http1::header headers[max_header_lines];

                std::size_t header_len = 0;

                do {
                    const int size_read = ::recv(client_socket, buffer + header_len, sizeof(char) * (max_header_size - header_len), 0);
                    if (size_read < 0) {
                        throw std::system_error{errno, std::generic_category()};
                    }

                    header_len += size_read;

                    http1::request parsed_req{};
                    const auto parse_result = http1::parse_request(parsed_req, headers, std::string_view{buffer, header_len});
                    switch (parse_result) {
                        case http1::parse_result::done:
                            break;

                        case http1::parse_result::in_progress:
                            continue;

                        case http1::parse_result::too_many_headers:
                        case http1::parse_result::invalid:
                        default:
                            throw std::runtime_error{"invalid request"};
                    }

                    const auto req = construct_request(parsed_req);
                    callback(req);

                    ::send(client_socket, "OK\r\n", 2, 0);
                    break;
                } while (true);
            } catch (...) {
                ::send(client_socket, "server error\r\n", 14, 0);
            }

            ::close(client_socket);
        }
    } // namespace

    void http_server::listen(std::uint16_t port, std::function<void(const http_request& req)> callback) {
        const auto socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket < 0) {
            throw std::system_error{errno, std::generic_category()};
        }

        try {
            const int reuseaddr = 1;
            ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));

            struct sockaddr_in addr {};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(port);
            if (::bind(socket, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)) < 0) {
                throw std::system_error{errno, std::generic_category()};
            }

            const int backlog = 10;
            if (::listen(socket, backlog) < 0) {
                throw std::system_error{errno, std::generic_category()};
            }

            for (;;) {
                struct sockaddr_in client_addr;
                ::socklen_t client_addr_len = sizeof(client_addr);
                const auto client_socket = ::accept(socket, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
                if (client_socket < 0) {
                    throw std::system_error{errno, std::generic_category()};
                }

                std::thread{on_client_accepted, client_socket, callback}
                    .detach();
            }
        } catch (...) {
            ::close(socket);
            throw;
        }

        ::close(socket);
    }
} // namespace mio
