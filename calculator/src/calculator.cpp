#include "calculator.h"
#include <stack>
#include <sstream>
#include <cmath>
#include <cctype>
#include <iostream>

double Calculator::evaluate(const std::string& expression) {
    auto tokens = tokenize(expression);
    auto rpnTokens = shuntingYard(tokens);
    return evaluateRPN(rpnTokens);
}

std::vector<Calculator::Token> Calculator::tokenize(const std::string& expression) {
    std::vector<Token> tokens;
    for (size_t i = 0; i < expression.length(); ++i) {
        char c = expression[i];

        if (isspace(c)) {
            continue;
        }
        else if (isdigit(c) || (c == '.')) {
            std::string num_str;
            while (i < expression.length() && (isdigit(expression[i]) || expression[i] == '.')) {
                num_str += expression[i];
                i++;
            }
            i--;
            tokens.push_back({TokenType::NUMBER, num_str});
        }
        else if (isOperator(c)) {
            tokens.push_back({TokenType::OPERATOR, std::string(1, c), getPrecedence(c), isLeftAssociative(c)});
        }
        else if (c == '(') {
            tokens.push_back({TokenType::LEFT_PAREN, "("});
        }
        else if (c == ')') {
            tokens.push_back({TokenType::RIGHT_PAREN, ")"});
        }
        else {
            throw std::runtime_error("Invalid character in expression: " + std::string(1, c));
        }
    }
    return tokens;
}

std::vector<Calculator::Token> Calculator::shuntingYard(const std::vector<Token>& tokens) {
    std::vector<Token> output_queue;
    std::stack<Token> operator_stack;

    for (const auto& token : tokens) {
        switch (token.type) {
            case TokenType::NUMBER:
                output_queue.push_back(token);
                break;
            case TokenType::OPERATOR:
                while (!operator_stack.empty() && operator_stack.top().type == TokenType::OPERATOR &&
                       ((operator_stack.top().isLeftAssociative && operator_stack.top().precedence >= token.precedence) ||
                        (!operator_stack.top().isLeftAssociative && operator_stack.top().precedence > token.precedence))) {
                    output_queue.push_back(operator_stack.top());
                    operator_stack.pop();
                }
                operator_stack.push(token);
                break;
            case TokenType::LEFT_PAREN:
                operator_stack.push(token);
                break;
            case TokenType::RIGHT_PAREN:
                while (!operator_stack.empty() && operator_stack.top().type != TokenType::LEFT_PAREN) {
                    output_queue.push_back(operator_stack.top());
                    operator_stack.pop();
                }
                if (operator_stack.empty()) {
                    throw std::runtime_error("Mismatched parentheses");
                }
                operator_stack.pop();
                break;
        }
    }

    while (!operator_stack.empty()) {
        if (operator_stack.top().type == TokenType::LEFT_PAREN) {
            throw std::runtime_error("Mismatched parentheses");
        }
        output_queue.push_back(operator_stack.top());
        operator_stack.pop();
    }

    return output_queue;
}

double Calculator::evaluateRPN(std::vector<Token> rpnTokens) {
    std::stack<double> value_stack;

    for (const auto& token : rpnTokens) {
        if (token.type == TokenType::NUMBER) {
            value_stack.push(std::stod(token.value));
        }
        else if (token.type == TokenType::OPERATOR) {
            if (value_stack.size() < 2) {
                throw std::runtime_error("Invalid expression: not enough operands for operator");
            }
            double right = value_stack.top();
            value_stack.pop();
            double left = value_stack.top();
            value_stack.pop();

            switch (token.value[0]) {
                case '+': value_stack.push(left + right); break;
                case '-': value_stack.push(left - right); break;
                case '*': value_stack.push(left * right); break;
                case '/':
                    if (right == 0) throw std::runtime_error("Division by zero");
                    value_stack.push(left / right);
                    break;
            }
        }
    }

    if (value_stack.size() != 1) {
        throw std::runtime_error("Invalid expression: too many values left");
    }

    return value_stack.top();
}

bool Calculator::isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

int Calculator::getPrecedence(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        default:
            return -1;
    }
}

bool Calculator::isLeftAssociative(char op) {
    return true;
}
