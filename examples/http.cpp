#include "mio/http_server.hpp"

#include "mio/application.hpp"

class application : public mio::application_base {
public:
    application() {
        auto& r = get_router();

        r.get("/", [](mio::http_request&) {
            return mio::http_response{
                200,
                mio::http_headers{
                    {"Content-Type", "text/html; charset=utf8"},
                },
                "<html>Hello, world!</html>",
            };
        });
    }
};

int main() {
    mio::http_server{std::make_unique<application>()}.listen(3000);
}
