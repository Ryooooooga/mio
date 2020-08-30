#include "mio/http1/request.hpp"

// Ensure character encoding follows ASCII
static_assert('\t' == 0x09 && '\n' == 0x0a && '\r' == 0x0d);
static_assert(' ' == 0x20 && '!' == 0x21 && '\"' == 0x22 && '#' == 0x23 && '$' == 0x24 && '%' == 0x25 && '&' == 0x26 && '\'' == 0x27 && '(' == 0x28 && ')' == 0x29 && '*' == 0x2a && '+' == 0x2b && ',' == 0x2c && '-' == 0x2d && '.' == 0x2e && '/' == 0x2f);
static_assert('0' == 0x30 && '1' == 0x31 && '2' == 0x32 && '3' == 0x33 && '4' == 0x34 && '5' == 0x35 && '6' == 0x36 && '7' == 0x37 && '8' == 0x38 && '9' == 0x39 && ':' == 0x3a && ';' == 0x3b && '<' == 0x3c && '=' == 0x3d && '>' == 0x3e && '?' == 0x3f);
static_assert('@' == 0x40 && 'A' == 0x41 && 'B' == 0x42 && 'C' == 0x43 && 'D' == 0x44 && 'E' == 0x45 && 'F' == 0x46 && 'G' == 0x47 && 'H' == 0x48 && 'I' == 0x49 && 'J' == 0x4a && 'K' == 0x4b && 'L' == 0x4c && 'M' == 0x4d && 'N' == 0x4e && 'O' == 0x4f);
static_assert('P' == 0x50 && 'Q' == 0x51 && 'R' == 0x52 && 'S' == 0x53 && 'T' == 0x54 && 'U' == 0x55 && 'V' == 0x56 && 'W' == 0x57 && 'X' == 0x58 && 'Y' == 0x59 && 'Z' == 0x5a && '[' == 0x5b && '\\' == 0x5c && ']' == 0x5d && '^' == 0x5e && '_' == 0x5f);
static_assert('`' == 0x60 && 'a' == 0x61 && 'b' == 0x62 && 'c' == 0x63 && 'd' == 0x64 && 'e' == 0x65 && 'f' == 0x66 && 'g' == 0x67 && 'h' == 0x68 && 'i' == 0x69 && 'j' == 0x6a && 'k' == 0x6b && 'l' == 0x6c && 'm' == 0x6d && 'n' == 0x6e && 'o' == 0x6f);
static_assert('p' == 0x70 && 'q' == 0x71 && 'r' == 0x72 && 's' == 0x73 && 't' == 0x74 && 'u' == 0x75 && 'v' == 0x76 && 'w' == 0x77 && 'x' == 0x78 && 'y' == 0x79 && 'z' == 0x7a && '{' == 0x7b && '|' == 0x7c && '}' == 0x7d && '~' == 0x7e);

namespace mio::http1 {
    namespace {
        constexpr bool is_space(char c) noexcept {
            return c == ' ';
        }

        constexpr bool is_control(char c) noexcept {
            return (unsigned char)c < 0x20 || c == 0x7f;
        }

        constexpr bool is_digit(char c) noexcept {
            return '0' <= c && c <= '9';
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

#define CHECK_RESULT(expr)                                                                                   \
    if (const ::mio::http1::parse_result result = (expr); result != ::mio::http1::parse_result::completed) { \
        return result;                                                                                       \
    }

#define DISCARD_ERROR(expr)                                                                                    \
    if (const ::mio::http1::parse_result result = (expr); result == ::mio::http1::parse_result::in_progress) { \
        return result;                                                                                         \
    }

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
            return parse_result::completed;
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

            return parse_result::completed;
        }

        parse_result expect_char(char c, std::string_view s, std::size_t& i) {
            CHECK_EOF(s, i);
            if (s[i] != c) {
                return parse_result::invalid;
            }

            i++;
            return parse_result::completed;
        }

        parse_result expect_spaces(std::string_view s, std::size_t& i) {
            return expect_n(is_space, s, i);
        }

        parse_result expect_eol(std::string_view s, std::size_t& i) {
            CHECK_EOF(s, i + 1);
            if (s[i] != '\r' || s[i + 1] != '\n') {
                return parse_result::invalid;
            }

            i += 2;
            return parse_result::completed;
        }

        parse_result parse_token(std::string_view& token, std::string_view s, std::size_t& i) {
            const auto pos = i;
            CHECK_RESULT(expect_n(is_token, s, i));

            token = s.substr(pos, i - pos);
            return parse_result::completed;
        }

        parse_result parse_request_uri(std::string_view& request_uri, std::string_view s, std::size_t& i) {
            constexpr auto is_uri = [](char c) noexcept {
                return !is_control(c) && !is_space(c);
            };

            const auto pos = i;
            CHECK_RESULT(expect_n(is_uri, s, i));

            request_uri = s.substr(pos, i - pos);
            return parse_result::completed;
        }

        parse_result parse_http_version(std::string_view& http_version, std::string_view s, std::size_t& i) {
            const auto pos = i;
            CHECK_RESULT(expect_char('H', s, i));
            CHECK_RESULT(expect_char('T', s, i));
            CHECK_RESULT(expect_char('T', s, i));
            CHECK_RESULT(expect_char('P', s, i));
            CHECK_RESULT(expect_char('/', s, i));
            CHECK_RESULT(expect_char('1', s, i));
            CHECK_RESULT(expect_char('.', s, i));
            CHECK_RESULT(expect_n(is_digit, s, i));

            http_version = s.substr(pos, i - pos);
            return parse_result::completed;
        }

        parse_result parse_header_value(std::string_view& value, std::string_view s, std::size_t& i) {
            // Skip SPs.
            DISCARD_ERROR(expect_spaces(s, i));

            const auto pos = i;
            for (;;) {
                CHECK_EOF(s, i + 1);
                if (s[i] == '\r' && s[i + 1] == '\n') {
                    break;
                }

                i++;
            }

            value = s.substr(pos, i - pos);

            // Remove trailing SPs
            while (!value.empty() && value.back() == ' ') {
                value.remove_suffix(1);
            }

            return parse_result::completed;
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
                DISCARD_ERROR(expect_spaces(s, i)); // Skip SPs.
                CHECK_RESULT(expect_char(':', s, i));
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

            return parse_result::completed;
        }
    } // namespace

    parse_result parse_request(request& req, std::span<header> headers, std::string_view input) {
        std::size_t pos = 0;
        return parse_request(req, headers, input, pos);
    }
} // namespace mio::http1
