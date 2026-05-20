#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace {

enum class Side {
    Client,
    Server,
};

enum class PanType {
    Id,
    Char64,
    Int8,
    Int16,
    Int32,
    Int64,
    String,
    Bool,
    Unsupported,
};

struct Arg {
    PanType type = PanType::Unsupported;
    std::string type_name;
    std::string name;
};

struct Msg {
    Side side = Side::Client;
    std::string prefix;
    std::string type;
    std::string prefix_id;
    std::string type_id;
    std::vector<Arg> args;
};

void fail(const std::string& message)
{
    throw std::runtime_error(message);
	return;
}

std::string read_text_file(const std::string& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        fail("cannot open " + path);
    }

    std::ostringstream out;
    out << input.rdbuf();
    return out.str();
}

void write_text_file(const std::string& path, const std::string& text)
{
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        fail("cannot create " + path);
    }

    output << text;
    if (!output) {
        fail("cannot write " + path);
    }
}

std::string sanitize_ident(std::string_view value)
{
    std::string result;
    result.reserve(value.size() + 1);

    for (unsigned char ch : value) {
        result.push_back(std::isalnum(ch) ? static_cast<char>(ch) : '_');
    }

    if (result.empty() || std::isdigit(static_cast<unsigned char>(result[0]))) {
        result.insert(result.begin(), '_');
    }

    return result;
}

PanType parse_type_name(std::string_view name)
{
    if (name == "id")     return PanType::Id;
    if (name == "char64") return PanType::Char64;
    if (name == "int8")   return PanType::Int8;
    if (name == "int16")  return PanType::Int16;
    if (name == "int32")  return PanType::Int32;
    if (name == "int64")  return PanType::Int64;
    if (name == "string") return PanType::String;
    if (name == "bool")   return PanType::Bool;
    return PanType::Unsupported;
}

std::size_t fixed_type_size(PanType type)
{
    switch (type) {
        case PanType::Id:     return 4;
        case PanType::Char64: return 8;
        case PanType::Int8:   return 1;
        case PanType::Int16:  return 2;
        case PanType::Int32:  return 4;
        case PanType::Int64:  return 8;
        case PanType::Bool:   return 1;
        default:              fail("type has no fixed size");
    }

	return 0;
}

const char* write_func(PanType type)
{
    switch (type) {
        case PanType::Id:     return "binmsg_write_id";
        case PanType::Char64: return "binmsg_write_char64";
        case PanType::Int8:   return "binmsg_write_i8";
        case PanType::Int16:  return "binmsg_write_i16";
        case PanType::Int32:  return "binmsg_write_i32";
        case PanType::Int64:  return "binmsg_write_i64";
        case PanType::String: return "binmsg_write_string";
        case PanType::Bool:   return "binmsg_write_bool";
        default:              return nullptr;
    }
}

const char* read_func(PanType type)
{
    switch (type) {
        case PanType::Id:    return "binmsg_read_u32";
        case PanType::Int8:  return "binmsg_read_i8";
        case PanType::Int16: return "binmsg_read_i16";
        case PanType::Int32: return "binmsg_read_i32";
        case PanType::Int64: return "binmsg_read_i64";
        case PanType::Bool:  return "binmsg_read_bool";
        default:             return nullptr;
    }
}

class Parser {
public:
    explicit Parser(std::string_view text)
        : text_(text)
    {}

    bool eof()
    {
        skip_ws_and_comments();
        return pos_ >= text_.size();
    }

    std::string parse_word(const char* what)
    {
        skip_ws_and_comments();

        const std::size_t start = pos_;
        while (pos_ < text_.size() && is_word_char(text_[pos_])) {
            ++pos_;
        }

        if (pos_ == start) {
            fail(std::string("expected ") + what);
        }

        return std::string(text_.substr(start, pos_ - start));
    }

    void expect(char ch)
    {
        skip_ws_and_comments();
        if (peek() != ch) {
            std::string got = peek() ? std::string(1, peek()) : "EOF";
            fail("expected '" + std::string(1, ch) + "', got '" + got + "'");
        }
        ++pos_;
    }

    char peek() const
    {
        return pos_ < text_.size() ? text_[pos_] : '\0';
    }

    void skip_ws_and_comments()
    {
        while (true) {
            while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) {
                ++pos_;
            }

            if (peek() != '#') {
                return;
            }

            while (pos_ < text_.size() && text_[pos_] != '\n') {
                ++pos_;
            }
        }
    }

