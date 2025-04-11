#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final {
public:
   /* Реализуйте Node, используя std::variant */

    Node(const Node&) = default;
    Node(Node&&) noexcept = default;
    Node& operator=(const Node&) = default;
    Node& operator=(Node&&) noexcept = default;

    Node() : value_(std::nullptr_t()) {}
    Node(Array array) : value_(std::move(array)) {}
    Node(Dict map) : value_(std::move(map)) {}
    Node(int value) : value_(value) {}
    Node(double value) : value_(value) {}
    Node(std::string value) : value_(std::move(value)) {}
    Node(bool value) : value_(value) {}
    Node(std::nullptr_t) : value_(nullptr) {}

    bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
    bool IsArray() const { return std::holds_alternative<Array>(value_); }
    bool IsMap() const { return std::holds_alternative<Dict>(value_); }
    bool IsBool() const { return std::holds_alternative<bool>(value_); }
    bool IsInt() const { return std::holds_alternative<int>(value_); }
    bool IsPureDouble() const { return std::holds_alternative<double>(value_); }
    bool IsDouble() const { return IsInt() || IsPureDouble(); }
    bool IsString() const { return std::holds_alternative<std::string>(value_); }

    int AsInt() const {
        if (!IsInt()) throw std::logic_error("Not an int");
        return std::get<int>(value_);
    }

    bool AsBool() const {
        if (!IsBool()) throw std::logic_error("Not a bool");
        return std::get<bool>(value_);
    }

    double AsDouble() const {
        if (IsInt()) return static_cast<double>(std::get<int>(value_));
        if (!IsPureDouble()) throw std::logic_error("Not a double");
        return std::get<double>(value_);
    }

    const std::string& AsString() const {
        if (!IsString()) throw std::logic_error("Not a string");
        return std::get<std::string>(value_);
    }

    const Array& AsArray() const {
        if (!IsArray()) throw std::logic_error("Not an array");
        return std::get<Array>(value_);
    }

    const Dict& AsMap() const {
        if (!IsMap()) throw std::logic_error("Not a map");
        return std::get<Dict>(value_);
    }


    const Value& GetValue() const { return value_; }

    bool operator==(const Node& rhs) const { return value_ == rhs.value_; }
    bool operator!=(const Node& rhs) const { return !(*this == rhs); }

private:
    Value value_{nullptr};
};

class Document {
public:
    explicit Document(Node root) : root_(std::move(root)) {}

    const Node& GetRoot() const { return root_; }

    bool operator==(const Document& other) const {
        return root_ == other.root_;
    }

    bool operator!=(const Document& other) const {
        return !(*this == other);
    }

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json