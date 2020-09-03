#include "mio/http1/response.hpp"

namespace mio::http1 {
    void write_response(std::ostream& ostream, const response& res) {
        ostream << res.http_version << " " << res.status_code << " " << status_code_string(res.status_code) << "\r\n";

        for (const auto& header : res.headers) {
            ostream << header.key << ": " << header.value << "\r\n";
        }

        ostream << "content-length: " << res.body.size() << "\r\n";
        ostream << "\r\n";
        ostream.write(reinterpret_cast<const char*>(res.body.data()), res.body.size());
    }
} // namespace mio::http1
