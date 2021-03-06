#include "mio/router.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

#include "mio/http_request.hpp"
#include "mio/http_response.hpp"

namespace {
    void test_tree(const mio::routing_tree& tree, const std::string& path, const std::string& method, std::initializer_list<std::pair<std::string_view, std::string>> expected_params, std::string_view expected_body) {
        mio::http_headers headers{};
        mio::http_request req{method, path, "HTTP/1.1", std::move(headers)};
        std::vector<std::pair<std::string_view, std::string>> params{};

        const auto handler = tree.find(path, req.method(), params);
        if (handler == nullptr) {
            std::cerr << method << " " << path << ": not found" << std::endl;
            assert(handler != nullptr);
        }

        if (!std::ranges::equal(params, expected_params)) {
            std::cerr << method << " " << path << ": params not matched" << std::endl;
            assert(std::ranges::equal(params, expected_params));
        }

        const auto res = (*handler)(req);
        if (res.body_as_text() != expected_body) {
            std::cerr << method << " " << path << ": res.body_as_text() != " << expected_body << " (actual " << res.body_as_text() << ")" << std::endl;
            assert(res.body_as_text() == expected_body);
        }
    }

    void test_tree_not_found(const mio::routing_tree& tree, const std::string& path, const std::string& method) {
        std::vector<std::pair<std::string_view, std::string>> params{};

        const auto handler = tree.find(path, method, params);
        if (handler != nullptr) {
            std::cerr << method << " " << path << ": found" << std::endl;
            assert(handler == nullptr);
        }
    }

    void test_routing_tree() {
        {
            mio::routing_tree tree{"", std::nullopt};
            tree.insert("/", "GET", [](const mio::http_request&) { return mio::http_response{200, "GET /"}; });
            tree.insert("foo", "GET", [](const mio::http_request&) { return mio::http_response{200, "GET foo"}; });
            tree.insert("bar", "GET", [](const mio::http_request&) { return mio::http_response{200, "GET bar"}; });
            tree.insert("baz", "GET", [](const mio::http_request&) { return mio::http_response{200, "GET baz"}; });
            tree.insert("baz", "POST", [](const mio::http_request&) { return mio::http_response{200, "POST baz"}; });

            test_tree(tree, "/", "GET", {}, "GET /");
            test_tree(tree, "", "GET", {}, "GET /");
            test_tree(tree, "foo", "GET", {}, "GET foo");
            test_tree(tree, "bar", "GET", {}, "GET bar");
            test_tree(tree, "baz", "GET", {}, "GET baz");
            test_tree(tree, "baz", "POST", {}, "POST baz");

            test_tree(tree, "/foo", "GET", {}, "GET foo");
            test_tree(tree, "foo/", "GET", {}, "GET foo");
            test_tree(tree, "/foo/", "GET", {}, "GET foo");

            test_tree_not_found(tree, "foo//", "GET");
            test_tree_not_found(tree, "foo", "POST");
            test_tree_not_found(tree, "bar", "POST");
        }
        {
            mio::routing_tree tree{"", std::nullopt};
            tree.insert("foo", "GET", [](const mio::http_request&) { return mio::http_response{200, "GET foo"}; });
            tree.insert("foo/bar", "GET", [](const mio::http_request&) { return mio::http_response{200, "GET foo/bar"}; });
            tree.insert("baz/bar", "GET", [](const mio::http_request&) { return mio::http_response{200, "GET baz/bar"}; });
            tree.insert("baz/bar", "POST", [](const mio::http_request&) { return mio::http_response{200, "POST baz/bar"}; });

            test_tree(tree, "foo", "GET", {}, "GET foo");
            test_tree(tree, "foo/bar", "GET", {}, "GET foo/bar");
            test_tree(tree, "baz/bar", "GET", {}, "GET baz/bar");
            test_tree(tree, "baz/bar", "POST", {}, "POST baz/bar");

            test_tree(tree, "/foo/bar/", "GET", {}, "GET foo/bar");
            test_tree(tree, "/baz/bar", "POST", {}, "POST baz/bar");

            test_tree_not_found(tree, "baz", "GET");
            test_tree_not_found(tree, "baz", "POST");
            test_tree_not_found(tree, "foo//bar", "POST");
        }
        {
            mio::routing_tree tree{"", std::nullopt};
            tree.insert("foo/:id/a", "GET", [](const mio::http_request&) { return mio::http_response{200, "GET foo/:id/a"}; });
            tree.insert("foo/:XX/b", "GET", [](const mio::http_request&) { return mio::http_response{200, "GET foo/:XX/b"}; });

            test_tree(tree, "foo/0/a", "GET", {{"id", "0"}}, "GET foo/:id/a");
            test_tree(tree, "/foo/XXX/a", "GET", {{"id", "XXX"}}, "GET foo/:id/a");
            test_tree(tree, "foo/1/b", "GET", {{"XX", "1"}}, "GET foo/:XX/b");
            test_tree(tree, "/foo/YYY/b/", "GET", {{"XX", "YYY"}}, "GET foo/:XX/b");
            test_tree(tree, "/foo/a%20b/b/", "GET", {{"XX", "a b"}}, "GET foo/:XX/b");

            test_tree_not_found(tree, "foo", "GET");
            test_tree_not_found(tree, "foo/a", "GET");
            test_tree_not_found(tree, "foo/0/c", "GET");
            test_tree_not_found(tree, "bar/0/a", "GET");
        }
    }

