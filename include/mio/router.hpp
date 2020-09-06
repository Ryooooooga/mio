#ifndef INCLUDE_mio_router_hpp
#define INCLUDE_mio_router_hpp

#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mio {
    class http_request;
    class http_response;
    class router;

    using request_handler = std::function<http_response(http_request&)>;

    class routing_tree {
    public:
        routing_tree(std::string_view name, std::optional<std::string>&& placeholder);
        ~routing_tree() noexcept = default;

        void insert(std::string_view path, const std::string& method, request_handler&& handler);

        const request_handler* find(std::string_view path, const std::string& method, std::vector<std::pair<std::string_view, std::string_view>>& params) const;

    private:
        std::string name_;
        std::unordered_map<std::string_view, std::unique_ptr<routing_tree>> children_;
        std::vector<std::unique_ptr<routing_tree>> wildcards_;
        std::unordered_map<std::string, request_handler> actions_;
        std::optional<std::string> placeholder_;

    private:
        // Uncopyable and unmovable
        routing_tree(const routing_tree&) = delete;
        routing_tree(routing_tree&&) = delete;

        routing_tree& operator=(const routing_tree&) = delete;
        routing_tree& operator=(routing_tree&&) = delete;
    };

    class scope_inserter {
        friend class router;

    private:
        explicit scope_inserter(router& router, std::string_view prefix)
            : router_(router)
            , prefix_(prefix) {
            if (!prefix_.ends_with('/')) {
                prefix_ += '/';
            }
        }

        ~scope_inserter() noexcept = default;

    public:
        void add(std::string_view path, std::string_view method, request_handler&& handler);

        void get(std::string_view path, request_handler&& handler) {
            add(path, "GET", std::move(handler));
        }

        void post(std::string_view path, request_handler&& handler) {
            add(path, "POST", std::move(handler));
        }

        void put(std::string_view path, request_handler&& handler) {
            add(path, "PUT", std::move(handler));
        }

        void delete_(std::string_view path, request_handler&& handler) {
            add(path, "DELETE", std::move(handler));
        }

        template <std::invocable<scope_inserter&> F>
        void scope(std::string_view path, F f) {
            std::string prefix = prefix_;
            prefix += path.starts_with('/') ? path.substr(1) : path;

            scope_inserter scope{router_, prefix};
            std::invoke(f, scope);
        }

    private:
        router& router_;
        std::string prefix_;

    private:
        // Uncopyable and unmovable
        scope_inserter(const scope_inserter&) = delete;
        scope_inserter(scope_inserter&&) = delete;

        scope_inserter& operator=(const scope_inserter&) = delete;
        scope_inserter& operator=(scope_inserter&&) = delete;
    };

    class router {
    public:
        router()
            : tree_("", std::nullopt) {
        }

        ~router() noexcept = default;

        void add(std::string_view path, std::string_view method, request_handler&& handler) {
            tree_.insert(path, std::string{method}, std::move(handler));
        }

        void get(std::string_view path, request_handler&& handler) {
            add(path, "GET", std::move(handler));
        }

        void post(std::string_view path, request_handler&& handler) {
            add(path, "POST", std::move(handler));
        }

        void put(std::string_view path, request_handler&& handler) {
            add(path, "PUT", std::move(handler));
        }

        void delete_(std::string_view path, request_handler&& handler) {
            add(path, "DELETE", std::move(handler));
        }

        template <std::invocable<scope_inserter&> F>
        void scope(std::string_view path, F f) {
            scope_inserter scope{*this, path};
            std::invoke(f, scope);
        }

        std::optional<http_response> handle_request(http_request& req) const;

    private:
        routing_tree tree_;

    private:
        // Uncopyable and unmovable
        router(const router&) = delete;
        router(router&&) = delete;

        router& operator=(const router&) = delete;
        router& operator=(router&&) = delete;
    };

    inline void scope_inserter::add(std::string_view path, std::string_view method, request_handler&& handler) {
        std::string full_path = prefix_;
        full_path += path.starts_with('/') ? path.substr(1) : path;

        router_.add(full_path, method, std::move(handler));
    }
} // namespace mio

#endif // INCLUDE_mio_router_hpp
