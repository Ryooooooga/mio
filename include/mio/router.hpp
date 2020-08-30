#ifndef INCLUDE_mio_router_hpp
#define INCLUDE_mio_router_hpp

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mio {
    class http_request;
    class http_response;

    using request_handler = std::function<http_response(const http_request&)>;

    class routing_tree {
    public:
        explicit routing_tree(std::optional<std::string>&& placeholder = std::nullopt);
        ~routing_tree() noexcept = default;

        void insert(const std::string& path, const std::string& method, request_handler&& handler);

        const request_handler* find(const std::string& path, const std::string& method) const;

    private:
        std::unordered_map<std::string, std::unique_ptr<routing_tree>> children_;
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

    class router {
    public:
    private:
        routing_tree tree_;

    private:
        // Uncopyable and unmovable
        router(const router&) = delete;
        router(router&&) = delete;

        router& operator=(const router&) = delete;
        router& operator=(router&&) = delete;
    };
} // namespace mio

#endif // INCLUDE_mio_router_hpp
