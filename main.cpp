#include <iostream>
#include <map>
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "calculator.h"

using json = nlohmann::json;

int main() {
    httplib::Server svr;

    // ✅ STATEFUL ОБЪЕКТЫ
    Calculator calc;
    std::map<std::string, double> variables;

    svr.Post("/calculate", [&](const httplib::Request &req, httplib::Response &res) {
        json response_json;

        try {
            json request_json = json::parse(req.body);

            // --- COMMANDS ---
            if (request_json.contains("cmd")) {
                std::string cmd = request_json["cmd"];

                if (cmd == "echo") {
                    response_json["res"] = "echo";
                }
                else if (cmd == "clean") {
                    variables.clear();               // ✅ СБРОС СОСТОЯНИЯ
                    response_json = json::object();  // {}
                }
                else {
                    throw std::runtime_error("Unknown command: " + cmd);
                }
            }
            // --- EXPRESSIONS ---
            else if (request_json.contains("exp")) {
                std::string expression = request_json["exp"];
                double result = calc.evaluate(expression, variables);
                response_json["res"] = result;
            }
            else {
                throw std::runtime_error("Invalid JSON: expected 'exp' or 'cmd'");
            }

            res.status = 200;
        }
        catch (const std::exception& e) {
            res.status = 400;
            response_json["err"] = e.what();
        }

        res.set_content(response_json.dump(), "application/json");
    });

    std::cout << "Server started on http://0.0.0.0:8080\n";
    svr.listen("0.0.0.0", 8080);
}
