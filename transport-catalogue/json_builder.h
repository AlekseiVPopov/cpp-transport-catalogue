#pragma once

#include <optional>
#include <utility>
#include "json.h"

namespace json {

    class BaseContext;

    class DictContext;

    class DictKeyContext;

    class ArrayContext;

    class Builder {
    public:
        Builder() = default;

        DictContext StartDict();

        Builder &EndDict();

        ArrayContext StartArray();

        Builder &EndArray();

        DictKeyContext Key(std::string key);

        Builder &Value(Node value);

        json::Document Build();

        virtual ~Builder() = default;


    private:
        Node root_;
        std::vector<Node *> nodes_stack_;
        std::optional<std::string> key_ = std::nullopt;
        Node *current_node_ = nullptr;
    };


    class BaseContext {
    public:
        BaseContext(Builder &builder) : builder_(builder) {}

    protected:
        Builder &builder_;

    };

    class DictContext : public BaseContext {
    public:
        DictContext(Builder &builder) : BaseContext(builder) {}

        DictKeyContext Key(std::string key);

        Builder &EndDict();
    };

    class DictKeyContext : public BaseContext {
    public:
        DictKeyContext(Builder &builder) : BaseContext(builder) {}

        DictContext Value(Node value);

        DictContext StartDict();

        ArrayContext StartArray();
    };

    class ArrayContext : public BaseContext {
    public:
        ArrayContext(Builder &builder) : BaseContext(builder) {}

        ArrayContext Value(Node value);

        DictContext StartDict();

        ArrayContext StartArray();

        Builder &EndArray();
    };


}