private:
    static bool is_word_char(char raw_ch)
    {
        const unsigned char ch = static_cast<unsigned char>(raw_ch);
        return std::isalnum(ch) || ch == '_' || ch == '.' || ch == '-';
    }

    std::string_view text_;
    std::size_t pos_ = 0;
};

std::vector<Msg> parse_proto(std::string_view text)
{
    Parser parser(text);
    std::vector<Msg> messages;

    while (!parser.eof()) {
        const std::string side_word = parser.parse_word("message side");

        Msg msg;
        if (side_word == "client") {
            msg.side = Side::Client;
        } else if (side_word == "server") {
            msg.side = Side::Server;
        } else {
            fail("unknown message side '" + side_word + "'");
        }

        msg.prefix = parser.parse_word("message prefix");
        parser.expect(':');
        msg.type = parser.parse_word("message type");
        parser.expect('(');

        msg.prefix_id = sanitize_ident(msg.prefix);
        msg.type_id = sanitize_ident(msg.type);

        while (true) {
            parser.skip_ws_and_comments();
            if (parser.peek() == ')') {
                parser.expect(')');
                break;
            }

            Arg arg;
            arg.type_name = parser.parse_word("argument type");
            arg.type = parse_type_name(arg.type_name);

            parser.skip_ws_and_comments();
            if (parser.peek() == ',' || parser.peek() == ')') {
                arg.name = "arg" + std::to_string(msg.args.size());
            } else {
                arg.name = sanitize_ident(parser.parse_word("argument name"));
            }

            msg.args.push_back(std::move(arg));

            parser.skip_ws_and_comments();
            if (parser.peek() == ',') {
                parser.expect(',');
            }
        }

        parser.expect(';');
        messages.push_back(std::move(msg));
    }

    return messages;
}

std::string send_func_name(const Msg& msg)
{
    std::string name = msg.prefix_id + "_" + msg.type_id;
    if (msg.args.empty()) {
        return name + "_send";
    }

    for (const Arg& arg : msg.args) {
        name += "_" + arg.name;
    }

    return name;
}

std::string match_func_name(const Msg& msg)
{
    return msg.prefix_id + "_" + msg.type_id;
}

std::string accessor_func_name(const Msg& msg, const Arg& arg)
{
    return msg.prefix_id + "_" + msg.type_id + "_" + arg.name;
}

std::string eq_accessor_func_name(const Msg& msg, const Arg& arg)
{
    return accessor_func_name(msg, arg) + "_is";
}

std::string type_check_func_name(const Msg& msg)
{
    return msg.prefix_id + "_binmsg_type_is_" + msg.type_id;
}

std::string params(const Msg& msg)
{
    std::string result;
    for (std::size_t i = 0; i < msg.args.size(); ++i) {
        result += (i == 0 ? ": " : " : ");
        result += "var ";
        result += msg.args[i].name;
    }
    return result;
}

std::string fdecl(const std::string& name, const Msg* params_from = nullptr)
{
    return "~ fdecl " + name + "(" + (params_from ? params(*params_from) : "") + ")\n";
}

std::string runtime_fdecls()
{
    return R"(~ fdecl socket_connect_raw(: var host : var port)
~ fdecl socket_alive()
~ fdecl socket_set_dead()
~ fdecl socket_read_next()
~ fdecl socket_close()
~ fdecl socket_fd()

~ fdecl binmsg_begin(: var prefix : var type)
~ fdecl binmsg_write_i8(: var value)
~ fdecl binmsg_write_i16(: var value)
~ fdecl binmsg_write_i32(: var value)
~ fdecl binmsg_write_i64(: var value)
~ fdecl binmsg_write_id(: var value)
~ fdecl binmsg_write_bool(: var value)
~ fdecl binmsg_write_string(: var value)
~ fdecl binmsg_write_char64(: var value)
~ fdecl binmsg_send()

~ fdecl binmsg_prefix()
~ fdecl binmsg_type()
~ fdecl binmsg_prefix_is(: var prefix)
~ fdecl binmsg_type_is(: var type)
~ fdecl binmsg_id()
~ fdecl binmsg_len()
~ fdecl binmsg_flags()
~ fdecl binmsg_read_i8(: var offset)
~ fdecl binmsg_read_i16(: var offset)
~ fdecl binmsg_read_i32(: var offset)
~ fdecl binmsg_read_i64(: var offset)
~ fdecl binmsg_read_u32(: var offset)
~ fdecl binmsg_read_bool(: var offset)
~ fdecl binmsg_string_eq(: var offset : var expected)
~ fdecl binmsg_string_size(: var offset)
~ fdecl binmsg_char64_eq(: var offset : var expected)

~ fdecl memcpy(: var dst : var src : var size)
~ fdecl memset(: var dst : var value : var size)
~ fdecl strlen(: var str)
~ fdecl strcmp(: var lhs : var rhs)
~ fdecl print_num(: var value)
~ fdecl print_str(: var str)
~ fdecl scan_num()

~ fdecl equal(: var lhs : var rhs)
~ fdecl not_equal(: var lhs : var rhs)
~ fdecl smaller(: var lhs : var rhs)
~ fdecl bigger(: var lhs : var rhs)
~ fdecl smaller_or_eq(: var lhs : var rhs)
~ fdecl bigger_or_eq(: var lhs : var rhs)
~ fdecl random_mod(: var max)
~ fdecl streq(: var lhs : var rhs)

)";
}

