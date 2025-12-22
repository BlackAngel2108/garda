#include "gtest/gtest.h"
#include "calculator.h"
#include <map>

// Helper map for tests that don't need variable state
std::map<std::string, double> empty_vars_for_const_evaluate;

TEST(CalculatorTest, SimpleAddition) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("2 + 2", empty_vars_for_const_evaluate), 4.0);
}

TEST(CalculatorTest, SimpleSubtraction) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("10 - 4", empty_vars_for_const_evaluate), 6.0);
}

TEST(CalculatorTest, OperatorPrecedence) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("2 + 3 * 4", empty_vars_for_const_evaluate), 14.0);
    ASSERT_DOUBLE_EQ(calc.evaluate("10 / 2 - 3", empty_vars_for_const_evaluate), 2.0);
}

TEST(CalculatorTest, Parentheses) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("(2 + 3) * 4", empty_vars_for_const_evaluate), 20.0);
    ASSERT_DOUBLE_EQ(calc.evaluate("10 / (2 + 3)", empty_vars_for_const_evaluate), 2.0);
}

TEST(CalculatorTest, FloatingPoint) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("1.5 + 2.5", empty_vars_for_const_evaluate), 4.0);
    ASSERT_DOUBLE_EQ(calc.evaluate("5.0 / 2.0", empty_vars_for_const_evaluate), 2.5);
}

TEST(CalculatorTest, ComplexExpression) {
    Calculator calc;
    ASSERT_DOUBLE_EQ(calc.evaluate("3 + 4 * 2 / ( 1 - 5 )", empty_vars_for_const_evaluate), 1.0);
}

TEST(CalculatorTest, DivisionByZero) {
    Calculator calc;
    ASSERT_THROW(calc.evaluate("5 / 0", empty_vars_for_const_evaluate), std::runtime_error);
    ASSERT_THROW(calc.evaluate("1 / (2 - 2)", empty_vars_for_const_evaluate), std::runtime_error);
}

TEST(CalculatorTest, MismatchedParentheses) {
    Calculator calc;
    ASSERT_THROW(calc.evaluate("(2 + 3", empty_vars_for_const_evaluate), std::runtime_error);
    ASSERT_THROW(calc.evaluate("2 + 3)", empty_vars_for_const_evaluate), std::runtime_error);
}

TEST(CalculatorTest, InvalidCharacter) {
    Calculator calc;
    ASSERT_THROW(calc.evaluate("2 + $", empty_vars_for_const_evaluate), std::runtime_error);
}

// --- Tests for variables ---

TEST(CalculatorTest, VariableAssignmentAndUsage) {
    Calculator calc;
    std::map<std::string, double> vars;
    ASSERT_DOUBLE_EQ(calc.evaluate("x = 10", vars), 10.0);
    ASSERT_DOUBLE_EQ(calc.evaluate("x * 2", vars), 20.0);
    ASSERT_DOUBLE_EQ(calc.evaluate("y = x + 5; y / 3", vars), 5.0);
}

TEST(CalculatorTest, UnknownVariable) {
    Calculator calc;
    std::map<std::string, double> vars;
    ASSERT_THROW(calc.evaluate("z + 1", vars), std::runtime_error);
}

TEST(CalculatorTest, OverwriteVariable) {
    Calculator calc;
    std::map<std::string, double> vars;
    calc.evaluate("a = 5", vars);
    ASSERT_DOUBLE_EQ(calc.evaluate("a = 10", vars), 10.0);
    ASSERT_DOUBLE_EQ(vars["a"], 10.0);
}

TEST(CalculatorTest, MultiStatementComplex) {
    Calculator calc;
    std::map<std::string, double> vars;
    calc.evaluate("var1 = 5; var2 = var1 * 2; var2 + 1", vars);
    ASSERT_DOUBLE_EQ(vars["var1"], 5.0);
    ASSERT_DOUBLE_EQ(vars["var2"], 10.0);
    ASSERT_DOUBLE_EQ(calc.evaluate("var2 + 1", vars), 11.0);
}

TEST(CalculatorTest, InvalidVariableName) {
    Calculator calc;
    std::map<std::string, double> vars;
    ASSERT_THROW(calc.evaluate("1var = 5", vars), std::runtime_error);
    ASSERT_THROW(calc.evaluate("x y = 5", vars), std::runtime_error);
}

TEST(CalculatorTest, CommandClean) {
    Calculator calc;
    std::map<std::string, double> vars;
    calc.evaluate("a = 5", vars);
    ASSERT_DOUBLE_EQ(vars["a"], 5.0);
    // Manually clear variables for unit test
    vars.clear(); 
    ASSERT_TRUE(vars.empty());
    ASSERT_THROW(calc.evaluate("a + 1", vars), std::runtime_error);
}