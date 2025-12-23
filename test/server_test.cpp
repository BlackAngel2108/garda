#include "gtest/gtest.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "calculator.h"
#include <thread>
#include <chrono>
#include <map> // Include for std::map

// Для удобства
using json = nlohmann::json;

class ServerIntegrationTest : public ::testing::Test {
protected:
    // Эта функция будет вызываться перед каждым тестом
    void SetUp() override {
        port = svr.bind_to_any_port("localhost");

        server_thread = std::thread([this]() {
            Calculator calculator;

            // 🔥 LEVEL 11: sessions storage
            std::unordered_map<
                std::string,
                std::map<std::string, double>
            > sessions;

            svr.Post("/calculate", [&](const httplib::Request &req,
                                    httplib::Response &res) {
                json res_json;

                try {
                    json req_json = json::parse(req.body);

                    // --- SID handling ---
                    std::string sid = "default";
                    if (req_json.contains("sid") && req_json["sid"].is_string()) {
                        sid = req_json["sid"];
                    }

                    // get session variables (auto-create)
                    auto& vars = sessions[sid];

                    // --- command ---
                    if (req_json.contains("cmd")) {
                        std::string cmd = req_json["cmd"];

                        if (cmd == "echo") {
                            res_json["res"] = "echo";
                        }
                        else if (cmd == "clean") {
                            vars.clear();
                            res_json = json::object();
                        }
                        else {
                            throw std::runtime_error("Unknown command: " + cmd);
                        }
                    }
                    // --- expression ---
                    else if (req_json.contains("exp")) {
                        double result = calculator.evaluate(req_json["exp"], vars);
                        res_json["res"] = result;
                    }
                    else {
                        throw std::runtime_error("Invalid JSON format");
                    }

                    res.status = 200;
                }
                catch (const std::exception& e) {
                    res.status = 400;
                    res_json["err"] = e.what();
                }

                res.set_content(res_json.dump(), "application/json");
            });

            svr.listen_after_bind();
        });

        // give server time to start
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

// ---- Level 10 ----
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

TEST_F(ServerIntegrationTest, StatefulVariablesPersist) {
    httplib::Client cli("localhost", port);

    cli.Post("/calculate", R"({"exp":"pi=3.14"})", "application/json");
    auto res = cli.Post("/calculate", R"({"exp":"2*pi*3"})", "application/json");

    ASSERT_EQ(res->status, 200);
    json j = json::parse(res->body);
    EXPECT_DOUBLE_EQ(j["res"], 18.84);
}

TEST_F(ServerIntegrationTest, CleanClearsState) {
    httplib::Client cli("localhost", port);

    cli.Post("/calculate", R"({"exp":"x=5"})", "application/json");
    cli.Post("/calculate", R"({"cmd":"clean"})", "application/json");

    auto res = cli.Post("/calculate", R"({"exp":"x+1"})", "application/json");

    ASSERT_EQ(res->status, 400);
    json j = json::parse(res->body);
    EXPECT_EQ(j["err"], "Unknown variable: x");
}


// ---- Level 11 ----

TEST_F(ServerIntegrationTest, SessionKeepsState) {
    httplib::Client cli("localhost", port);

    cli.Post("/calculate",
             R"({"sid":"A","exp":"x=10"})",
             "application/json");

    auto res = cli.Post("/calculate",
                        R"({"sid":"A","exp":"x*2"})",
                        "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    json j = json::parse(res->body);
    EXPECT_DOUBLE_EQ(j["res"], 20.0);
}

TEST_F(ServerIntegrationTest, SessionsAreIsolated) {
    httplib::Client cli("localhost", port);

    cli.Post("/calculate",
             R"({"sid":"A","exp":"x=5"})",
             "application/json");

    auto res = cli.Post("/calculate",
                        R"({"sid":"B","exp":"x+1"})",
                        "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400);

    json j = json::parse(res->body);
    EXPECT_EQ(j["err"], "Unknown variable: x");
}

TEST_F(ServerIntegrationTest, CleanClearsOnlyOneSession) {
    httplib::Client cli("localhost", port);

    cli.Post("/calculate",
             R"({"sid":"A","exp":"x=7"})",
             "application/json");

    cli.Post("/calculate",
             R"({"sid":"B","exp":"x=9"})",
             "application/json");

    cli.Post("/calculate",
             R"({"sid":"A","cmd":"clean"})",
             "application/json");

    // A → очищена
    auto resA = cli.Post("/calculate",
                          R"({"sid":"A","exp":"x+1"})",
                          "application/json");

    EXPECT_EQ(resA->status, 400);

    // B → не затронута
    auto resB = cli.Post("/calculate",
                          R"({"sid":"B","exp":"x+1"})",
                          "application/json");

    ASSERT_EQ(resB->status, 200);
    json j = json::parse(resB->body);
    EXPECT_DOUBLE_EQ(j["res"], 10.0);
}