void check_supported_types(const std::vector<Msg>& messages)
{
    for (const Msg& msg : messages) {
        for (const Arg& arg : msg.args) {
            if (arg.type == PanType::Unsupported) {
                fail("unsupported type '" + arg.type_name + "' in " + msg.prefix + ":" + msg.type);
            }
        }
    }
}

std::string emit_offset_advance(const Arg& arg, std::string_view offset_var)
{
    std::string out;
    out += "    ~ ";
    out += offset_var;
    out += " = ";
    out += offset_var;
    out += " + ";

    if (arg.type == PanType::String) {
        out += "call binmsg_string_size(: ";
        out += offset_var;
        out += ")\n";
    } else {
        out += std::to_string(fixed_type_size(arg.type));
        out += "\n";
    }

    return out;
}

std::string emit_prefix_send_wrappers(const std::vector<Msg>& messages, std::string* inc)
{
    std::unordered_set<std::string> emitted;
    std::string lang;

    for (const Msg& msg : messages) {
        if (!emitted.insert(msg.prefix_id).second) {
            continue;
        }

        const std::string name = msg.prefix_id + "_send_binmsg";
        lang += "~ func " + name + "()\n";
        lang += "{\n";
        lang += "    ~ return call binmsg_send()\n";
        lang += "}\n\n";

        *inc += fdecl(name);
    }

    *inc += "\n";
    return lang;
}

std::string emit_client_message(const Msg& msg, std::string* inc)
{
    const std::string name = send_func_name(msg);
    *inc += fdecl(name, &msg);

    std::string out;
    out += "~ func " + name + "(" + params(msg) + ")\n";
    out += "{\n";
    out += "    ~ call binmsg_begin(: \"" + msg.prefix + "\" : \"" + msg.type + "\")\n";

    for (const Arg& arg : msg.args) {
        out += "    ~ call ";
        out += write_func(arg.type);
        out += "(: ";
        out += arg.name;
        out += ")\n";
    }

    out += "    ~ return call " + msg.prefix_id + "_send_binmsg()\n";
    out += "}\n\n";
    return out;
}

std::string emit_server_type_checker(const Msg& msg, std::string* inc)
{
    const std::string name = type_check_func_name(msg);
    *inc += fdecl(name);

    std::string out;
    out += "~ func " + name + "()\n";
    out += "{\n";
    out += "    ~ if (call binmsg_type_is(: \"" + msg.type + "\")) {\n";

    bool has_string = false;
    size_t fixed_size = 0;
    for (const Arg& arg : msg.args) {
        if (arg.type == PanType::String) {
            has_string = true;
        } else {
            fixed_size += fixed_type_size(arg.type);
        }
    }

    if (!has_string) {
        out += "        ~ return call equal(: call binmsg_len() : " + std::to_string(fixed_size) + ")\n";
    } else {
        out += "        ~ var offset = 0\n";
        size_t string_index = 0;
        for (const Arg& arg : msg.args) {
            if (arg.type == PanType::String) {
                std::string string_size_var = "string_size_" + std::to_string(string_index++);
                out += "        ~ var " + string_size_var + " = call binmsg_string_size(: offset)\n";
                out += "        ~ if (call equal(: " + string_size_var + " : 0)) {\n";
                out += "            ~ return 0\n";
                out += "        }\n";
                out += "        ~ offset = offset + " + string_size_var + "\n";
            } else {
                out += "        ~ offset = offset + " + std::to_string(fixed_type_size(arg.type)) + "\n";
            }
        }
        out += "        ~ return call equal(: offset : call binmsg_len())\n";
    }

    out += "    }\n\n";
    out += "    ~ return 0\n";
    out += "}\n\n";
    return out;
}

