#include "mio/http_server.hpp"

#include <cassert>
#include <sstream>
#include <thread>

#include <netinet/in.h>
#include <unistd.h>

#include "mio/application.hpp"
#include "mio/http1/request.hpp"
#include "mio/http1/response.hpp"
#include "mio/sockets/socket.hpp"

namespace mio {
    namespace {
        constexpr std::size_t max_header_size = 4096;
        constexpr std::size_t max_header_lines = 100;

        http1::response convert_to_http1_response(const http_response& from, std::span<http1::header> buffer) {
            http1::response res{};
            res.http_version = "HTTP/1.1";
            res.status_code = from.status_code();

            size_t n = 0;
            for (const auto& header_entries = from.headers().entries(); n < header_entries.size() && n < buffer.size(); n++) {
                buffer[n].key = header_entries[n].key;
                buffer[n].value = header_entries[n].value;
            }

            res.headers = buffer.subspan(0, n);
            res.body = from.body();
            return res;
        }
    } // namespace

    http_server::http_server(std::unique_ptr<application>&& app)
        : app_(std::move(app)) {
        assert(app_);
    }

    http_server::~http_server() noexcept = default;

    void http_server::listen(std::uint16_t port) {
        sockets::socket socket{sockets::address_family::inet, sockets::socket_type::stream};
        socket.set_reuse_addr(true);
        socket.listen(port);

        for (;;) {
            std::thread{&http_server::on_client_accepted, this, socket.accept()}.detach();
        }
    }

    void http_server::on_client_accepted(http_server* self, sockets::socket client_socket) noexcept {
        const auto& app = self->app_;

        bool keep_alive;
        do {
            char buffer[max_header_size];
            http1::header headers[max_header_lines];
            http_response res{500};

            try {
                std::size_t pos = 0;
                std::size_t header_size;
                http1::request http1_req{};

                // TODO: too large request header
                for (;;) {
                    const auto size_read = client_socket.receive(buffer + pos, max_header_size - pos);
                    if (size_read == 0) {
                        return; // Connection closed.
                    }

                    pos += size_read;

                    const auto parse_result = http1::parse_request(http1_req, headers, std::string_view{buffer, pos}, header_size);
                    if (parse_result == http1::parse_result::completed) {
                        break;
                    } else if (parse_result == http1::parse_result::in_progress) {
                        continue;
                    } else {
                        throw std::runtime_error{"invalid request"};
                    }
                }

                http_headers headers{};
                for (const auto& header : http1_req.headers) {
                    headers.append(header.key, header.value);
                }

                std::vector<std::byte> body(reinterpret_cast<const std::byte*>(buffer + header_size), reinterpret_cast<const std::byte*>(buffer + pos));
                body.resize(headers.content_length());

                for (pos -= header_size; pos < headers.content_length();) {
                    const auto size_read = client_socket.receive(body.data() + pos, headers.content_length() - pos);
                    if (size_read == 0) {
                        return; // Connection closed.
                    }

                    pos += size_read;
                }

                http_request req{
                    http1_req.method,
                    http1_req.request_uri,
                    http1_req.http_version,
                    std::move(headers),
                    std::move(body),
                };

                keep_alive = req.headers().get("connection") == "keep-alive";
                req.headers().remove("connection");
                req.headers().remove("keep-alive");

                res = app->on_request(req);
            } catch (const std::exception& e) {
                res = app->on_error(e);
            } catch (...) {
                res = app->on_unknown_error();
            }

            try {
                res.headers().set("connection", keep_alive ? "keep-alive" : "close");

                const auto http1_res = convert_to_http1_response(res, headers);

                std::ostringstream oss;
                http1::write_response(oss, http1_res);

                const auto s = oss.str();
                client_socket.send(s.data(), s.size());
            } catch (...) {
            }
        } while (keep_alive);
    }
} // namespace mio
