#include "json.h"
#include <sstream>
#include <stdexcept>
#include <cassert>

namespace json {

namespace {

Node LoadNode(std::istream& input);
void PrintValue(const Node& node, std::ostream& out);

void SkipWhitespace(std::istream& input) {
    while (isspace(input.peek())) input.get();
}

Node LoadNull(std::istream& input) {
    constexpr std::string_view NULL_TOKEN = "null";
    constexpr int NULL_LENGTH = NULL_TOKEN.size();
    std::string word;
    for (int i = 0; i < NULL_LENGTH; ++i) word += static_cast<char>(input.get());
    if (word != NULL_TOKEN) throw ParsingError("Expected 'null'");
    if (isalnum(input.peek())) {
        throw ParsingError("Invalid token after 'null'");
    }
    return Node(nullptr);
}

Node LoadBool(std::istream& input) {
    constexpr std::string_view TRUE_TOKEN = "true";
    constexpr std::string_view FALSE_TOKEN = "false";
    constexpr int TRUE_LENGTH = TRUE_TOKEN.size();
    constexpr int FALSE_LENGTH = FALSE_TOKEN.size();

    if (input.peek() == TRUE_TOKEN.front()) {
        std::string word;
        for (int i = 0; i < TRUE_LENGTH; ++i) word += static_cast<char>(input.get());
        if (word != TRUE_TOKEN) throw ParsingError("Expected 'true'");
        if (isalnum(input.peek())) throw ParsingError("Invalid token after 'true'");
        return Node(true);
    } else {
        std::string word;
        for (int i = 0; i < FALSE_LENGTH; ++i) word += static_cast<char>(input.get());
        if (word != FALSE_TOKEN) throw ParsingError("Expected 'false'");
        if (isalnum(input.peek())) throw ParsingError("Invalid token after 'false'");
        return Node(false);
    }
}

Node LoadNumber(std::istream& input) {
    std::string num_str;
    if (input.peek() == '-') num_str += static_cast<char>(input.get());

    while (isdigit(input.peek())) num_str += static_cast<char>(input.get());
    
    if (input.peek() == '.') {
        num_str += static_cast<char>(input.get());
        while (isdigit(input.peek())) num_str += static_cast<char>(input.get());
    }

    if (input.peek() == 'e' || input.peek() == 'E') {
        num_str += static_cast<char>(input.get());
        if (input.peek() == '+' || input.peek() == '-') num_str += static_cast<char>(input.get());
        while (isdigit(input.peek())) num_str += static_cast<char>(input.get());
    }

    try {
        if (num_str.find('.') != std::string::npos || num_str.find('e') != std::string::npos || num_str.find('E') != std::string::npos)
            return Node(stod(num_str));
        return Node(stoi(num_str));
    } catch (...) {
        throw ParsingError("Invalid number: " + num_str);
    }
}

Node LoadString(std::istream& input) {
    std::string s;
    char c;
    while (true) {
        if (!input.get(c)) {
            throw ParsingError("Unexpected end of input in string");
        }
        if (c == '"') {
            break;
        }
        if (c == '\\') {
            if (!input.get(c)) {
                throw ParsingError("Incomplete escape sequence in string");
            }
            switch (c) {
                case 'n': s.push_back('\n'); break;
                case 'r': s.push_back('\r'); break;
                case 't': s.push_back('\t'); break;
                case '"': s.push_back('"'); break;
                case '\\': s.push_back('\\'); break;
                default:
                    throw ParsingError("Unrecognized escape sequence");
            }
        } else if (c == '\n' || c == '\r') {
            throw ParsingError("Newline in string literal");
        } else {
            s.push_back(c);
        }
    }
    return Node(std::move(s));
}

Node LoadArray(std::istream& input) {
    Array result;
    SkipWhitespace(input);

    if (input.peek() == ']') {
        input.get();
        return Node(result);
    }

    while (true) {
        result.push_back(LoadNode(input));
        SkipWhitespace(input);

        int next = input.get();
        if (next == EOF) {
            throw ParsingError("Unexpected end of input in array");
        }

        char ch = static_cast<char>(next);
        if (ch == ']') break;
        if (ch != ',') throw ParsingError("Expected ',' or ']' in array");

        SkipWhitespace(input);
    }

    return Node(result);
}

Node LoadDict(std::istream& input) {
    Dict result;
    SkipWhitespace(input);

    if (input.peek() == '}') {
        input.get();
        return Node(result);
    }

    while (true) {
        SkipWhitespace(input);
        if (input.peek() != '"') {
            throw ParsingError("Expected \" at start of key in object");
        }
        input.get();
        std::string key = LoadString(input).AsString();
        SkipWhitespace(input);

        if (input.get() != ':') {
            throw ParsingError("Expected ':' after key in object");
        }
        SkipWhitespace(input);

        result[std::move(key)] = LoadNode(input);
        SkipWhitespace(input);

        int next = input.get();
        if (next == EOF) {
            throw ParsingError("Unexpected end of input in object");
        }

        char ch = static_cast<char>(next);
        if (ch == '}') break;
        if (ch != ',') throw ParsingError("Expected ',' or '}' in object");
        SkipWhitespace(input);
    }

    return Node(result);
}

Node LoadNode(std::istream& input) {
    SkipWhitespace(input);
    char ch = static_cast<char>(input.peek());

    if (ch == 'n') return LoadNull(input);
    if (ch == 't' || ch == 'f') return LoadBool(input);
    if (ch == '-' || isdigit(ch)) return LoadNumber(input);
    if (ch == '"') {
        input.get();
        return LoadString(input);
    }
    if (ch == '[') {
        input.get();
        return LoadArray(input);
    }
    if (ch == '{') {
        input.get();
        return LoadDict(input);
    }

    throw ParsingError("Unexpected character");
}

template <typename Value>
void PrintValue(const Value& value, std::ostream& out) {
    out << value;
}

void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null";
}

void PrintValue(bool value, std::ostream& out) {
    out << (value ? "true" : "false");
}

void PrintValue(const std::string& value, std::ostream& out) {
    out << '"';
    for (char ch : value) {
        switch (ch) {
            case '\\': out << "\\\\"; break;
            case '"':  out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << ch;
        }
    }
    out << '"';
}

void PrintValue(const Array& array, std::ostream& out) {
    out << '[';
    bool first = true;
    for (const auto& elem : array) {
        if (!first) out << ',';
        PrintValue(elem, out);
        first = false;
    }
    out << ']';
}

void PrintValue(const Dict& dict, std::ostream& out) {
    out << '{';
    bool first = true;
    for (const auto& [key, val] : dict) {
        if (!first) out << ',';
        PrintValue(key, out);
        out << ':';
        PrintValue(val, out);
        first = false;
    }
    out << '}';
}

void PrintValue(const Node& node, std::ostream& out) {
    std::visit([&out](const auto& val) {
        PrintValue(val, out);
    }, node.GetValue());
}

}  // namespace

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintValue(doc.GetRoot(), output);
}

}  // namespace json
