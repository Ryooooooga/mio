#ifndef INCLUDE_mio_bodies_x_www_form_url_encoded_hpp
#define INCLUDE_mio_bodies_x_www_form_url_encoded_hpp

namespace mio {
    class http_request;
}

namespace mio::bodies {
    void parse_x_www_form_url_encoded(http_request& req);
}

#endif // INCLUDE_mio_bodies_x_www_form_url_encoded_hpp
