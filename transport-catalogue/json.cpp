#include "json.h"

using namespace std;
using namespace literals;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c;
            for (c = '['; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') { throw ParsingError("LoadArray"s); }

            return Node(move(result));
        }

        Node LoadNumber(istream& input) {

            string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (isdigit(input.peek())) {
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
            }
            else {
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
                        return Node(stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        //std::string
        Node LoadString(istream& input) {

            auto it = istreambuf_iterator<char>(input);
            auto end = istreambuf_iterator<char>();
            string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error"s);
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error"s);
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
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(move(s));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;
            for (c = '{'; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c != '}') { throw ParsingError("LoadDict"s); }
            return Node(move(result));
        }

        Node LoadBool(istream& input) {
            string bool_pars;
            size_t bool_size = input.peek() == 't' ? 4 : 5;
            for (size_t i = 0; i < bool_size; ++i) {
                bool_pars += input.get();
            }
            if (bool_pars == "true"sv) { return Node(true); }
            if (bool_pars == "false"sv) { return Node(false); }
            throw ParsingError("LoadBool"s);
        }

        Node LoadNull(istream& input) {
            string null_pars;
            size_t null_size = 4;
            for (size_t i = 0; i < null_size; ++i) {
                null_pars += input.get();
            }
            // cout << null_pars << endl;
            if (null_pars == "null"sv) {
                return Node(nullptr);
            }
            else {
                throw ParsingError("LoadNull"s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            try {
                if (c == '[') {
                    return LoadArray(input);
                }
                else if (c == ']') {
                    throw ParsingError("LoadNode");
                }
                else if (c == '{') {
                    return LoadDict(input);
                }
                else if (c == '}') {
                    throw ParsingError("LoadNode");
                }
                else if (c == '"') {
                    return LoadString(input);
                }
                else if (c == '-' || isdigit(static_cast<unsigned char>(c))) {
                    input.putback(c);
                    return LoadNumber(input);
                }
                else if (c == 't' || c == 'f') {
                    input.putback(c);
                    return LoadBool(input);
                }
                else if (c == 'n') {
                    input.putback(c);
                    return LoadNull(input);
                }
            }
            catch (...) {
                throw ParsingError("LoadNode"s);
            }
            return LoadNode(input);
        }


    }  // namespace

    Node::Node(Array array) : value_(move(array)) {}

    Node::Node(Dict map) : value_(move(map)) {}

    Node::Node(int value) : value_(value) {}

    Node::Node(std::string value) : value_(move(value)) {}

    Node::Node(double value) : value_(value) {}

    Node::Node(bool value) : value_(value) {}

    bool Node::IsInt() const {
        return holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return holds_alternative<int>(value_) || holds_alternative<double>(value_);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value_);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return holds_alternative<string>(value_);
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(value_);
    }

    const Array& Node::AsArray() const {
        if (holds_alternative<Array>(value_)) {
            return get<Array>(value_);
        }
        else {
            throw logic_error("AsArray");
        }
    }

    const Dict& Node::AsMap() const {
        if (holds_alternative<Dict>(value_)) {
            return get<Dict>(value_);
        } throw logic_error("AsMap");
    }

    int Node::AsInt() const {
        if (holds_alternative<int>(value_)) {
            return get<int>(value_);
        } throw logic_error("AsInt");
    }

    const string& Node::AsString() const {
        if (holds_alternative<string>(value_)) {
            return get<string>(value_);
        } throw logic_error("AsString");
    }

    bool Node::AsBool() const {
        if (holds_alternative<bool>(value_)) {
            return get<bool>(value_);
        } throw logic_error("AsBool");
    }

    double Node::AsDouble() const {
        if (holds_alternative<double>(value_)) {
            return get<double>(value_);
        }
        else if (holds_alternative<int>(value_)) {
            return static_cast<double>(get<int>(value_));
        }throw logic_error("AsDouble");
    }

    bool Node::operator==(const Node& other) const {
        return value_ == other.value_;
    }

    bool Node::operator!=(const Node& other) const {
        return value_ != other.value_;
    }

    const Node::Value& Node::GetValue() const { return value_; }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& other) const {
        return root_ == other.root_;
    }

    bool Document::operator!=(const Document& other) const {
        return root_ != other.root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintNode(const Node& node, const PrintContext& ctx);

    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintValue(const string& value, const PrintContext& ctx) {
        ctx.PrintIndent();
        ostream& out = ctx.out;
        out << "\""sv;
        for (char c : value) {
            switch (c) {
            case '\"': out << R"(\")"sv; break;
            case '\r': out << R"(\r)"sv; break;
            case '\n': out << R"(\n)"sv; break;
            case '\\': out << R"(\\)"sv; break;
            case '\t': out << "\t"sv; break;
            default: out << c;
            }
        }
        out << "\""sv;
    }

    void PrintValue(const Array& array, const PrintContext& ctx) {
        ostream& out = ctx.out;
        auto ctx1 = ctx;
        out << "[\n"sv;
        bool first = true;
        for (const Node& node : array) {
            if (first) {
                first = false;
            }
            else { out << ",\n"sv; }
            PrintNode(node, ctx.Indented());
        }
        out << '\n';
        ctx1.PrintIndent();
        out << ']';
    }

    void PrintValue(const Dict& map, const PrintContext& ctx) {
        ctx.PrintIndent();
        ostream& out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        for (const auto& [key, node] : map) {
            if (first) {
                first = false;
            }
            else { out << ",\n"sv; }
            ctx.PrintIndent();
            ctx.PrintIndent();
            out << "\""sv;
            out << key;
            out << "\": "sv;
            PrintNode(node, ctx.Indented());
        }
        out << '\n';
        ctx.PrintIndent();
        out <<'}';
    }

    void PrintValue(const bool& value, const PrintContext& ctx) {
        ctx.out << (value ? "true"sv : "false"sv);
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) { PrintValue(value, ctx); },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json