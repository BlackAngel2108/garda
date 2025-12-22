#include "gtest/gtest.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "calculator.h"
#include <thread>
#include <chrono>
#include <map> // Include for std::map

// Для удобства
using json = nlohmann::json;

// Test Fixture для интеграционных тестов сервера.
// Этот класс будет управлять запуском и остановкой сервера для каждого теста.
class ServerIntegrationTest : public ::testing::Test {
protected:
    // Эта функция будет вызываться перед каждым тестом
    void SetUp() override {
        // Находим свободный порт
        port = svr.bind_to_any_port("localhost");
        
        // Запускаем сервер в отдельном потоке
        server_thread = std::thread([this]() {
            Calculator calc; 

            svr.Post("/echo", [](const httplib::Request &req, httplib::Response &res) {
                res.set_content(req.body, "text/plain");
            });

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
                        } else if (cmd == "clean") { // Clear state. For now, clear local variables.
                            variables.clear(); // Clear variables for this request context.
                            response_json = json::object(); // Empty JSON response for clean command
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
            // --- Конец логики сервера ---

            svr.listen_after_bind();
        });

        // Даем серверу немного времени на запуск
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Эта функция будет вызываться после каждого теста
    void TearDown() override {
        svr.stop();
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }

    httplib::Server svr;
    // Calculator calc; // This is now created inside the thread
    std::thread server_thread;
    int port;
};

// Тест для эндпоинта /echo
TEST_F(ServerIntegrationTest, EchoTest) {
    httplib::Client cli("localhost", port);
    json req_json;
    req_json["cmd"] = "echo";
    auto res = cli.Post("/calculate", req_json.dump(), "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    
    json res_json = json::parse(res->body);
    EXPECT_TRUE(res_json.contains("res"));
    EXPECT_EQ(res_json["res"], "echo");
}

// Тест для эндпоинта /calculate с корректным выражением
TEST_F(ServerIntegrationTest, CalculateSuccess) {
    httplib::Client cli("localhost", port);
    json req_json;
    req_json["exp"] = " (10 + 2) * 5 / 2 - 10"; // 30 - 10 = 20
    
    auto res = cli.Post("/calculate", req_json.dump(), "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    
    json res_json = json::parse(res->body);
    EXPECT_TRUE(res_json.contains("res"));
    EXPECT_FALSE(res_json.contains("err"));
    EXPECT_DOUBLE_EQ(res_json["res"], 20.0);
}

// Тест для /calculate с ошибкой (деление на ноль)
TEST_F(ServerIntegrationTest, CalculateError) {
    httplib::Client cli("localhost", port);
    json req_json;
    req_json["exp"] = "10 / (5 - 5)";
    
    auto res = cli.Post("/calculate", req_json.dump(), "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400); // Ожидаем статус "Bad Request"
    
    json res_json = json::parse(res->body);
    EXPECT_TRUE(res_json.contains("err"));
    EXPECT_FALSE(res_json.contains("res"));
    EXPECT_EQ(res_json["err"], "Division by zero");
}


// --- New tests for Task 9: Variables ---

TEST_F(ServerIntegrationTest, CalculateVariableAssignment) {
    httplib::Client cli("localhost", port);
    json req_json;
    req_json["exp"] = "x = 10";
    auto res = cli.Post("/calculate", req_json.dump(), "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    json res_json = json::parse(res->body);
    EXPECT_TRUE(res_json.contains("res"));
    EXPECT_DOUBLE_EQ(res_json["res"], 10.0);
}

// This test demonstrates that variables are ephemeral per request for Task 9
// It will pass because the server context for each POST is fresh.
TEST_F(ServerIntegrationTest, CalculateVariableUsageEphemeral) {
    httplib::Client cli("localhost", port);
    json req_json;

    // First, try to assign a variable in one request (this does not persist)
    req_json["exp"] = "y = 20";
    auto res_assign = cli.Post("/calculate", req_json.dump(), "application/json");
    ASSERT_TRUE(res_assign);
    EXPECT_EQ(res_assign->status, 200);

    // Then, try to use it in a separate request (should fail as it's a new context)
    req_json["exp"] = "y * 2";
    auto res_use = cli.Post("/calculate", req_json.dump(), "application/json");

    ASSERT_TRUE(res_use);
    EXPECT_EQ(res_use->status, 400); // Expected to fail
    json res_json = json::parse(res_use->body);
    EXPECT_TRUE(res_json.contains("err"));
    EXPECT_EQ(res_json["err"], "Unknown variable: y");
}


TEST_F(ServerIntegrationTest, CalculateUnknownVariable) {
    httplib::Client cli("localhost", port);
    json req_json;
    req_json["exp"] = "z + 5";
    auto res = cli.Post("/calculate", req_json.dump(), "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400); // Expect bad request due to error
    json res_json = json::parse(res->body);
    EXPECT_TRUE(res_json.contains("err"));
    EXPECT_EQ(res_json["err"], "Unknown variable: z");
}

TEST_F(ServerIntegrationTest, CalculateMultiStatement) {
    httplib::Client cli("localhost", port);
    json req_json;
    req_json["exp"] = "var1 = 5; var2 = var1 * 2; var2 + 1";
    auto res = cli.Post("/calculate", req_json.dump(), "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    json res_json = json::parse(res->body);
    EXPECT_TRUE(res_json.contains("res"));
    EXPECT_DOUBLE_EQ(res_json["res"], 11.0);
}

TEST_F(ServerIntegrationTest, CleanCommand) {
    httplib::Client cli("localhost", port);
    json req_json;
    // The "clean" command clears variables *within the same request's context*.
    // Since each request gets a fresh 'variables' map for Task 9,
    // sending a clean command in one request doesn't affect subsequent requests in terms of state.
    // However, it should still return an empty JSON.
    req_json = json{{"cmd", "clean"}};
    auto res = cli.Post("/calculate", req_json.dump(), "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    json res_json = json::parse(res->body);
    EXPECT_TRUE(res_json.empty()); // Expect empty JSON for clean command
}

// Test for invalid variable name
TEST_F(ServerIntegrationTest, InvalidVariableName) {
    httplib::Client cli("localhost", port);
    json req_json;
    req_json["exp"] = "1var = 5"; // Invalid start character
    auto res = cli.Post("/calculate", req_json.dump(), "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400);
    json res_json = json::parse(res->body);
    EXPECT_TRUE(res_json.contains("err"));
    EXPECT_EQ(res_json["err"], "Invalid variable name: 1var");
}