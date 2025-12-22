#include "gtest/gtest.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "calculator.h"
#include <thread>
#include <chrono>

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
            // --- Логика сервера, которую мы тестируем ---
            svr.Post("/echo", [](const httplib::Request &req, httplib::Response &res) {
                res.set_content(req.body, "text/plain");
            });

            svr.Post("/calculate", [this](const httplib::Request &req, httplib::Response &res) {
                json response_json;
                try {
                    json request_json = json::parse(req.body);
                    if (request_json.contains("exp")) {
                        std::string expression = request_json["exp"];
                        double result = calc.evaluate(expression);
                        response_json["res"] = result;
                    } else if (request_json.contains("cmd") && request_json["cmd"] == "echo") {
                         response_json["res"] = "echo";
                    } else {
                        throw std::runtime_error("Invalid JSON format");
                    }
                } catch (const std::exception& e) {
                    res.status = 400; // Bad Request
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
    Calculator calc;
    std::thread server_thread;
    int port;
};

// Тест для эндпоинта /echo
TEST_F(ServerIntegrationTest, EchoTest) {
    httplib::Client cli("localhost", port);
    auto res = cli.Post("/echo", "hello", "text/plain");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "hello");
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

// Тест для команды "echo" в эндпоинте /calculate
TEST_F(ServerIntegrationTest, CalculateEchoCommand) {
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
