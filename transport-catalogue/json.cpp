#include "json.h"
#include <algorithm>
#include <cstdio>
#include <string>
#include <limits>



using namespace std;

namespace json {

//namespace {



Node LoadNode(std::istream& input);

Node LoadArray(std::istream& input) {
    Array result;

    char t;
    if (input >> t){
        input.putback(t);
    } else {
        throw ParsingError("Invalid_token"s);
    }

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node{move(result)};
}

Node LoadDict(std::istream& input) {
    Dict result;

    char t;
    if (input >> t){
        input.putback(t);
    } else if (input.eof()){
        throw ParsingError("Invalid_token"s);
    }
    for (char c; input >> c && c != '}' && c != EOF && c!='\0';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    return Node{move(result)};
}

Node LoadNull(std::istream& input){
    char c;
    std::string s;
    while (input >> c || s.size() <= 4){
        s += c;
        if (s == "null"s){
            break;
        }
        c = 0;
    }
    if (s.find("null") != std::string::npos) {
        return Node{};
    }
    else {
        throw ParsingError("Invalid_null_token"s);
    }
}

Node LoadBool(std::istream& input){
    char c;
    std::string s;
    while (input >> c || s.size() <= 5){
        s += c;
        if (s == "true"s || s == "false"s){
            break;
        }
        c = 0;
    }
    if (s.find("true") != std::string::npos){
        return Node{true};
    }
    else if (s.find("false") != std::string::npos) {
        return Node{false};
    }
    else {
        throw ParsingError("Invalid_bool_token"s);
    }
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;
    if (c == '[') {
        return LoadArray(input);        
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return Node{LoadString(input)};
        }
    else if (c == 'n'){
        input.putback(c);
        return LoadNull(input);             
    } else if (c == 't' || c == 'f'){
        input.putback(c);
        return LoadBool(input); 
    }
    else {
        input.putback(c);
        auto return_value = LoadNumber(input);
        if (std::holds_alternative<int>(return_value)){
            return Node{std::get<int>(return_value)};
        }
        else {
            return Node{std::get<double>(return_value)};
        }
    }
    throw ParsingError("Invalid_token"s);
}

//}  // namespace

// Node::Node(Array array)
//     : value_(move(array)) {
// }

// Node::Node(std::nullptr_t value){
//     value_ = value;
// }

// Node::Node(Dict map)
//     : value_(move(map)) {
// }

// Node::Node(int value)
//     : value_(value) {
// }

// Node::Node(double value)
//     : value_(value){
// }

// Node::Node(bool value)
//     : value_(value){

// }

// Node::Node(string value) : value_(std::move(value))
// {}

// Node::Node(std::variant<int, double> value){
//     if (std::holds_alternative<int>(value)){
//         value_ = std::get<int>(value);
//     }
//     if (std::holds_alternative<double>(value)){
//         value_ = std::get<double>(value);
//     }
// }

bool Node::IsInt() const {
    if (std::holds_alternative<int>(GetValue())){
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsDouble() const {
    if (std::holds_alternative<double>(GetValue()) || std::holds_alternative<int>(GetValue())){
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsPureDouble() const {
    if (std::holds_alternative<double>(GetValue())){
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsBool() const {
    if (std::holds_alternative<bool>(GetValue())){
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsString() const {
    if (std::holds_alternative<std::string>(GetValue())){
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsNull() const {
    if (std::holds_alternative<std::nullptr_t>(GetValue())){
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsArray() const {
    if (std::holds_alternative<Array>(GetValue())){
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsMap() const {
    if (std::holds_alternative<Dict>(GetValue())){
        return true;
    }
    else {
        return false;
    }
}

int Node::AsInt() const {
    if (IsInt()){
        return std::get<int>(GetValue());
    }
    else {
        throw std::logic_error("invalid_data_type");
    }
}

bool Node::AsBool() const {
    if (IsBool()){
        return std::get<bool>(GetValue());
    }
    else {
        throw std::logic_error("invalid_data_type");
    }
}

double Node::AsDouble() const {
    if (std::holds_alternative<int>(GetValue())){
        return static_cast<double>(std::get<int>(GetValue()));
    }
    else if (std::holds_alternative<double>(GetValue())){
        return std::get<double>(GetValue());
    }
    else {
        throw std::logic_error("invalid_data_type");
    }
}

const std::string& Node::AsString() const {
    if (IsString()){
        return std::get<std::string>(GetValue());
    }
    else {
        throw std::logic_error("invalid_data_type");
    }
}

const Array& Node::AsArray() const {
    if (IsArray()){
        return std::get<Array>(GetValue());
    }
    else {
        throw std::logic_error("invalid_data_type");
    }
}

const Dict& Node::AsMap() const {
    if (IsMap()){
        return std::get<Dict>(GetValue());
    }
    else {
        throw std::logic_error("invalid_data_type");
    }
}

// const Node::Value& Node::GetValue() const {
//     return value_;
// }

const Node& Node::GetValue() const {
    return *this;
}

Document::Document(Node root) : root_(std::move(root))
{
   // root_ = root;
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(std::istream& input) {
    std::string query {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
    //
    
    //
    size_t quote_f = query.find_first_of('\"');
    size_t quote_l = query.find_last_of('\"');

    const std::vector<char> escape_chars = {'\r', '\n', '\t', '\\'}; 
    std::string processed_query;
    for (size_t i = 0; i < query.size(); ++i){
        if ((i <= quote_f && i >= quote_l) && std::any_of(escape_chars.begin(), escape_chars.end(), [&query, i](char y){ return y == query[i]; })){
        }
        else {
            processed_query += query[i];
        }
    }
    std::istringstream ss(processed_query);
    return Document{LoadNode(ss)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
    // Реализуйте функцию самостоятельно
}

void PrintValue(const int value, std::ostream& out) {
    out << value;
}

void PrintValue(const double value, std::ostream& out) {
    out << value;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

void PrintValue(bool var, std::ostream& out) {
    out << std::boolalpha;
    out << var;
}

void PrintValue(const std::string& s, std::ostream& out) {
    const std::vector<char> escape_chars = {'\"', '\\'};
    std::string output;
    output += '\"';
    for (const char c : s){
        if (c == '\r'){
             output += "\\r"s;
        }
        else if (c == '\n'){
            output += "\\n"s;
        }
        else if (any_of(escape_chars.begin(), escape_chars.end(), [c](char y){ return y == c; })){
            output += '\\';
            output += c;
        }
        else {
            output += c;
        }
    }
    output += '\"';
    out << output;
}

void PrintValue(const Array& array, std::ostream& out){
    bool is_first = true;
    out << '[';
    for (const auto& value : array){
        if (is_first){
           PrintNode(value, out);
           is_first = false;
        }
        else {
            out << ','; 
            PrintNode(value, out);
        }
    }
    out << ']';
}

void PrintValue(const Dict& dict, std::ostream& out){
    bool is_first = true;
    out << '{';
    for (const auto& [key, value] : dict){
        if (is_first){
        out << '\"' << key << '\"' << ':';
           PrintNode(value, out);
           is_first = false;
        }
        else {
            out << ',';
            out << '\"' << key << '\"' << ':';
            PrintNode(value, out);
        }
    }
    out << '}';
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value){ PrintValue(value, out); },
        static_cast<Value>(node));
}

bool Node::operator==(const Node& other) const {
    if ((this->IsInt() && other.IsInt()) && (this->GetValue() == other.GetValue())){
        return true;
    }
    else if ((this->IsPureDouble() && other.IsPureDouble()) && (this->GetValue() == other.GetValue())){
        return true;
    }
    else if ((this->IsBool() && other.IsBool()) && (this->GetValue() == other.GetValue())){
        return true;
    }
    else if ((this->IsString() && other.IsString()) && (this->GetValue() == other.GetValue())){
        return true;
    }
    else if ((this->IsNull() && other.IsNull()) && (this->GetValue() == other.GetValue())){
        return true;
    }
    else if ((this->IsArray() && other.IsArray()) && (this->GetValue() == other.GetValue())){
        return true;
    }
    else if ((this->IsMap() && other.IsMap()) && (this->GetValue() == other.GetValue())){
        return true;
    }
    else {
        return false;
    }
}

bool Node::operator!=(const Node& other) const {
    return !(*this == other);
}

bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}

bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

}  // namespace json