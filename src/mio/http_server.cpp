#include "mio/http_server.hpp"

#include <iostream>
#include <sstream>
#include <system_error>
#include <thread>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "mio/http1/request.hpp"
#include "mio/http1/response.hpp"

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

        http1::response construct_http1_response(const http_response& from, std::span<http1::header> buffer) {
            http1::response res{};
            res.http_version = "HTTP/1.1";
            res.status_code = from.status_code();

            size_t n = 0;
            for (const auto& header_entries = from.headers().entries(); n < header_entries.size() && n < buffer.size(); n++) {
                buffer[n].key = header_entries[n].key;
                buffer[n].value = header_entries[n].value;
            }

            res.headers = buffer.subspan(0, n);
            res.content = from.content();
            return res;
        }

        void on_client_accepted(int client_socket, std::function<http_response(const http_request& req)> callback) noexcept {
            char buffer[max_header_size];
            http1::header headers[max_header_lines];
            http_response res{200};

            try {
                std::size_t header_len = 0;

                do {
                    ::ssize_t size_read;
                    do {
                        size_read = ::recv(client_socket, buffer + header_len, sizeof(char) * (max_header_size - header_len), 0);
                    } while (size_read < 0 && errno == EINTR);

                    if (size_read < 0) {
                        throw std::system_error{errno, std::generic_category()};
                    }

                    header_len += size_read;

                    http1::request http1_req{};
                    const auto parse_result = http1::parse_request(http1_req, headers, std::string_view{buffer, header_len});
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

                    const auto req = construct_request(http1_req);
                    res = callback(req);
                    break;
                } while (true);
            } catch (...) {
                res = http_response{
                    400,
                    http_headers{
                        {"Content-Type", "text/html; charset=utf8"},
                    },
                    "<html><body>400 Internal Server Error</body></html>"};
            }

            const auto http1_res = construct_http1_response(res, headers);

            try {
                std::ostringstream oss;
                http1::write_response(oss, http1_res);

                const auto s = oss.str();
                ::ssize_t size_sent;
                do {
                    size_sent = ::send(client_socket, s.data(), s.size(), 0);
                } while (size_sent < 0 && errno == EINTR);
            } catch (...) {
            }

            ::close(client_socket);
        }
    } // namespace

    void http_server::listen(std::uint16_t port, std::function<http_response(const http_request& req)> callback) {
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