std::string emit_string_or_char64_predicate(const Msg& msg, const Arg& arg, std::size_t arg_index, std::string* inc)
{
    const std::string name = eq_accessor_func_name(msg, arg);
    const char* runtime_eq = arg.type == PanType::String ? "binmsg_string_eq" : "binmsg_char64_eq";

    *inc += "~ fdecl " + name + "(: var expected)\n";

    std::string out;
    out += "~ func " + name + "(: var expected)\n";
    out += "{\n";
    out += "    ~ var offset = 0\n";

    for (std::size_t i = 0; i < arg_index; ++i) {
        out += emit_offset_advance(msg.args[i], "offset");
    }

    out += "    ~ return call ";
    out += runtime_eq;
    out += "(: offset : expected)\n";
    out += "}\n\n";
    return out;
}

std::string emit_fixed_accessor(const Msg& msg, const Arg& arg, std::size_t arg_index, std::string* inc)
{
    const char* reader = read_func(arg.type);
    if (!reader) {
        return {};
    }

    const std::string name = accessor_func_name(msg, arg);
    *inc += fdecl(name);

    std::string out;
    out += "~ func " + name + "()\n";
    out += "{\n";
    out += "    ~ var offset = 0\n";

    for (std::size_t i = 0; i < arg_index; ++i) {
        out += emit_offset_advance(msg.args[i], "offset");
    }

    out += "    ~ return call ";
    out += reader;
    out += "(: offset)\n";
    out += "}\n\n";
    return out;
}

std::string emit_server_message(const Msg& msg, std::string* inc)
{
    std::string out = emit_server_type_checker(msg, inc);

    for (std::size_t i = 0; i < msg.args.size(); ++i) {
        const Arg& arg = msg.args[i];
        if (arg.type == PanType::String || arg.type == PanType::Char64) {
            out += emit_string_or_char64_predicate(msg, arg, i, inc);
        } else {
            out += emit_fixed_accessor(msg, arg, i, inc);
        }
    }

    return out;
}

void generate_lang(const std::vector<Msg>& messages, const std::string& lang_path, const std::string& langinc_path)
{
    check_supported_types(messages);

    std::string lang;
    std::string inc;

    lang += "// GENERATED FILE -- DO NOT EDIT\n\n";
    lang += runtime_fdecls();

    inc += "// GENERATED FILE -- DO NOT EDIT\n\n";
    lang += emit_prefix_send_wrappers(messages, &inc);

    for (const Msg& msg : messages) {
        if (msg.side == Side::Client) {
            lang += emit_client_message(msg, &inc);
        } else {
            lang += emit_server_message(msg, &inc);
        }
    }

    write_text_file(lang_path, lang);
    write_text_file(langinc_path, inc);
}

struct CliArgs {
    std::string input;
    std::string lang;
    std::string langinc;
};

void usage(const char* argv0)
{
    std::fprintf(stderr, "Usage: %s -i <proto.pan> --lang <out.lang> --langinc <out.langinc>\n", argv0);
}

CliArgs parse_cli(int argc, char** argv)
{
    CliArgs args;

    for (int i = 1; i < argc; ++i) {
        const std::string_view cur = argv[i];

        if (cur == "-i" && i + 1 < argc) {
            args.input = argv[++i];
        } else if (cur == "--lang" && i + 1 < argc) {
            args.lang = argv[++i];
        } else if (cur == "--langinc" && i + 1 < argc) {
            args.langinc = argv[++i];
        } else {
            usage(argv[0]);
            std::exit(EXIT_FAILURE);
        }
    }

    if (args.input.empty() || args.lang.empty() || args.langinc.empty()) {
        usage(argv[0]);
        std::exit(EXIT_FAILURE);
    }

    return args;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        const CliArgs args = parse_cli(argc, argv);
        const std::string text = read_text_file(args.input);
        const std::vector<Msg> messages = parse_proto(text);
        generate_lang(messages, args.lang, args.langinc);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "pan2lang: %s\n", error.what());
        return EXIT_FAILURE;
    }
}
