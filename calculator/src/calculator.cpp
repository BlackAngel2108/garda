#include "calculator.h"
#include <stack>
#include <sstream>
#include <cmath>
#include <cctype> // For isalpha, isalnum
#include <iostream>
#include <algorithm> // For std::find_first_not_of, find_last_not_of

bool Calculator::isValidVariableName(const std::string& name) {
    if (name.empty() || !isalpha(name[0])) {
        return false;
    }
    for (char c : name) {
        if (!isalnum(c)) { // Allows letters and numbers
            return false;
        }
    }
    return true;
}

double Calculator::evaluate(const std::string& expression, std::map<std::string, double>& variables) {
    std::istringstream iss(expression);
    std::string segment;
    double last_result = 0.0; // Store the result of the last successful evaluation

    while(std::getline(iss, segment, ';')) {
        if (segment.empty()) continue;

        // Trim whitespace from segment
        segment.erase(0, segment.find_first_not_of(" 	\n\r\f\v"));
        segment.erase(segment.find_last_not_of(" 	\n\r\f\v") + 1);

        if (segment.empty()) continue;

        // The assignment logic is handled by RPN evaluation now
        std::vector<Calculator::Token> tokens = tokenize(segment);
        std::vector<Calculator::Token> rpnTokens = shuntingYard(tokens);
        last_result = evaluateRPN(rpnTokens, variables);
    }
    return last_result;
}

std::vector<Calculator::Token> Calculator::tokenize(const std::string& expression) {
    std::vector<Token> tokens;
    for (size_t i = 0; i < expression.length(); ++i) {
        char c = expression[i];

        if (isspace(c)) {
            continue;
        }
        else if (isdigit(c) || (c == '.' && i + 1 < expression.length() && isdigit(expression[i+1]))) { // NUMBER
            std::string num_str;
            while (i < expression.length() && (isdigit(expression[i]) || expression[i] == '.')) {
                num_str += expression[i];
                i++;
            }
            i--;
            tokens.push_back({TokenType::NUMBER, num_str});
        }
        else if (isalpha(c)) { // VARIABLE
            std::string var_name_str;
            while (i < expression.length() && isalnum(expression[i])) {
                var_name_str += expression[i];
                i++;
            }
            i--;
            tokens.push_back({TokenType::VARIABLE, var_name_str, -1, true, var_name_str});
        }
        else if (c == '=') { // ASSIGNMENT
            tokens.push_back({TokenType::ASSIGNMENT, "=", 0, false}); // Assignment is right-associative, lowest precedence
        }
        else if (isOperator(c)) { // OPERATOR (+-*/)
            tokens.push_back({TokenType::OPERATOR, std::string(1, c), getPrecedence(c), isLeftAssociative(c)});
        }
        else if (c == '(') { // LEFT_PAREN
            tokens.push_back({TokenType::LEFT_PAREN, "("});
        }
        else if (c == ')') { // RIGHT_PAREN
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
            case TokenType::VARIABLE:
                output_queue.push_back(token);
                break;
            case TokenType::OPERATOR:
            case TokenType::ASSIGNMENT:
                while (!operator_stack.empty() && 
                       (operator_stack.top().type == TokenType::OPERATOR || operator_stack.top().type == TokenType::ASSIGNMENT) &&
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

double Calculator::evaluateRPN(std::vector<Token> rpnTokens, std::map<std::string, double>& variables) {
    std::stack<Token> internal_stack; // Now stores Tokens

    for (const auto& token : rpnTokens) {
        if (token.type == TokenType::NUMBER) {
            internal_stack.push(token);
        }
        else if (token.type == TokenType::VARIABLE) {
            internal_stack.push(token); // Push variable token itself
        }
        else if (token.type == TokenType::OPERATOR) {
            if (internal_stack.size() < 2) {
                throw std::runtime_error("Invalid expression: not enough operands for operator '" + token.value + "'");
            }
            // Pop right operand
            Token right_token = internal_stack.top(); internal_stack.pop();
            double right_val;
            if (right_token.type == TokenType::NUMBER) {
                right_val = std::stod(right_token.value);
            } else if (right_token.type == TokenType::VARIABLE) {
                if (variables.count(right_token.variableName)) {
                    right_val = variables[right_token.variableName];
                } else {
                    throw std::runtime_error("Unknown variable: " + right_token.variableName);
                }
            } else {
                throw std::runtime_error("Invalid operand for operator: " + right_token.value);
            }

            // Pop left operand
            Token left_token = internal_stack.top(); internal_stack.pop();
            double left_val;
            if (left_token.type == TokenType::NUMBER) {
                left_val = std::stod(left_token.value);
            } else if (left_token.type == TokenType::VARIABLE) {
                if (variables.count(left_token.variableName)) {
                    left_val = variables[left_token.variableName];
                } else {
                    throw std::runtime_error("Unknown variable: " + left_token.variableName);
                }
            } else {
                throw std::runtime_error("Invalid operand for operator: " + left_token.value);
            }

            double result;
            switch (token.value[0]) {
                case '+': result = left_val + right_val; break;
                case '-': result = left_val - right_val; break;
                case '*': result = left_val * right_val; break;
                case '/':
                    if (right_val == 0) throw std::runtime_error("Division by zero");
                    result = left_val / right_val;
                    break;
                default:
                    throw std::runtime_error("Unknown operator: " + token.value);
            }
            internal_stack.push({TokenType::NUMBER, std::to_string(result)});
        }
        else if (token.type == TokenType::ASSIGNMENT) {
            if (internal_stack.size() < 2) {
                throw std::runtime_error("Invalid assignment: not enough operands for assignment");
            }
            // Pop value to assign (RHS)
            Token value_token = internal_stack.top(); internal_stack.pop();
            double value_to_assign;
            if (value_token.type == TokenType::NUMBER) {
                value_to_assign = std::stod(value_token.value);
            } else if (value_token.type == TokenType::VARIABLE) {
                 if (variables.count(value_token.variableName)) {
                    value_to_assign = variables[value_token.variableName];
                } else {
                    throw std::runtime_error("Unknown variable: " + value_token.variableName);
                }
            } else {
                throw std::runtime_error("Invalid value for assignment: " + value_token.value);
            }

            // Pop variable token (LHS)
            Token var_token = internal_stack.top(); internal_stack.pop();
            if (var_token.type != TokenType::VARIABLE) {
                throw std::runtime_error("Invalid target for assignment: " + var_token.value);
            }

            variables[var_token.variableName] = value_to_assign;
            internal_stack.push({TokenType::NUMBER, std::to_string(value_to_assign)}); // Push the assigned value back as the result
        }
        else {
            throw std::runtime_error("Unexpected token type in RPN evaluation: " + std::to_string(static_cast<int>(token.type)));
        }
    }

    if (internal_stack.size() != 1 || internal_stack.top().type != TokenType::NUMBER) {
        throw std::runtime_error("Invalid expression: result is not a single number");
    }

    return std::stod(internal_stack.top().value);
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
        case '=': // Assignment operator
            return 0; // Lowest precedence
        default:
            return -1;
    }
}

bool Calculator::isLeftAssociative(char op) {
    if (op == '=') {
        return false; // Assignment is right-associative
    }
    return true; // Other operators are left-associative
}

bool Calculator::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}