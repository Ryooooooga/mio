#ifndef INCLUDE_mio_middlewares_static_hpp
#define INCLUDE_mio_middlewares_static_hpp

#include <filesystem>
#include <string_view>

namespace mio {
    class http_request;
    class http_response;
} // namespace mio

namespace mio::middlewares {
    class static_ {
    public:
        explicit static_(const std::filesystem::path& path, std::string_view base_uri = "/", bool show_hidden_files = false);

        // Copyable and movable
        static_(const static_&) = default;
        static_(static_&&) = default;

        static_& operator=(const static_&) = default;
        static_& operator=(static_&&) = default;

        ~static_() noexcept = default;

        void operator()(http_request& req, http_response& res) const;

    private:
        std::filesystem::path path_;
        std::string base_uri_;
        bool show_hidden_files_;
    };
} // namespace mio::middlewares

#endif // INCLUDE_mio_middlewares_static_hpp
