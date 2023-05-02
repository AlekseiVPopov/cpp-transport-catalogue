#include "json.h"


namespace json {

    using namespace std;
    using namespace std::string_view_literals;

    namespace {

        using Number = std::variant<int, double>;

        Node LoadNode(istream &input);

        Node LoadNull(istream &input) {

            string value;

            while (std::isalpha(input.peek())) {
                value.push_back(static_cast<char>(input.get()));
            }

            if (value == "null"s) return nullptr;
            throw ParsingError("Null parsing error");
        }


// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream &input) {
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

        Node LoadNumber(std::istream &input) {
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

        Node LoadBool(istream &input) {
            std::string value;

            while (std::isalpha(input.peek())) {
                value.push_back(static_cast<char>(input.get()));
            }


            if (value == "true") return true;
            if (value == "false") return false;

            throw ParsingError("Bool parsing error");
        }

        Node LoadArray(istream &input) {
            Array result;
            char c;
            input >> c;

            if (input.eof()) {
                throw ParsingError("Array parsing error"s);
            }

            for (; !input.eof() && c != ']'; input >> c) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            return {std::move(result)};
        }

        Node LoadDict(istream &input) {
            Dict result;
            char c;
            input >> c;

            if (input.eof()) {
                throw ParsingError("Dict parsing error"s);
            }


            for (; !input.eof() && c != '}'; input >> c) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({std::move(key), LoadNode(input)});
            }

            return {std::move(result)};
        }

        Node LoadNode(istream &input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            } else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace


    template<class T>
    Node::Node(T value) : value_(std::move(value)){

    }

    bool Node::IsInt() const {
        return holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return holds_alternative<double>(value_) || holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value_);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return holds_alternative<std::string>(value_);
    }

    bool Node::IsNull() const {
        return holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()) throw std::logic_error("Type mismatch - not an int");
        return std::get<int>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) throw std::logic_error("Type mismatch - not a bool");
        return std::get<bool>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) throw std::logic_error("Type mismatch - neither double nor int");
        if (IsInt()) return static_cast<double>(std::get<int>(value_));
        return std::get<double>(value_);
    }

    const std::string &Node::AsString() const {
        if (!IsString()) throw std::logic_error("Type mismatch - not a string");
        return std::get<std::string>(value_);
    }

    const Array &Node::AsArray() const {
        if (!IsArray()) throw std::logic_error("Type mismatch - not an array");
        return std::get<Array>(value_);
    }

    const Dict &Node::AsMap() const {
        if (!IsMap()) throw std::logic_error("Type mismatch - not a map");
        return std::get<Dict>(value_);
    }

    const Node::Value &Node::GetValue() const {
        return value_;
    }

    bool Node::operator==(const Node &rhs) const {
        return value_ == rhs.value_;
    }

    bool Node::operator!=(const Node &rhs) const {
        return !(value_ == rhs.value_);
    }


    Document::Document(Node root)
            : root_(std::move(root)) {
    }

    const Node &Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document &rhs) const {
        return root_ == rhs.root_;
    }

    bool Document::operator!=(const Document &rhs) const {
        return !(root_ == rhs.root_);
    }

    Document Load(istream &input) {
        return Document{LoadNode(input)};
    }

    void ValuePrinter::operator()(std::nullptr_t) {
        out << "null"sv;
    }

    void ValuePrinter::operator()(const std::string &value) {
        out << "\""sv;
        for (const char &c: value) { /* экранируем \n, \r, \", \t, \\ */
            if (c == '\n') {
                out << "\\n"sv;
                continue;
            }
            if (c == '\r') {
                out << "\\r"sv;
                continue;
            }
            if (c == '\"') out << "\\"sv;
            if (c == '\t') {
                out << "\t"sv;
                continue;
            }
            if (c == '\\') out << "\\"sv;
            out << c;
        }
        out << "\""sv;
    }

    void ValuePrinter::operator()(int value) {
        out << value;
    }

    void ValuePrinter::operator()(double value) {
        out << value;
    }

    void ValuePrinter::operator()(bool value) {
        out << std::boolalpha << value;
    }

    void ValuePrinter::operator()(const Array &array) {
        out << "["sv;
        bool first = true;
        for (const auto &elem: array) {
            if (first) {
                first = false;
            } else {
                out << ", "sv;
            }
            std::visit(ValuePrinter{out}, elem.GetValue());
        }
        out << "]"sv;
    }

    void ValuePrinter::operator()(const Dict &dict) {
        out << "{"sv;
        bool first = true;
        for (auto &[key, node]: dict) {
            if (first) {
                first = false;
            } else {
                out << ", "sv;
            }
            out << "\"" << key << "\":"sv;
            std::visit(ValuePrinter{out}, node.GetValue());
        }
        out << "}"sv;
    }

    void Print(const Document &doc, std::ostream &out) {
        std::visit(ValuePrinter{out}, doc.GetRoot().GetValue());
    }

}  // namespace json
