#include <iostream>
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "calculator.h"


using json = nlohmann::json;

int main() {
    httplib::Server svr;
    Calculator calc; 

    
    svr.Post("/calculate", [&](const httplib::Request &req, httplib::Response &res) {
        json response_json;
        // Declare variables map here to be local to each request
        std::map<std::string, double> variables; 
        try {
            json request_json = json::parse(req.body);
            
            if (request_json.contains("cmd")) { // Handle commands first
                std::string cmd = request_json["cmd"];
                if (cmd == "echo") {
                    response_json["res"] = "echo";
                } else if (cmd == "clean") { // Clear state
                    variables.clear();
                    response_json = json::object();
                } else {
                    throw std::runtime_error("Unknown command: " + cmd);
                }
            } 
            else if (request_json.contains("exp")) {
                std::string expression = request_json["exp"];
                // Pass the variables map to evaluate
                double result = calc.evaluate(expression, variables); 
                response_json["res"] = result;
            } 
            else {
                throw std::runtime_error("Invalid JSON format: 'exp' or 'cmd' key is missing.");
            }
        } catch (const json::parse_error& e) {
            res.status = 400; 
            response_json["err"] = "Invalid JSON format: " + std::string(e.what());
        } catch (const std::exception& e) {
            res.status = 400; 
            response_json["err"] = e.what();
        }
        
        res.set_content(response_json.dump(), "application/json");
    });

    std::cout << "Server is starting on http://0.0.0.0:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}