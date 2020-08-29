#include "mio/http1/request_parser.hpp"

namespace mio::http1 {
    namespace {
        constexpr bool is_space(char c) noexcept {
            return c == 0x20;
        }

        constexpr bool is_control(char c) noexcept {
            return (unsigned char)c < 0x20 || c == 0x7f;
        }

        constexpr bool is_digit(char c) noexcept {
            return 0x30 <= c && c <= 0x39;
        }

        constexpr bool is_token(char c) noexcept {
            constexpr unsigned char table[128] = {
                // clang-format off
                /*       NUL SOH STX ETX EOT ENQ ACK BEL  BS  HT  LF  VT  FF  CR  SO  SI */
                /* 0X */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                /*       DLE DC1 DC2 DC3 DC4 NAK SYN ETB CAN  EM SUB ESC  FS  GS  RS  US */
                /* 1X */   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                /*        SP   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   / */
                /* 2X */   0,  1,  0,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  0,
                /*         0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ? */
                /* 3X */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,
                /*         @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O */
                /* 4X */   0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
                /*         P   Q   R   S   T   U   V   W   X   Y   Z   [  \\   ]   ^   _ */
                /* 5X */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  1,  1,
                /*         `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
                /* 6X */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
                /*         p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ DEL */
                /* 7X */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  0,  1,  0,
                // clang-format on
            };

            return (c & 0x80) == 0 && table[c & 0x7f] == 1;
        }

#define CHECK_RESULT(expr)                                                                              \
    if (const ::mio::http1::parse_result result = (expr); result != ::mio::http1::parse_result::done) { \
        return result;                                                                                  \
    }

#define IGNORE_RESULT(expr) \
    ((void)(expr))

#define CHECK_EOF(s, i)                                 \
    if ((i) >= (s).size()) {                            \
        return ::mio::http1::parse_result::in_progress; \
    }

        template <typename F>
        parse_result expect(F f, std::string_view s, std::size_t& i) {
            CHECK_EOF(s, i);
            if (!f(s[i])) {
                return parse_result::invalid;
            }

            i++;
            return parse_result::done;
        }

        template <typename F>
        parse_result expect_n(F f, std::string_view s, std::size_t& i) {
            const auto pos = i;

            for (;;) {
                CHECK_EOF(s, i);
                if (!f(s[i])) {
                    break;
                }
                i++;
            }

            if (i == pos) {
                return parse_result::invalid;
            }

            return parse_result::done;
        }

        parse_result expect_char(char c, std::string_view s, std::size_t& i) {
            CHECK_EOF(s, i);
            if (s[i] != c) {
                return parse_result::invalid;
            }

            i++;
            return parse_result::done;
        }

        parse_result expect_spaces(std::string_view s, std::size_t& i) {
            return expect_n(is_space, s, i);
        }

        parse_result expect_eol(std::string_view s, std::size_t& i) {
            CHECK_EOF(s, i + 1);
            if (s[i] != /*CR*/ 0x0d || s[i + 1] != /*LF*/ 0x0a) {
                return parse_result::invalid;
            }

            i += 2;
            return parse_result::done;
        }

        parse_result parse_token(std::string_view& token, std::string_view s, std::size_t& i) {
            const auto pos = i;
            CHECK_RESULT(expect_n(is_token, s, i));

            token = s.substr(pos, i - pos);
            return parse_result::done;
        }

        parse_result parse_request_uri(std::string_view& request_uri, std::string_view s, std::size_t& i) {
            constexpr auto is_uri = [](char c) noexcept {
                return !is_control(c) && !is_space(c);
            };

            const auto pos = i;
            CHECK_RESULT(expect_n(is_uri, s, i));

            request_uri = s.substr(pos, i - pos);
            return parse_result::done;
        }

        parse_result parse_http_version(std::string_view& http_version, std::string_view s, std::size_t& i) {
            const auto pos = i;
            CHECK_RESULT(expect_char(/*'H'*/ 0x48, s, i));
            CHECK_RESULT(expect_char(/*'T'*/ 0x54, s, i));
            CHECK_RESULT(expect_char(/*'T'*/ 0x54, s, i));
            CHECK_RESULT(expect_char(/*'P'*/ 0x50, s, i));
            CHECK_RESULT(expect_char(/*'/'*/ 0x2f, s, i));
            CHECK_RESULT(expect_char(/*'1'*/ 0x31, s, i));
            CHECK_RESULT(expect_char(/*'.'*/ 0x2e, s, i));
            CHECK_RESULT(expect_n(is_digit, s, i));

            http_version = s.substr(pos, i - pos);
            return parse_result::done;
        }

        parse_result parse_header_value(std::string_view& value, std::string_view s, std::size_t& i) {
            // Skip SPs.
            IGNORE_RESULT(expect_spaces(s, i));

            const auto pos = i;
            for (;;) {
                CHECK_EOF(s, i + 1);
                if (s[i] == /*CR*/ 0x0d && s[i + 1] == /*LF*/ 0x0a) {
                    value = s.substr(pos, i - pos);
                    return parse_result::done;
                }

                i++;
            }
        }

        parse_result parse_headers(std::span<header>& headers, std::span<header> buffer, std::string_view s, std::size_t& i) {
            for (std::size_t n = 0;; n++) {
                if (const auto result = expect_eol(s, i); result != parse_result::invalid) {
                    headers = buffer.subspan(0, n);
                    return result;
                }

                std::string_view key;
                std::string_view value;

                CHECK_RESULT(parse_token(key, s, i));
                IGNORE_RESULT(expect_spaces(s, i)); // Skip SPs.
                CHECK_RESULT(expect_char(/*':'*/ 0x3a, s, i));
                CHECK_RESULT(parse_header_value(value, s, i));
                CHECK_RESULT(expect_eol(s, i));

                if (n >= buffer.size()) {
                    return parse_result::too_many_headers;
                }

                buffer[n].key = key;
                buffer[n].value = value;
            }
        }

        parse_result parse_request(request& req, std::span<header> headers, std::string_view input, std::size_t& i) {
            CHECK_RESULT(parse_token(req.method, input, i));
            CHECK_RESULT(expect_spaces(input, i));
            CHECK_RESULT(parse_request_uri(req.request_uri, input, i));
            CHECK_RESULT(expect_spaces(input, i));
            CHECK_RESULT(parse_http_version(req.http_version, input, i));
            CHECK_RESULT(expect_eol(input, i));

            CHECK_RESULT(parse_headers(req.headers, headers, input, i));

            return parse_result::done;
        }
    } // namespace

    parse_result parse_request(request& req, std::span<header> headers, std::string_view input) {
        std::size_t pos = 0;
        return parse_request(req, headers, input, pos);
    }
} // namespace mio::http1
