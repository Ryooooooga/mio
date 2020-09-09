#include "mio/router.hpp"

#include "mio/http_request.hpp"
#include "mio/http_response.hpp"
#include "mio/uri.hpp"

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
            if (const auto [it, inserted] = actions_.try_emplace(method, std::move(handler)); !inserted) {
                throw std::runtime_error{"routing is already registered"};
            }
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

    const request_handler* routing_tree::find(std::string_view path, const std::string& method, std::vector<std::pair<std::string_view, std::string>>& params) const {
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
            if (const auto handler = it->second->find(tail, method, params)) {
                return handler;
            }
        }

        for (const auto& child : wildcards_) {
            auto param = decode_uri(segment);
            if (!param) {
                throw std::runtime_error{"invalid request"};
            }

            params.emplace_back(*child->placeholder_, std::move(*param));

            if (const auto handler = child->find(tail, method, params)) {
                return handler;
            }

            params.pop_back();
        }

        return nullptr;
    }

    std::optional<http_response> router::handle_request(http_request& req) const {
        std::vector<std::pair<std::string_view, std::string>> params{};
        if (const auto handler = tree_.find(req.request_uri(), req.method(), params)) {
            for (auto&& [key, value] : params) {
                req.set_param(std::string{key}, std::move(value));
            }

            return (*handler)(req);
        }

        return std::nullopt;
    }
} // namespace mio
