#include "mio/http_server.hpp"

#include <iostream>
#include <sstream>
#include <system_error>
#include <thread>

#include <netinet/in.h>
#include <unistd.h>

#include "mio/http1/request.hpp"
#include "mio/http1/response.hpp"
#include "mio/sockets/socket.hpp"

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

        void on_client_accepted(sockets::socket client_socket, std::function<http_response(const http_request& req)> callback) noexcept {
            char buffer[max_header_size];
            http1::header headers[max_header_lines];
            http_response res{500};

            try {
                std::size_t header_size = 0;
                http1::request http1_req{};

                do {
                    const auto size_read = client_socket.receive(buffer + header_size, sizeof(char) * (max_header_size - header_size));
                    header_size += size_read;

                    const auto parse_result = http1::parse_request(http1_req, headers, std::string_view{buffer, header_size});
                    if (parse_result == http1::parse_result::completed) {
                        break;
                    } else if (parse_result == http1::parse_result::in_progress && size_read > 0) {
                        continue;
                    }

                    throw std::runtime_error{"invalid request"};
                } while (true);

                const auto req = construct_request(http1_req);
                res = callback(req);
            } catch (...) {
                res = http_response{
                    500,
                    http_headers{
                        {"Content-Type", "text/html; charset=utf8"},
                    },
                    "<html><body>500 Internal Server Error</body></html>",
                };
            }

            try {
                const auto http1_res = construct_http1_response(res, headers);

                std::ostringstream oss;
                http1::write_response(oss, http1_res);

                const auto s = oss.str();
                client_socket.send(s.data(), s.size());
            } catch (...) {
            }
        }
    } // namespace

    void http_server::listen(std::uint16_t port, std::function<http_response(const http_request& req)> callback) {
        sockets::socket socket{sockets::address_family::inet, sockets::socket_type::stream};
        socket.set_reuse_addr(true);
        socket.listen(port);

        for (;;) {
            std::thread{on_client_accepted, socket.accept(), callback}.detach();
        }
    }
} // namespace mio
