#include "mio/middlewares/static.hpp"

#include <fstream>
#include "mio/http_request.hpp"
#include "mio/http_response.hpp"

namespace mio::middlewares {
    namespace {
        const std::unordered_map<std::string, std::string> known_content_types = {
            {".txt", "text/plain; charset=utf-8"},
            {".html", "text/html; charset=utf-8"},
            {".css", "text/css; charset=utf-8"},
            {".js", "text/javascript; charset=utf-8"},
            {".json", "application/json; charset=utf-8"},
            {".ico", "image/x-icon"},
        };

        const std::string default_content_type = "application/octet-stream";

        std::string_view estimate_content_type(const std::filesystem::path& path) {
            if (const auto it = known_content_types.find(path.extension().string()); it != std::end(known_content_types)) {
                return it->second;
            }
            return default_content_type;
        }

        std::optional<std::string> read_file(const std::filesystem::path& path) {
            if (!std::filesystem::is_regular_file(path)) {
                return std::nullopt;
            }

            std::ifstream ifs{path, std::ios::binary};
            if (!ifs) {
                return std::nullopt;
            }

            return std::string(std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{});
        }
    } // namespace

    static_::static_(const std::filesystem::path& path, std::string_view base_uri, bool show_hidden_files)
        : path_(std::filesystem::weakly_canonical(path))
        , base_uri_(base_uri)
        , show_hidden_files_(show_hidden_files) {
        if (!base_uri_.starts_with('/')) {
            base_uri_.insert(0, 1, '/');
        }
        if (!base_uri_.ends_with('/')) {
            base_uri_.push_back('/');
        }
    }

    void static_::operator()(http_request& req, http_response& res) const {
        if ((req.method() != "GET" && req.method() != "HEAD") || res.status_code() != 404) {
            return;
        }

        std::string_view base = base_uri_;
        base.remove_suffix(1);
        // TODO: base = "/assets"

        std::string_view path = req.path();
        if (!path.starts_with(base)) {
            return;
        }

        path.remove_prefix(base.size());
        if (!path.empty() && !path.starts_with('/')) {
            return;
        }

        // path == "" or path == "/XXX"
        auto file_path = path_;
        file_path.concat(path); // not path::append()
        file_path = file_path.lexically_normal();

        if (!file_path.native().starts_with(path_.native())) {
            return;
        }

        if (std::filesystem::is_directory(file_path)) {
            file_path /= "index.html";
        }

        const auto content = read_file(file_path);
        if (!content) {
            return;
        }

        res.set_status_code(200);
        res.headers().set("content-type", estimate_content_type(file_path));
        res.body(*content);
    }
} // namespace mio::middlewares
