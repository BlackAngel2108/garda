#include "gtest/gtest.h"
#include "calculator.h"

TEST(CalculatorTest, SimpleAddition) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("2 + 2"), 4.0);
}

TEST(CalculatorTest, SimpleSubtraction) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("10 - 4"), 6.0);
}

TEST(CalculatorTest, OperatorPrecedence) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("2 + 3 * 4"), 14.0);
    ASSERT_DOUBLE_EQ(calc.evaluate("10 / 2 - 3"), 2.0);
}

TEST(CalculatorTest, Parentheses) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("(2 + 3) * 4"), 20.0);
    ASSERT_DOUBLE_EQ(calc.evaluate("10 / (2 + 3)"), 2.0);
}

TEST(CalculatorTest, FloatingPoint) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("1.5 + 2.5"), 4.0);
    ASSERT_DOUBLE_EQ(calc.evaluate("5.0 / 2.0"), 2.5);
}

TEST(CalculatorTest, ComplexExpression) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("3 + 4 * 2 / ( 1 - 5 )"), 1.0);
}

TEST(CalculatorTest, DivisionByZero) {
    Calculator calc;
    ASSERT_THROW(calc.evaluate("5 / 0"), std::runtime_error);
    ASSERT_THROW(calc.evaluate("1 / (2 - 2)"), std::runtime_error);
}

TEST(CalculatorTest, MismatchedParentheses) {
    Calculator calc;
    ASSERT_THROW(calc.evaluate("(2 + 3"), std::runtime_error);
    ASSERT_THROW(calc.evaluate("2 + 3)"), std::runtime_error);
}

TEST(CalculatorTest, InvalidCharacter) {
    Calculator calc;
    ASSERT_THROW(calc.evaluate("2 + a"), std::runtime_error);
}