    void test_request(const mio::router& router, std::string_view method, std::string_view path, std::string_view expected_body) {
        mio::http_headers headers{};
        mio::http_request req{method, path, "HTTP/1.1", std::move(headers)};

        const std::optional<mio::http_response> res = router.handle_request(req);
        if (!res) {
            std::cerr << method << " " << path << ": res == nullopt" << std::endl;
            assert(res);
        }

        if (res->body_as_text() != expected_body) {
            std::cerr << method << " " << path << ": res->body_as_text() != " << expected_body << " (actual " << res->body_as_text() << ")" << std::endl;
            assert(res->body_as_text() == expected_body);
        }
    }

    void test_request_not_found(const mio::router& router, std::string_view method, std::string_view path) {
        mio::http_headers headers{};
        mio::http_request req{method, path, "HTTP/1.1", std::move(headers)};

        const std::optional<mio::http_response> res = router.handle_request(req);
        if (res) {
            std::cerr << method << " " << path << ": res != nullopt (" << res->body_as_text() << ")" << std::endl;
            assert(!res);
        }
    }

    void test_router_() {
        mio::router router{};
        router.get("/", [](const mio::http_request&) { return mio::http_response{200, "GET /"}; });
        router.post("/", [](const mio::http_request&) { return mio::http_response{200, "POST /"}; });

        router.scope("/:id", [](auto& r) {
            r.get("/", [](const mio::http_request&) { return mio::http_response{200, "GET /:id/"}; });
            r.get("/foo", [](const mio::http_request&) { return mio::http_response{200, "GET /:id/foo"}; });
        });

        router.scope("foo/:bar/baz", [](auto& r) {
            r.get("/", [](const mio::http_request&) { return mio::http_response{200, "GET /foo/:bar/baz/"}; });
            r.get("/x", [](const mio::http_request&) { return mio::http_response{200, "GET /foo/:bar/baz/x"}; });
        });

        router.scope("/xxx", [](auto& r) {
            r.scope("/yyy", [](auto& r) {
                r.get("/zzz", [](const mio::http_request&) { return mio::http_response{200, "GET /xxx/yyy/zzz"}; });
            });
        });

        test_request(router, "GET", "/", "GET /");
        test_request(router, "GET", "/?name=xxx", "GET /");
        test_request(router, "POST", "/", "POST /");
        test_request(router, "GET", "/10", "GET /:id/");
        test_request(router, "GET", "/10/foo", "GET /:id/foo");
        test_request(router, "GET", "/foo/foo", "GET /:id/foo");
        test_request(router, "GET", "/foo/xxx/baz/", "GET /foo/:bar/baz/");
        test_request(router, "GET", "/foo/xxx/baz/x/", "GET /foo/:bar/baz/x");
        test_request(router, "GET", "/xxx/yyy/zzz", "GET /xxx/yyy/zzz");

        test_request_not_found(router, "PUT", "/");
        test_request_not_found(router, "GET", "/foo/bar");
        test_request_not_found(router, "GET", "/xxx/yyy");
    }
} // namespace

void test_router() {
    test_routing_tree();
    test_router_();
}
