#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <cstdlib> // For system()
#include <sstream> // For capturing stdout
#include <array>   // For std::array
#include <cstdio>  // For popen, pclose
#include <memory>  // For std::unique_ptr

// Helper function to execute a command and capture its output
std::string exec(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    // Use popen for POSIX compatibility (Linux, macOS)
    // Redirect stderr to stdout using "2>&1"
    std::string full_cmd = cmd + " 2>&1";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(full_cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Base server URL for the mock server
const std::string MOCK_SERVER_URL = "http://127.0.0.1:8081";
// Path to the calc_client executable relative to the build/bin directory
const std::string CALC_CLIENT_PATH = "./bin/calc_client"; 

TEST(ClientCLITests, EchoCommand) {
    std::string command = CALC_CLIENT_PATH + " -s " + MOCK_SERVER_URL + " -c echo";
    std::string output = exec(command);
    // Remove trailing newline for consistent comparison
    if (!output.empty() && output.back() == '\n') {
        output.pop_back();
    }
    EXPECT_EQ(output, "echo");
}

TEST(ClientCLITests, AddExpression) {
    std::string command = CALC_CLIENT_PATH + " -s " + MOCK_SERVER_URL + " -e \"2 + 2\"";
    std::string output = exec(command);
    if (!output.empty() && output.back() == '\n') {
        output.pop_back();
    }
    // Output should be "4"
    EXPECT_EQ(output, "4");
}

TEST(ClientCLITests, InvalidExpression) {
    std::string command = CALC_CLIENT_PATH + " -s " + MOCK_SERVER_URL + " -e \"invalid expression\"";
    std::string output = exec(command);
    if (!output.empty() && output.back() == '\n') {
        output.pop_back();
    }
    // Expected error from server
    EXPECT_TRUE(output.find("Error from server: Invalid expression") != std::string::npos);
}
