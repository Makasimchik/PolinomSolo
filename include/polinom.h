#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <gtest.h>

using namespace std;

enum class ParseState {
    Start,      // Начальное состояние
    ReadCoeff,  // Чтение коэффициента
    ReadVar,    // Чтение переменной
    ReadPower   // Чтение степени
};

class Monomial {
private:
    double coefficient;
    int powerX, powerY, powerZ;

public:
    // Конструкторы
    Monomial(double coeff = 0, int px = 0, int py = 0, int pz = 0)
        : coefficient(coeff), powerX(px), powerY(py), powerZ(pz) {
    }

    Monomial(const string& str) {
        *this = parse(str);
    }

    // Геттеры
    double getCoefficient() const { return coefficient; }
    int getPowerX() const { return powerX; }
    int getPowerY() const { return powerY; }
    int getPowerZ() const { return powerZ; }

    // Операторы сравнения
    bool operator==(const Monomial& other) const {
        return coefficient == other.coefficient &&
            powerX == other.powerX &&
            powerY == other.powerY &&
            powerZ == other.powerZ;
    }

    bool operator!=(const Monomial& other) const {
        return !(*this == other);
    }

    // Операторы и методы
    friend istream& operator>>(istream& is, Monomial& m) {
        string input;
        is >> input;
        m = parse(input);
        return is;
    }

    friend ostream& operator<<(ostream& os, const Monomial& m) {
        os << m.toString();
        return os;
    }

    bool isSimilar(const Monomial& other) const {
        return powerX == other.powerX &&
            powerY == other.powerY &&
            powerZ == other.powerZ;
    }

    bool operator<(const Monomial& other) const {
        if (powerX != other.powerX) return powerX > other.powerX;
        if (powerY != other.powerY) return powerY > other.powerY;
        return powerZ > other.powerZ;
    }

    Monomial operator+(const Monomial& other) const {
        if (!isSimilar(other)) throw runtime_error("Cannot add different monomials");
        return Monomial(coefficient + other.coefficient, powerX, powerY, powerZ);
    }

    Monomial operator-(const Monomial& other) const {
        if (!isSimilar(other)) throw runtime_error("Cannot subtract different monomials");
        return Monomial(coefficient - other.coefficient, powerX, powerY, powerZ);
    }

    Monomial operator*(const Monomial& other) const {
        if (powerX + other.powerX > 9 ||
            powerY + other.powerY > 9 ||
            powerZ + other.powerZ > 9) {
            throw runtime_error("Degree overflow");
        }
        return Monomial(
            coefficient * other.coefficient,
            powerX + other.powerX,
            powerY + other.powerY,
            powerZ + other.powerZ
        );
    }

    Monomial operator*(double scalar) const {
        return Monomial(coefficient * scalar, powerX, powerY, powerZ);
    }

    Monomial operator/(double divisor) const {
        if (divisor == 0) throw runtime_error("Division by zero");
        return Monomial(coefficient / divisor, powerX, powerY, powerZ);
    }

    string toString() const {
        if (coefficient == 0) return "0";

        ostringstream oss;
        if (coefficient != 1 && coefficient != -1 ||
            (powerX == 0 && powerY == 0 && powerZ == 0)) {
            oss << coefficient;
        }
        else if (coefficient == -1) {
            oss << "-";
        }

        if (powerX > 0) {
            oss << "x";
            if (powerX > 1) oss << "^" << powerX;
        }
        if (powerY > 0) {
            oss << "y";
            if (powerY > 1) oss << "^" << powerY;
        }
        if (powerZ > 0) {
            oss << "z";
            if (powerZ > 1) oss << "^" << powerZ;
        }

        return oss.str();
    }

