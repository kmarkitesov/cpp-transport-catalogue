#include "json_builder.h"

namespace json {


KeyValueContext::KeyValueContext(Builder& builder)
    : builder_(builder) {}

DictItemContext KeyValueContext::Value(Node value) {
    builder_.Value(std::move(value));
    return DictItemContext(builder_);
}

DictItemContext KeyValueContext::StartDict() {
    builder_.StartDict();
    return DictItemContext(builder_);
}

ArrayItemContext KeyValueContext::StartArray() {
    builder_.StartArray();
    return ArrayItemContext(builder_);
}


DictItemContext::DictItemContext(Builder& builder)
    : builder_(builder) {}

KeyValueContext DictItemContext::Key(std::string key) {
    builder_.Key(std::move(key));
    return KeyValueContext(builder_);
}

Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}


ArrayItemContext::ArrayItemContext(Builder& builder)
    : builder_(builder) {}

ArrayItemContext ArrayItemContext::Value(Node value) {
    builder_.Value(std::move(value));
    return *this;
}

DictItemContext ArrayItemContext::StartDict() {
    builder_.StartDict();
    return DictItemContext(builder_);
}

ArrayItemContext ArrayItemContext::StartArray() {
    builder_.StartArray();
    return *this;
}

Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}


KeyValueContext Builder::Key(std::string key) {
    if (nodes_stack_.empty() || !GetCurrentContainer()->IsDict()) {
        throw std::logic_error("Key outside of dictionary");
    }

    auto& dict = const_cast<Dict&>(GetCurrentContainer()->AsDict());
    auto [it, inserted] = dict.emplace(std::move(key), Node(nullptr));
    if (!inserted) {
        throw std::logic_error("Duplicate key");
    }
    nodes_stack_.push_back(&it->second);
    return KeyValueContext(*this);
}

Builder& Builder::Value(Node value) {
    if (nodes_stack_.empty()) {
        if (IsComplete()) {
            throw std::logic_error("Value after complete object");
        }
        root_ = std::move(value);
        return *this;
    }

    auto current = GetCurrentContainer();

    if (current->IsNull()) {
        *current = std::move(value);
        nodes_stack_.pop_back();
    } else if (current->IsArray()) {
        auto& array = const_cast<Array&>(current->AsArray());
        array.push_back(std::move(value));
    } else {
        throw std::logic_error("Value in invalid context");
    }

    return *this;
}

DictItemContext Builder::StartDict() {
    Node dict_node = Node(Dict{});

    if (nodes_stack_.empty()) {
        if (IsComplete()) {
            throw std::logic_error("StartDict after complete object");
        }
        root_ = std::move(dict_node);
        nodes_stack_.push_back(&root_);
    } else {
        auto current = GetCurrentContainer();

        if (current->IsNull()) {
            *current = std::move(dict_node);
            nodes_stack_.pop_back();
            nodes_stack_.push_back(&*current);
        } else if (current->IsArray()) {
            auto& array = const_cast<Array&>(current->AsArray());
            array.push_back(std::move(dict_node));
            nodes_stack_.push_back(&array.back());
        } else {
            throw std::logic_error("StartDict in invalid context");
        }
    }
    return DictItemContext(*this);
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !GetCurrentContainer()->IsDict()) {
        throw std::logic_error("EndDict without StartDict");
    }
    nodes_stack_.pop_back();
    return *this;
}

ArrayItemContext Builder::StartArray() {
    Node array_node = Node(Array{});

    if (nodes_stack_.empty()) {
        if (IsComplete()) {
            throw std::logic_error("StartArray after complete object");
        }
        root_ = std::move(array_node);
        nodes_stack_.push_back(&root_);
    } else {
        auto current = GetCurrentContainer();

        if (current->IsNull()) {
            *current = std::move(array_node);
            nodes_stack_.pop_back();
            nodes_stack_.push_back(&*current);
        } else if (current->IsArray()) {
            auto& array = const_cast<Array&>(current->AsArray());
            array.push_back(std::move(array_node));
            nodes_stack_.push_back(&array.back());
        } else {
            throw std::logic_error("StartArray in invalid context");
        }
    }
    return ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !GetCurrentContainer()->IsArray()) {
        throw std::logic_error("EndArray without StartArray");
    }
    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
    if (!IsComplete() || !nodes_stack_.empty()) {
        throw std::logic_error("Build incomplete object");
    }
    return std::move(root_);
}

bool Builder::IsComplete() const {
    return !root_.IsNull();
}

Node* Builder::GetCurrentContainer() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("No current container");
    }
    return nodes_stack_.back();
}

} // namespace json