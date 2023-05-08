#include "json_builder.h"

namespace json {

    DictContext Builder::StartDict() {
        using namespace std::string_literals;

        if (!nodes_stack_.empty() && current_node_ &&
            !((current_node_->IsDict() && key_) || current_node_->IsArray())) {
            throw std::logic_error("Wrong call StartDict() for Builder"s);
        }

        if (nodes_stack_.empty()) { // just after ctor
            root_ = Node(Dict{});
            current_node_ = &root_;
            nodes_stack_.emplace_back(current_node_);
            return {*this};

        } else if (current_node_->IsArray()) {
            auto &arr = const_cast<Array &> (current_node_->AsArray());
            current_node_ = &arr.emplace_back(std::move(Dict{}));
            nodes_stack_.emplace_back(current_node_);
            return {*this};
        } else if (current_node_->IsDict()) {
            auto &dict = const_cast<Dict &> (current_node_->AsDict());
            dict[key_.value()] = std::move(Dict{});
            current_node_ = &dict.at(key_.value());
            nodes_stack_.emplace_back(current_node_);
            key_.reset();
        }
        return {*this};
    }

    Builder &Builder::EndDict() {
        using namespace std::string_literals;

        if (!current_node_->IsDict() || key_) {
            throw std::logic_error("Wrong call EndDict() for Builder"s);
        } else {
            nodes_stack_.pop_back();
            current_node_ = nodes_stack_.empty() ? &root_ : nodes_stack_.back();
        }

        return {*this};
    }

    ArrayContext Builder::StartArray() {
        using namespace std::string_literals;
        if (!nodes_stack_.empty() && current_node_ &&
            !((current_node_->IsDict() && key_) || current_node_->IsArray())) {
            throw std::logic_error("Wrong call StartDict() for Builder"s);
        }

        if (nodes_stack_.empty()) { // just after ctor
            root_ = Node(Array{});
            current_node_ = &root_;
            nodes_stack_.emplace_back(current_node_);
            return {*this};

        } else if (current_node_->IsArray()) {
            auto &arr = const_cast<Array &> (current_node_->AsArray());
            current_node_ = &arr.emplace_back(std::move(Array{}));
            nodes_stack_.emplace_back(current_node_);
            return {*this};
        } else if (current_node_->IsDict()) {
            auto &dict = const_cast<Dict &> (current_node_->AsDict());
            dict[key_.value()] = std::move(Array{});
            current_node_ = &dict.at(key_.value());
            nodes_stack_.emplace_back(current_node_);
            key_.reset();
        }

        return {*this};
    }

    Builder &Builder::EndArray() {
        using namespace std::string_literals;

        if (!current_node_->IsArray()) {
            throw std::logic_error("Wrong call EndArray() for Builder"s);
        } else {
            nodes_stack_.pop_back();
            current_node_ = nodes_stack_.empty() ? &root_ : nodes_stack_.back();
        }
        return {*this};
    }

    DictKeyContext Builder::Key(std::string key) {
        using namespace std::string_literals;

        if (!current_node_->IsDict() || key_) {
            throw std::logic_error("Wrong call Key() for Builder"s);
        } else {
            key_ = std::move(key);
        }
        return {*this};
    }

    Builder &Builder::Value(Node value) {
        using namespace std::string_literals;

        if (nodes_stack_.empty() && current_node_ == &root_){
            throw std::logic_error("Wrong call Value() for Builder"s);
        }
        if (!current_node_) {  // just after ctor
            root_ = Node(std::move(value));
            //nodes_stack_.emplace_back(current_node_);
            current_node_ = &root_;
            return {*this};
        } else if (current_node_->IsArray()) {
            auto &arr = const_cast<Array &> (current_node_->AsArray());
            arr.emplace_back(std::move(value));
            return {*this};
        } else if (current_node_->IsDict()) {
            auto &dict = const_cast<Dict &> (current_node_->AsDict());
            dict[key_.value()] = std::move(value);
            key_.reset();
        }
        return {*this};
    }

    json::Document Builder::Build() {
        using namespace std::string_literals;
        if ((!current_node_ && nodes_stack_.size() > 1) || (!nodes_stack_.empty() && (root_.IsDict() || root_.IsArray()))) {
            throw std::logic_error("Wrong call Build() for Builder"s);
        }
        return Document(std::move(root_));
    }


    DictKeyContext DictContext::Key(std::string key) {
        builder_.Key(std::move(key));
        return {builder_};
    }

    Builder &DictContext::EndDict() {
        builder_.EndDict();
        return {builder_};
    }

    DictContext DictKeyContext::Value(Node value) {
        builder_.Value(std::move(value));
        return {builder_};
    }

    DictContext DictKeyContext::StartDict() {
        builder_.StartDict();
        return {builder_};
    }

    ArrayContext DictKeyContext::StartArray() {
        builder_.StartArray();
        return {builder_};
    }

    ArrayContext ArrayContext::Value(Node value) {
        builder_.Value(std::move(value));
        return {builder_};
    }

    DictContext ArrayContext::StartDict() {
        builder_.StartDict();
        return {builder_};
    }

    ArrayContext ArrayContext::StartArray() {
        builder_.StartArray();
        return {builder_};
    }

    Builder &ArrayContext::EndArray() {
        builder_.EndArray();
        return {builder_};
    }

}