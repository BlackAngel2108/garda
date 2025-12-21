#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>
#include <vector>
#include <stdexcept>

class Calculator {
public:
    double evaluate(const std::string& expression);

private:
    enum class TokenType { NUMBER, OPERATOR, LEFT_PAREN, RIGHT_PAREN };
    struct Token {
        TokenType type;
        std::string value;
        int precedence = -1;
        bool isLeftAssociative = true;
    };

    std::vector<Token> tokenize(const std::string& expression);
    std::vector<Token> shuntingYard(const std::vector<Token>& tokens);
    double evaluateRPN(std::vector<Token> rpnTokens);

    bool isOperator(char c);
    int getPrecedence(char op);
    bool isLeftAssociative(char op);
};

#endif // CALCULATOR_H