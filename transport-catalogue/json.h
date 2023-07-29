#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>

namespace json {

using namespace std::literals;

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;


// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node : public Value {
public:

using std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>::variant;

    bool IsInt() const;
    bool IsDouble() const; //Возвращает true, если в Node хранится int либо double.
    bool IsPureDouble() const; //Возвращает true, если в Node хранится double.
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const; //Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

   // const Value& GetValue() const;

    const Node& GetValue() const;

    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

private:
     //Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

// Шаблон, подходящий для вывода double и int
void PrintValue(const int value, std::ostream& out);

void PrintValue(const double value, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out);

void PrintValue(bool var, std::ostream& out);

void PrintValue(const std::string& s, std::ostream& out);

void PrintValue(const Array& array, std::ostream& out);

void PrintValue(const Dict& dict, std::ostream& out);

template <typename T>
void PrintValue(const std::vector<T>& v, std::ostream& out) {
    bool x = true;
    for (auto it = v.begin(); it != v.end(); ++it){
        if (x == true){
            PrintNode(*it, out);
            out << " ";
            x = false;
        }
        else {
            PrintNode(*it, out);
        }
    }
}

template <typename K, typename V>
void PrintValue(const std::map<K, V>& m, std::ostream& out) {
    bool x = true;
    for (const auto [key, value] : m){
        if (x){
            out << key << " ";
            PrintNode(value, out); 
            out << ";"sv;
            x = false;
        }
        else {
            out << key << " ";
            PrintNode(value, out);
        }
    }
}


void PrintNode(const Node& node, std::ostream& out);

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input);

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input);

Node LoadNode(std::istream& input);

Node LoadBool(std::istream& input);

Node LoadNull(std::istream& input);


}  // namespace json