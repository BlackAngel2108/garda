#include "httplib.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

// For convenience
using json = nlohmann::json;

int main() {
    httplib::Server svr;

    svr.Post("/calculate", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json request_json = json::parse(req.body);
            
            if (request_json.contains("cmd") && request_json["cmd"].get<std::string>() == "echo") {
                res.set_content(json{{"res", "echo"}}.dump(), "application/json");
            } else if (request_json.contains("exp") && request_json["exp"].get<std::string>() == "2 + 2") {
                res.set_content(json{{"res", 4}}.dump(), "application/json");
            } else if (request_json.contains("exp") && request_json["exp"].get<std::string>() == "invalid expression") {
                res.set_content(json{{"err", "Invalid expression"}}.dump(), "application/json");
            } else {
                // Default response for other cases
                res.set_content(json{{"res", "default response"}}.dump(), "application/json");
            }
        } catch (const json::parse_error& e) {
            res.status = 400;
            res.set_content(json{{"err", "Invalid JSON in request body"}}.dump(), "application/json");
        }
    });

    std::cout << "Mock server listening on port 8081" << std::endl;
    svr.listen("0.0.0.0", 8081); // Listen on a different port than the main server (8080)

    return 0;
}
