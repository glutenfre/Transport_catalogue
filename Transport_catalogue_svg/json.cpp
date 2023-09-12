#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (input.eof()) {
                throw json::ParsingError("Data corrupted: waited for Array, but no correct input.");
            }

            return Node(move(result));
        }

        Node LoadDouble(int int_result, int negative, istream& input) {
            std::string result_left = std::to_string(int_result);
            std::string result_right = "0.";

            bool degree_flag = false;
            int degree = 0;
            int negative_degree = 1;

            bool left_before_dot = true;
            char c;
            for (; input >> c && (c != ',') && (c != '}') && (c != ']');) {
                if (isdigit(c)) {
                    if ((left_before_dot) && (!degree_flag)) {
                        result_left += c;
                    }
                    else if ((!left_before_dot) && (!degree_flag)) {
                        result_right += c;
                    }
                    else {// degree_flag == true
                        degree *= 10;
                        degree += c - '0';
                    }
                }
                if (c == '.') {
                    left_before_dot = false;
                }
                if ((c == 'e') || (c == 'E')) {
                    degree_flag = true;
                }
                if (c == '-') {
                    negative_degree = -1;
                }
            }
            input.putback(c);
            return Node(negative * (std::stod(result_left) + std::stod(result_right)) * std::pow(10, negative_degree * degree));
        }

        Node LoadInt(istream& input) {
            int result = 0;

            int negative = 1;

            if (input.peek() == '-') {
                negative = -1;
                input.get();
            }

            while (isdigit(input.peek())) {
                result *= 10;
                result += input.get() - '0';
            }
            if ((input.peek() == '.') || (input.peek() == 'e') || (input.peek() == 'E')) {
                return LoadDouble(result, negative, input);
            }

            return Node(negative * result);
        }

        Node LoadBool(istream& input) {
            bool result = false;
            std::string check;
            char c;
            for (; input >> c && (c != ',') && (c != '}') && (c != ']');) {
                check += c;
            }
            if (check == "true") {
                result = true;
            }
            else if (check == "false") {
                result = false;
            }
            else {
                throw json::ParsingError("Data corrupted: waited for Bool, but no correct input.");
            }
            input.putback(c);
            
            return Node(result);
        }

        void ParseString(std::string& result) {
            size_t len = result.size();
            size_t j = 0;
            while (j < len - 1) {
                char c1 = result.at(j + 1);
                if ((result.at(j) == '\\') && ((c1 == 'r') || (c1 == 'n') || (c1 == 't'))) {
                    result.erase(j, 1);
                    result.erase(j, 1);
                    len--;
                    if (c1 == 'r') {
                        result.insert(j, "\r");
                    }
                    else if (c1 == 'n') {
                        result.insert(j, "\n");
                    }
                    else {
                        result.insert(j, "\t");
                    }
                }
                else {
                    if (result.at(j) == '\\') {
                        result.erase(j, 1);
                        len--;
                    }
                    j++;
                }
            }
        }

        Node LoadString(istream& input) {
            std::string result;
            int num = 0;
            const char c = input.peek();
            if (c == '\"') {
                result += '\"';
            }
            for (std::string line; !(std::getline(input, line, '"').eof()); ) {

                num++;

                result += line;

                std::string shifts_line;

                while (input.peek() == ' ') {
                    shifts_line += input.get();
                }
                char c_temp;
                input >> c_temp;
                if ((c_temp == ',') || (c_temp == '}') || (c_temp == ']')) {
                    input.putback(c_temp);
                    ParseString(result);
                    return Node(move(result));
                }
                else if (!input.eof()) {
                    result += '\"';
                    result += shifts_line;
                    input.putback(c_temp);
                }

                if ((num % 2 != 0) && (num > 1)) {
                    ParseString(result);
                    return Node(move(result));
                }

            }

            if (result.empty()) {
                throw json::ParsingError("Data corrupted: waited for String, but no correct input.");
            }

            ParseString(result);
            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;

            std::string key;
            char c = input.get();

            while( input.peek()!= '}') {
                while (!(c == '\"')) {
                    input >> c;
                    if (c == '}') {
                        return Node(move(result));
                    }
                }
                std::getline(input, key, '"');
                while (!(c == ':')) {
                    input >> c;
                }
                result.insert({ key, LoadNode(input) });
                key.clear();
            }

            if (input.eof()) {
                throw json::ParsingError("Data corrupted: waited for Array, but no correct input.");
            }

            while (!(c == '}')) {
                input >> c;
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {
                std::string temp_line;
                std::getline(input, temp_line, ',');
                if (temp_line.substr(0, 3) != "ull") {
                    throw json::ParsingError("Data corrupted: waited for Bool, but no correct input.");
                }
                input.putback(temp_line.at(temp_line.size() - 1));
                return Node(nullptr);
            }
            else if ((c == 't') || (c == 'f')) {
                input.putback(c);
                return LoadBool(input);
            }
            else if ((std::isdigit(c)) || (c == '-')) {
                input.putback(c);
                return LoadInt(input);
            }
            else {
                input.putback(c);
                return LoadString(input);
            }
        }

    }  // namespace

    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }
    // Перегрузка функции PrintValue для вывода значений int
    void PrintValue(int i, std::ostream& out) {
        out << i;
    }
    // Перегрузка функции PrintValue для вывода значений bool
    void PrintValue(bool i, std::ostream& out) {
        out << std::boolalpha << i;
    }
    // Перегрузка функции PrintValue для вывода значений string
    void PrintValue(std::string_view i, std::ostream& out) {
        out << "\"";
        for (const char c : i) {
            if (c == '\r') {
                out << '\\';
                out << 'r';
            }
            else if (c == '\n') {
                out << '\\';
                out << 'n';
            }
            else if ((c == '\"') || (c == '\\')) {
                out << '\\';
                out << c;
            }
            else {
                out << c;
            }
        }
        out << "\"";
    }
    // Перегрузка функции PrintValue для вывода значений double
    void PrintValue(double i, std::ostream& out) {
        out << i;
    }
    // Перегрузка функции PrintValue для вывода значений Array
    void PrintValue(const Array& i, std::ostream& out) {
        out << i;
    }
    // Перегрузка функции PrintValue для вывода значений Dict
    void PrintValue(const Dict& i, std::ostream& out) {
        out << i;
    }

    Node::Node() :
        node_(nullptr)
    {
    }

    Node::Node(std::nullptr_t node) :
        node_(node) {
    }

    Node::Node(int node) :
        node_(node) {

    }
    Node::Node(double node) :
        node_(node) {

    }

    Node::Node(std::string node) :
        node_(move(node)) {

    }

    Node::Node(bool node) :
        node_(node) {
    }

    Node::Node(Array node) :
        node_(node) {
    }
    Node::Node(Dict node) :
        node_(node) {
    }

    //хранится ли внутри значение некоторого типа

    bool Node::IsInt() const {
        return std::holds_alternative< int >(node_);
    }
    bool Node::IsDouble() const {//Возвращает true, если в Node хранится int либо double.
        return ((std::holds_alternative< int >(node_)) || (std::holds_alternative< double >(node_)));
    }
    bool Node::IsPureDouble() const { //Возвращает true, если в Node хранится double.
        return std::holds_alternative< double >(node_);
    }
    bool Node::IsBool() const {
        return std::holds_alternative< bool >(node_);
    }
    bool Node::IsString() const {
        return std::holds_alternative< std::string >(node_);
    }
    bool Node::IsNull() const {
        return std::holds_alternative< std::nullptr_t >(node_);
    }
    bool Node::IsArray() const {
        return std::holds_alternative< Array >(node_);
    }
    bool Node::IsMap() const {
        return std::holds_alternative< Dict >(node_);
    }

    //возвращают хранящееся внутри Node значение заданного типа
    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(node_);
        }
        else {
            throw std::logic_error("Dude its not an int -_-");
        }
    }
    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(node_);
        }
        else {
            throw std::logic_error("Dude its not a bool -_-");
        }
    }
    //Возвращает значение типа double, если внутри хранится double либо int.В последнем случае возвращается приведённое в double значение.
    double Node::AsDouble() const { 
        if (IsDouble()) {
            if (IsInt()) {
                return std::get<int>(node_);
            }
            return std::get<double>(node_);
        }
        else {
            throw std::logic_error("Dude its not a double or int -_-");
        }
    }
    const std::string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(node_);
        }
        else {
            throw std::logic_error("Dude its not a string -_-");
        }
    }
    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(node_);
        }
        else {
            throw std::logic_error("Dude its not an array -_-");
        }
    }
    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(node_);
        }
        else {
            throw std::logic_error("Dude its not a map -_-");
        }
    }

    //перегруженные операторы сравнения
    bool Node::operator==(const Node& right) const {
        if ((IsInt()) && (right.IsInt())) {
            return AsInt() == right.AsInt();
        }
        else if ((IsInt()) && (right.IsDouble())) {
            return false;
        }
        else if ((IsBool()) && (right.IsBool())) {
            return AsBool() == right.AsBool();
        }
        else if ((IsDouble()) && (right.IsDouble())) {
            return std::fabs(AsDouble() - right.AsDouble()) < std::numeric_limits<double>::epsilon();
        }
        else if ((IsDouble()) && (right.IsInt())) {
            return false;
        }
        else if ((IsString()) && (right.IsString())) {
            return AsString() == right.AsString();
        }
        else if ((IsArray()) && (right.IsArray())) {
            return AsArray() == right.AsArray();
        }
        else if ((IsMap()) && (right.IsMap())) {
            return AsMap() == right.AsMap();
        }
        else if ((IsNull()) && (right.IsNull())) {
            return true;
        }
        return false;
    }
    bool Node::operator!=(const Node& right) const {
        return !(*this == right);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& right) const {
        return (this->GetRoot() == right.GetRoot());
    }
    bool Document::operator!=(const Document& right) const {
        return !(*this == right);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value) { PrintValue(value, out); },
            node.GetValue());
    }

    std::ostream& operator<< (std::ostream& out, Dict dict) {
        out << "{\"";
        int num = 0;
        for (auto p : dict) {
            if (num > 0) {
                out << ", \"";
            }
            num = 1;
            out << p.first << "\": ";
            PrintNode(p.second, out);
        }
        out << "}";
        return out;
    }

    std::ostream& operator<< (std::ostream& out, json::Array array) {
        out << "[";
        bool i = false;
        for (auto p : array) {
            if (!i) {
                i = true;
            }
            else {
                out << ", ";
            }
            PrintNode(p, out);
        }
        out << "]";
        return out;
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

}  // namespace json