    static Monomial parse(const string& str) {
        double coeff = 1.0;
        int px = 0, py = 0, pz = 0;
        string s = str;
        s.erase(remove(s.begin(), s.end(), ' '), s.end());

        ParseState state = ParseState::Start;
        string buffer;
        char currentVar = '\0';

        for (char c : s) {
            switch (state) {
            case ParseState::Start:
                if (c == '+' || c == '-') {
                    buffer += c;
                    state = ParseState::ReadCoeff;
                }
                else if (isdigit(c)) {
                    buffer += c;
                    state = ParseState::ReadCoeff;
                }
                else if (isalpha(c)) {
                    currentVar = tolower(c);
                    state = ParseState::ReadVar;
                }
                break;

            case ParseState::ReadCoeff:
                if (isdigit(c) || c == '.') {
                    buffer += c;
                }
                else if (isalpha(c)) {
                    if (!buffer.empty()) coeff = stod(buffer);
                    currentVar = tolower(c);
                    state = ParseState::ReadVar;
                }
                break;

            case ParseState::ReadVar:
                if (c == '^') {
                    buffer.clear();
                    state = ParseState::ReadPower;
                }
                else {
                    // Обработка степени 1
                    switch (currentVar) {
                    case 'x': px = 1; break;
                    case 'y': py = 1; break;
                    case 'z': pz = 1; break;
                    }
                    currentVar = '\0';
                    state = ParseState::Start;
                }
                break;

            case ParseState::ReadPower:
                if (isdigit(c)) {
                    buffer += c;
                }
                else {
                    int power = stoi(buffer);
                    switch (currentVar) {
                    case 'x': px = power; break;
                    case 'y': py = power; break;
                    case 'z': pz = power; break;
                    }
                    buffer.clear();
                    state = ParseState::Start;
                }
                break;
            }
        }

        // Обработка последнего символа
        if (!buffer.empty() && currentVar != '\0') {
            int power = buffer.empty() ? 1 : stoi(buffer);
            switch (currentVar) {
            case 'x': px = power; break;
            case 'y': py = power; break;
            case 'z': pz = power; break;
            }
        }

        return Monomial(coeff, px, py, pz);
    }
};

class Polynomial {
private:
    vector<Monomial> terms;

    void sortAndSimplify() {
        sort(terms.begin(), terms.end());
        vector<Monomial> simplified;

        for (auto& term : terms) {
            if (term.getCoefficient() == 0) continue;

            if (!simplified.empty() && simplified.back().isSimilar(term)) {
                simplified.back() = simplified.back() + term;
            }
            else {
                simplified.push_back(term);
            }
        }
        terms = simplified;
    }

public:
    // Конструкторы
    Polynomial() = default;
    Polynomial(const string& str) { parse(str); }

    // Операторы
    Polynomial operator+(const Polynomial& other) const {
        Polynomial result = *this;
        result.terms.insert(result.terms.end(), other.terms.begin(), other.terms.end());
        result.sortAndSimplify();
        return result;
    }

    Polynomial operator-(const Polynomial& other) const {
        Polynomial result = *this;
        for (const auto& term : other.terms) {
            result.terms.push_back(term * -1);
        }
        result.sortAndSimplify();
        return result;
    }

    Polynomial operator*(const Polynomial& other) const {
        Polynomial result;
        for (const auto& t1 : terms) {
            for (const auto& t2 : other.terms) {
                result.terms.push_back(t1 * t2);
            }
        }
        result.sortAndSimplify();
        return result;
    }

    Polynomial operator/(double divisor) const {
        if (divisor == 0) throw runtime_error("Division by zero");
        Polynomial result = *this;
        for (auto& term : result.terms) {
            term = term / divisor;
        }
        return result;
    }

    // Методы
    enum class ParseState { Start, ReadCoeff, ReadVar, ReadPower, End };

