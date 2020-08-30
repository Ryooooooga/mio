#include "mio/router.hpp"

namespace mio {
    routing_tree::routing_tree(std::optional<std::string>&& placeholder)
        : children_()
        , wildcards_()
        , actions_()
        , placeholder_(std::move(placeholder)) {
    }

    void routing_tree::insert(const std::string& path, const std::string& method, request_handler&& handler) {
        std::string_view p = path;

        // Skips over the first '/'.
        // "/foo/bar" -> "foo/bar"
        if (p.starts_with('/')) {
            p.remove_prefix(1);
        }

        routing_tree* node = this;
        while (!p.empty()) {
            const auto sep_index = p.find('/');
            const std::string segment{p.substr(0, sep_index)};

            p.remove_prefix(sep_index == std::string_view::npos ? p.size() : sep_index + 1);

            if (!segment.starts_with(':')) {
                if (const auto it = node->children_.find(segment); it != std::end(node->children_)) {
                    node = it->second.get();
                } else {
                    node = node->children_.emplace(segment, std::make_unique<routing_tree>()).first->second.get();
                }
            } else {
                const auto placeholder = std::string_view{segment}.substr(1);

                routing_tree* next = nullptr;
                for (const auto& child : node->wildcards_) {
                    if (child->placeholder_ == placeholder) {
                        next = child.get();
                        break;
                    }
                }

                if (next == nullptr) {
                    node->wildcards_.emplace_back(std::make_unique<routing_tree>(std::string{placeholder}));
                    next = node->wildcards_.back().get();
                }

                node = next;
            }
        }

        // Register the request handler.
        if (const auto it = node->actions_.find(method); it != std::end(node->actions_)) {
            throw std::runtime_error{"routing is already registered: " + path};
        }

        node->actions_[method] = std::move(handler);
    }

    const request_handler* routing_tree::find(const std::string& path, const std::string& method) const {
        std::string_view p = path;

        // Skips over the first '/'.
        // "/foo/bar" -> "foo/bar"
        if (p.starts_with('/')) {
            p.remove_prefix(1);
        }

        const routing_tree* node = this;
        while (!p.empty()) {
            const auto sep_index = p.find('/');
            const std::string segment{p.substr(0, sep_index)};

            p.remove_prefix(sep_index == std::string_view::npos ? p.size() : sep_index + 1);

            if (const auto it = node->children_.find(segment); it != std::end(node->children_)) {
                node = it->second.get();
            } else {
                for (const auto& child : node->wildcards_) {
                    if (const auto handler = child->find(std::string{p}, method)) {
                        return handler;
                    }
                }

                return nullptr;
            }
        }

        if (const auto it = node->actions_.find(method); it != std::end(node->actions_)) {
            return &it->second;
        }

        return nullptr;
    }
} // namespace mio
