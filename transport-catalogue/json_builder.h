#pragma once

#include <optional>
#include <utility>
#include "json.h"

namespace json {



    class Builder final {
        class BaseContext;

        class DictContext;

        class DictKeyContext;

        class ArrayContext;

    public:
        Builder() = default;

        DictContext StartDict();

        Builder &EndDict();

        ArrayContext StartArray();

        Builder &EndArray();

        DictKeyContext Key(std::string key);

        Builder &Value(Node value);

        json::Document Build();


    private:
        void StartContainer(bool is_array);

        Node root_;
        std::vector<Node *> nodes_stack_;
        std::optional<std::string> key_ = std::nullopt;
        Node *current_node_ = nullptr;
    };


    class Builder::BaseContext {
    public:
        BaseContext(Builder &builder) : builder_(builder) {}

    protected:
        Builder &builder_;

    };

    class Builder::DictContext : public BaseContext {
    public:
        DictContext(Builder &builder) : BaseContext(builder) {}

        DictKeyContext Key(std::string key);

        Builder &EndDict();
    };

    class Builder::DictKeyContext : public BaseContext {
    public:
        DictKeyContext(Builder &builder) : BaseContext(builder) {}

        DictContext Value(Node value);

        DictContext StartDict();

        ArrayContext StartArray();
    };

    class Builder::ArrayContext : public BaseContext {
    public:
        ArrayContext(Builder &builder) : BaseContext(builder) {}

        ArrayContext Value(Node value);

        DictContext StartDict();

        ArrayContext StartArray();

        Builder &EndArray();
    };
}