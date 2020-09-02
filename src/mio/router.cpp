#include "mio/router.hpp"

#include "mio/http_request.hpp"
#include "mio/http_response.hpp"

namespace mio {
    routing_tree::routing_tree(std::string_view name, std::optional<std::string>&& placeholder)
        : name_(name)
        , children_()
        , wildcards_()
        , actions_()
        , placeholder_(std::move(placeholder)) {
    }

    void routing_tree::insert(std::string_view path, const std::string& method, request_handler&& handler) {
        // Skips over the first '/'.
        // "/foo/bar" -> "foo/bar"
        if (path.starts_with('/')) {
            path.remove_prefix(1);
        }

        if (path.empty()) {
            // Register the request handler.
            if (const auto it = actions_.find(method); it != std::end(actions_)) {
                throw std::runtime_error{"routing is already registered"};
            }

            actions_[method] = std::move(handler);
            return;
        }

        const auto sep_index = path.find('/');
        const auto segment = path.substr(0, sep_index);
        const auto tail = path.substr(segment.size());

        if (!segment.starts_with(':')) {
            auto it = children_.find(segment);
            if (it == std::end(children_)) {
                auto child = std::make_unique<routing_tree>(segment, std::nullopt);
                it = children_.emplace(std::string_view{child->name_}, std::move(child)).first;
            }

            it->second->insert(tail, method, std::move(handler));
        } else {
            const auto placeholder = std::string_view{segment}.substr(1);

            routing_tree* node = nullptr;
            for (const auto& child : wildcards_) {
                if (child->placeholder_ == placeholder) {
                    node = child.get();
                    break;
                }
            }

            if (node == nullptr) {
                wildcards_.emplace_back(std::make_unique<routing_tree>(segment, std::string{placeholder}));
                node = wildcards_.back().get();
            }

            node->insert(tail, method, std::move(handler));
        }
    }

    const request_handler* routing_tree::find(std::string_view path, const std::string& method) const {
        // Skips over the first '/'.
        // "/foo/bar" -> "foo/bar"
        if (path.starts_with('/')) {
            path.remove_prefix(1);
        }

        if (path.empty()) {
            if (const auto it = actions_.find(method); it != std::end(actions_)) {
                return &it->second;
            }
            return nullptr;
        }

        const auto sep_index = path.find('/');
        const auto segment = path.substr(0, sep_index);
        const auto tail = path.substr(segment.size());

        if (const auto it = children_.find(segment); it != std::end(children_)) {
            if (const auto handler = it->second->find(tail, method)) {
                return handler;
            }
        }

        for (const auto& child : wildcards_) {
            if (const auto handler = child->find(tail, method)) {
                return handler;
            }
        }

        return nullptr;
    }

    http_response router::handle_request(http_request& req) const {
        if (const auto handler = tree_.find(req.request_uri(), req.method())) {
            return (*handler)(req);
        }

        return http_response{
            404,
            http_headers{
                {"content-type", "text/html; charset=utf-8"},
            },
            req.request_uri() + " not found",
        };
    }
} // namespace mio
