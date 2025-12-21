#include <iostream>
#include <httplib.h>

int main() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request &req, httplib::Response &res) {
        res.set_content("<h1>Garda Echo Server</h1><p>Send a POST request to /echo to get your data back.</p>", "text/html");
    });

    svr.Post("/echo", [](const httplib::Request &req, httplib::Response &res) {
        std::cout << "Received echo request with body: " << req.body << std::endl;
        res.set_content(req.body, "text/plain");
    });

    std::cout << "Server is starting on http://0.0.0.0:8080" << std::endl;

    svr.listen("0.0.0.0", 8080);

    return 0;
}
