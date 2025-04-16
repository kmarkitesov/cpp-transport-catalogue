#pragma once

#include "json.h"
#include <stack>
#include <string>
#include <vector>
#include <utility>

namespace json {

class Builder;
class KeyValueContext;
class DictItemContext;
class ArrayItemContext;

class KeyValueContext {
public:
    KeyValueContext(Builder& builder);

    DictItemContext Value(Node value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();

private:
    Builder& builder_;
};

class DictItemContext {
public:
    DictItemContext(Builder& builder);

    KeyValueContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ArrayItemContext {
public:
    ArrayItemContext(Builder& builder);

    ArrayItemContext Value(Node value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;
};

class Builder {
public:
    Builder() = default;

    KeyValueContext Key(std::string key);
    Builder& Value(Node value);

    DictItemContext StartDict();
    Builder& EndDict();

    ArrayItemContext StartArray();
    Builder& EndArray();

    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::string key_;

    bool IsComplete() const;
    Node* GetCurrentContainer();
    void AddNode(Node node);

    friend class KeyValueContext;
    friend class DictItemContext;
    friend class ArrayItemContext;
};

} // namespace json