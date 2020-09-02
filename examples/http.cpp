#include "mio/http_server.hpp"

#include "mio/application.hpp"
#include "mio/middlewares/static.hpp"

class application : public mio::application_base {
public:
    application() {
        use(mio::middlewares::static_{"./examples/static"});
    }
};

int main() {
    mio::http_server{std::make_unique<application>()}.listen(3000);
}
