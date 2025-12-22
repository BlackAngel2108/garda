#include <iostream>
#include <string>
#include <vector>
#include "httplib.h"
#include "nlohmann/json.hpp"

// For convenience
using json = nlohmann::json;

void print_help() {
    std::cout << "Usage: calc [OPTIONS]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -e <expression>  : Evaluate a mathematical expression (e.g., \"2 + 2\")" << std::endl;
    std::cout << "  -c <command>     : Execute a command (e.g., \"echo\", \"clean\")" << std::endl;
    std::cout << "  -s <address>     : Specify server address (default: http://garda_server:8080)" << std::endl; // New
    std::cout << "  -h, --help       : Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string server_url = "http://garda_server:8080"; // Default server URL
    std::string endpoint = "/calculate";
    
    json request_json;
    bool valid_args = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-e") {
            if (i + 1 < argc) {
                request_json["exp"] = argv[++i];
                valid_args = true;
            } else {
                std::cerr << "Error: -e requires an expression argument." << std::endl;
                print_help();
                return 1;
            }
        } else if (arg == "-c") {
            if (i + 1 < argc) {
                request_json["cmd"] = argv[++i];
                valid_args = true;
            } else {
                std::cerr << "Error: -c requires a command argument." << std::endl;
                print_help();
                return 1;
            }
        } else if (arg == "-s") { // New option to set server address
            if (i + 1 < argc) {
                server_url = argv[++i];
            } else {
                std::cerr << "Error: -s requires a server address argument." << std::endl;
                print_help();
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            print_help();
            return 0;
        } else {
            std::cerr << "Error: Unknown argument '" << arg << "'" << std::endl;
            print_help();
            return 1;
        }
    }

    if (!valid_args) {
        print_help();
        return 1;
    }

    // Send POST request
    httplib::Client cli(server_url.c_str()); // Use the potentially updated server_url
    if (auto res = cli.Post(endpoint.c_str(), request_json.dump(), "application/json")) {
        if (res->status == 200) {
            try {
                json response_json = json::parse(res->body);
                if (response_json.contains("res")) {
                    // Check if the result is a number or a string (e.g., "echo")
                    if (response_json["res"].is_number()) {
                        std::cout << response_json["res"].get<double>() << std::endl;
                    } else if (response_json["res"].is_string()) {
                        std::cout << response_json["res"].get<std::string>() << std::endl;
                    } else {
                        std::cout << response_json["res"] << std::endl; // Fallback for other types
                    }
                } else if (response_json.contains("err")) {
                    std::cerr << "Error from server: " << response_json["err"].get<std::string>() << std::endl;
                    return 1;
                } else {
                    // No 'res' or 'err' - might be for commands like 'clean' that return empty JSON
                    // Or for Level 9/10 requests that return empty JSON for variable assignment
                    std::cout << "Operation successful (no explicit result)." << std::endl;
                }
            } catch (const json::parse_error& e) {
                std::cerr << "Error parsing server response: " << e.what() << std::endl;
                std::cerr << "Response body: " << res->body << std::endl;
                return 1;
            }
        } else {
            std::cerr << "HTTP Error: " << res->status << " " << res->reason << std::endl;
            std::cerr << "Response body: " << res->body << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Error connecting to server or sending request: " << res.error() << std::endl;
        return 1;
    }

    return 0;
}
