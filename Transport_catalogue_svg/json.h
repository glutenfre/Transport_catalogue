#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <cmath>
#include <limits>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Number = std::variant<int, double>;
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    std::ostream& operator<< (std::ostream& out, Dict dict);
    std::ostream& operator<< (std::ostream& out, Array array);

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        Node();
        Node(std::nullptr_t node);
        Node(int node);
        Node(double node);
        Node(std::string node);
        Node(bool node);
        Node(Array node);
        Node(Dict node);

        const Value& GetValue() const { return node_; }

        //хранится ли внутри значение некоторого типа
        bool IsInt() const;
        //Возвращает true, если в Node хранится int либо double.
        bool IsDouble() const; 
        //Возвращает true, если в Node хранится double.
        bool IsPureDouble() const; 
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        //возвращают хранящееся внутри Node значение заданного типа
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const; 
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        //перегруженные операторы сравнения
        bool operator==(const Node& right) const;
        bool operator!=(const Node& right) const;

    private:

        Value node_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& right) const;
        bool operator!=(const Document& right) const;
    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json