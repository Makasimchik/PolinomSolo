#include "polinom.h"
#include <gtest.h>

TEST(MonomialTest, CreationFromString) {
    Monomial m1("3x^2y");
    EXPECT_DOUBLE_EQ(m1.getCoefficient(), 3.0);
    EXPECT_EQ(m1.getPowerX(), 2);
    EXPECT_EQ(m1.getPowerY(), 1);
    EXPECT_EQ(m1.getPowerZ(), 0);

    Monomial m2("-1.5xy^2z^3");
    EXPECT_DOUBLE_EQ(m2.getCoefficient(), -1.5);
    EXPECT_EQ(m2.getPowerX(), 1);
    EXPECT_EQ(m2.getPowerY(), 2);
    EXPECT_EQ(m2.getPowerZ(), 3);
}

TEST(MonomialTest, ValidAddition) {
    Monomial m1("2x^2y");
    Monomial m2("3x^2y");
    Monomial result = m1 + m2;

    EXPECT_DOUBLE_EQ(result.getCoefficient(), 5.0);
    EXPECT_EQ(result.getPowerX(), 2);
    EXPECT_EQ(result.getPowerY(), 1);
    EXPECT_EQ(result.getPowerZ(), 0);
}

TEST(MonomialTest, InvalidAddition) {
    Monomial m1("2x^2y");
    Monomial m2("3xy^2");
    EXPECT_THROW(m1 + m2, runtime_error);
}

TEST(MonomialTest, Multiplication) {
    Monomial m1("2x^2y");
    Monomial m2("3xy^3");
    Monomial result = m1 * m2;

    EXPECT_DOUBLE_EQ(result.getCoefficient(), 6.0);
    EXPECT_EQ(result.getPowerX(), 3);
    EXPECT_EQ(result.getPowerY(), 4);
    EXPECT_EQ(result.getPowerZ(), 0);
}

TEST(MonomialTest, ScalarMultiplication) {
    Monomial m("2x^2y");
    Monomial result = m * 3;

    EXPECT_DOUBLE_EQ(result.getCoefficient(), 6.0);
    EXPECT_EQ(result.getPowerX(), 2);
    EXPECT_EQ(result.getPowerY(), 1);
    EXPECT_EQ(result.getPowerZ(), 0);
}

TEST(PolynomialTest, CreationAndParsing) {
    Polynomial p("2x^2 + 3y - z");
    string result = p.toString();

    // Проверяем наличие всех членов (порядок может меняться)
    EXPECT_TRUE(result.find("2x^2") != string::npos);
    EXPECT_TRUE(result.find("3y") != string::npos);
    EXPECT_TRUE(result.find("-z") != string::npos);
}

TEST(PolynomialTest, Addition) {
    Polynomial p1("x + y");
    Polynomial p2("2x - y");
    Polynomial result = p1 + p2;

    bool has3x = false;
    for (const auto& term : result.getTerms()) {
        if (term.toString() == "3x") {
            has3x = true;
            break;
        }
    }
    EXPECT_TRUE(has3x);
    EXPECT_EQ(result.getTerms().size(), 1);
}

TEST(PolynomialTest, Ordering) {
    Polynomial p("x + x^3 + x^2");
    const auto& terms = p.getTerms();

    // Проверяем порядок членов
    EXPECT_GE(terms[0].getPowerX(), terms[1].getPowerX());
    EXPECT_GE(terms[1].getPowerX(), terms[2].getPowerX());
}

TEST(PolynomialTest, Multiplication) {
    Polynomial p1("x + y");
    Polynomial p2("x - y");
    Polynomial result = p1 * p2;

    bool hasX2 = false, hasY2 = false;
    for (const auto& term : result.getTerms()) {
        if (term.toString() == "x^2") hasX2 = true;
        if (term.toString() == "-y^2") hasY2 = true;
    }
    EXPECT_TRUE(hasX2);
    EXPECT_TRUE(hasY2);
}

TEST(PolynomialTest, DivisionByScalar) {
    Polynomial p("4x + 6y");
    Polynomial result = p / 2;

    bool has2x = false, has3y = false;
    for (const auto& term : result.getTerms()) {
        if (term.toString() == "2x") has2x = true;
        if (term.toString() == "3y") has3y = true;
    }
    EXPECT_TRUE(has2x);
    EXPECT_TRUE(has3y);
}