    void parse(const string& str) {
        terms.clear();
        string s = str;
        s.erase(remove(s.begin(), s.end(), ' '), s.end());

        ParseState state = ParseState::Start;
        char currentSign = '+';
        string currentTerm;
        size_t pos = 0;

        while (pos < s.size()) {
            char c = s[pos];

            switch (state) {
            case ParseState::Start:
                if (c == '+' || c == '-') {
                    currentSign = c;
                    pos++;
                }
                state = ParseState::ReadCoeff;
                currentTerm.clear();
                break;

            case ParseState::ReadCoeff:
                if (isdigit(c) || c == '.') {
                    currentTerm += c;
                    pos++;
                }
                else {
                    state = ParseState::ReadVar;
                }
                break;

            case ParseState::ReadVar:
                if (isalpha(c)) {
                    currentTerm += c;
                    pos++;
                }
                else if (c == '^') {
                    state = ParseState::ReadPower;
                    pos++;
                }
                else {
                    // Завершаем текущий одночлен
                    Monomial m(currentTerm);
                    if (currentSign == '-') m = m * -1;
                    terms.push_back(m);
                    state = ParseState::Start;
                }
                break;

            case ParseState::ReadPower:
                if (isdigit(c)) {
                    currentTerm += '^';
                    currentTerm += c;
                    pos++;
                }
                else {
                    state = ParseState::ReadVar;
                }
                break;
            }
        }

        // Добавляем последний одночлен, если он есть
        if (!currentTerm.empty()) {
            Monomial m(currentTerm);
            if (currentSign == '-') m = m * -1;
            terms.push_back(m);
        }

        sortAndSimplify();
    }

    void addTerm(const Monomial& m) {
        terms.push_back(m);
        sortAndSimplify();
    }

    string toString() const {
        if (terms.empty()) return "0";
        ostringstream oss;
        for (size_t i = 0; i < terms.size(); ++i) {
            string term = terms[i].toString();
            if (i > 0 && term[0] != '-') oss << "+";
            oss << term;
        }
        return oss.str();
    }

    const vector<Monomial>& getTerms() const { return terms; }

    friend ostream& operator<<(ostream& os, const Polynomial& p) {
        os << p.toString();
        return os;
    }

    friend istream& operator>>(istream& is, Polynomial& p) {
        string input;
        getline(is, input);
        p.parse(input);
        return is;
    }

    bool operator==(const Polynomial& other) const {
        return toString() == other.toString();
    }

    bool operator!=(const Polynomial& other) const {
        return !(*this == other);
    }
};

class PolynomialStorage {
private:
    map<string, Polynomial> polynomials;

public:
    void add(const string& name, const Polynomial& poly) {
        if (polynomials.count(name)) {
            cout << "Полином '" << name << "' уже существует. Перезаписать? (y/n): ";
            char choice;
            cin >> choice;
            cin.ignore();
            if (tolower(choice) != 'y') return;
        }
        polynomials[name] = poly;
        cout << "Полином '" << name << "' сохранен.\n";
    }

    void remove(const string& name) {
        if (polynomials.erase(name)) {
            cout << "Полином '" << name << "' удален.\n";
        }
        else {
            cout << "Полином '" << name << "' не найден.\n";
        }
    }

    Polynomial get(const string& name) const {
        auto it = polynomials.find(name);
        if (it == polynomials.end()) {
            throw runtime_error("Полином не найден");
        }
        return it->second;
    }

    void list() const {
        if (polynomials.empty()) {
            cout << "Нет сохраненных полиномов.\n";
            return;
        }
        cout << "Список полиномов:\n";
        for (const auto& entry : polynomials) {
            cout << "  " << entry.first << " = " << entry.second << endl;
        }
    }
};

void showMenu() {
    cout << "\nМеню:\n";
    cout << "1. Добавить полином\n";
    cout << "2. Удалить полином\n";
    cout << "3. Просмотреть полином\n";
    cout << "4. Список всех полиномов\n";
    cout << "5. Сложить два полинома\n";
    cout << "6. Вычесть два полинома\n";
    cout << "7. Умножить два полинома\n";
    cout << "8. Разделить полином на число\n";
    cout << "9. Выход\n";
    cout << "Выберите действие: ";
}

