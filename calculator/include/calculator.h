#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <stdexcept>

class Calculator {
public:
    enum class TokenType { NUMBER, OPERATOR, LEFT_PAREN, RIGHT_PAREN, VARIABLE, ASSIGNMENT };
    struct Token {
        TokenType type;
        std::string value;
        int precedence = -1;
        bool isLeftAssociative = true;
        std::string variableName; // Used for variable tokens
    };

    double evaluate(const std::string& expression, std::map<std::string, double>& variables);

private:
    std::vector<Token> tokenize(const std::string& expression);
    std::vector<Token> shuntingYard(const std::vector<Token>& tokens);
    double evaluateRPN(std::vector<Token> rpnTokens, std::map<std::string, double>& variables);

    bool isOperator(char c);
    int getPrecedence(char op);
    bool isLeftAssociative(char op);
    bool isAlpha(char c);
    bool isValidVariableName(const std::string& name);
};

#endif // CALCULATOR_H