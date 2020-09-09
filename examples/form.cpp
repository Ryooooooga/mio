#include "mio/http_server.hpp"

#include <iostream>

#include "mio/application.hpp"
#include "mio/middlewares/static.hpp"

class application : public mio::application_base {
public:
    application() {
        use(mio::middlewares::static_{"./examples/form"});

        get_router().post("/", [](mio::http_request& req) {
            std::cout << "name: " << req.form("name").value_or("") << std::endl;
            std::cout << "content: " << req.form("content").value_or("") << std::endl;

            return mio::http_response{200, "OK"};
        });
    }
};

int main() {
    mio::http_server{std::make_unique<application>()}.listen(3000);
}