void runTests() {
    // Простые тесты без Google Test
    cout << "\n=== ТЕСТИРОВАНИЕ ===\n";
    cout << "==================\n";
}

int main() {
    setlocale(LC_ALL, "Russian");
    PolynomialStorage storage;
    int choice;

    while (true) {
        showMenu();
        cin >> choice;
        cin.ignore();

        try {
            if (choice == 1) {
                string name, expr;
                cout << "Введите имя полинома: ";
                getline(cin, name);
                cout << "Введите полином (например, 2x^2y - 3z + 5): ";
                getline(cin, expr);
                storage.add(name, Polynomial(expr));
            }
            else if (choice == 2) {
                string name;
                cout << "Введите имя полинома для удаления: ";
                getline(cin, name);
                storage.remove(name);
            }
            else if (choice == 3) {
                string name;
                cout << "Введите имя полинома: ";
                getline(cin, name);
                cout << name << " = " << storage.get(name) << endl;
            }
            else if (choice == 4) {
                storage.list();
            }
            else if (choice == 5) {
                string name1, name2, resultName;
                cout << "Введите имя первого полинома: ";
                getline(cin, name1);
                cout << "Введите имя второго полинома: ";
                getline(cin, name2);

                Polynomial result = storage.get(name1) + storage.get(name2);
                cout << "Результат: " << result << endl;

                cout << "Сохранить результат? (y/n): ";
                char save;
                cin >> save;
                cin.ignore();
                if (tolower(save) == 'y') {
                    cout << "Введите имя для сохранения: ";
                    getline(cin, resultName);
                    storage.add(resultName, result);
                }
            }
            else if (choice == 6) {
                string name1, name2, resultName;
                cout << "Введите имя первого полинома: ";
                getline(cin, name1);
                cout << "Введите имя второго полинома: ";
                getline(cin, name2);

                Polynomial result = storage.get(name1) - storage.get(name2);
                cout << "Результат: " << result << endl;

                cout << "Сохранить результат? (y/n): ";
                char save;
                cin >> save;
                cin.ignore();
                if (tolower(save) == 'y') {
                    cout << "Введите имя для сохранения: ";
                    getline(cin, resultName);
                    storage.add(resultName, result);
                }
            }
            else if (choice == 7) {
                string name1, name2, resultName;
                cout << "Введите имя первого полинома: ";
                getline(cin, name1);
                cout << "Введите имя второго полинома: ";
                getline(cin, name2);

                Polynomial result = storage.get(name1) * storage.get(name2);
                cout << "Результат: " << result << endl;

                cout << "Сохранить результат? (y/n): ";
                char save;
                cin >> save;
                cin.ignore();
                if (tolower(save) == 'y') {
                    cout << "Введите имя для сохранения: ";
                    getline(cin, resultName);
                    storage.add(resultName, result);
                }
            }
            else if (choice == 8) {
                string name, resultName;
                double num;
                cout << "Введите имя полинома: ";
                getline(cin, name);
                cout << "Введите число для деления: ";
                cin >> num;
                cin.ignore();

                Polynomial result = storage.get(name) / num;
                cout << "Результат: " << result << endl;
            }
            else if (choice == 9) {
                break;
            }
            else if (choice == 0) {
                cout << "\n=== ЗАПУСК ТЕСТОВ ===\n";
                int argc = 1;
                char* argv[] = { (char*)"test_program" };
                testing::InitGoogleTest(&argc, argv);
                RUN_ALL_TESTS();
                cout << "\n=== ТЕСТЫ ЗАВЕРШЕНЫ ===\n\n";
            }
            else {
                cout << "Неверный выбор!\n";
            }
        }
        catch (const exception& e) {
            cout << "Ошибка: " << e.what() << endl;
        }
    }

    return 0;
}
