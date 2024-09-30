#pragma once

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <cctype>  // For std::isdigit
#include <cassert>
#include <algorithm>
#include <iostream>

#include "nlohmann/json.hpp"
#include "magic_enum.hpp"

struct Token {
    enum class Type {
        NONE,
        ID,
        PROC,
        LPAREN,
        RPAREN,
        SEMICOLON,
        COMMA,
        COLON,
        LEFTARROW, // <-
        RIGHTARROW, // ->
        BASIC_TYPE, // u8, u32, nil
        LCURLY,
        RCURLY,
        RETURN,
        ASTERISK,
        SLASH, // /
        PLUS,
        MINUS,
        STATICVAR,
        ASSIGN, // :=
        EQUAL, // =
        TILDA,
        NUMERIC_LITERAL,
    };

    static std::vector<std::pair<std::string, Token::Type>> symbol_to_token_type_map;
    static std::vector<std::string> basic_data_types;

    Type type;
    std::string value;

    Token(const Type t=Type::NONE, const std::string &val="") : type(t), value(val) {}
};

// #define TRY_SYMBOL_AS_TOKEN

class Lexer {
    int pos = 0;
    std::string source_text;

    bool starts_with_at_pos(const std::string &prefix);

    std::optional<Token> try_symbol_as_token(const std::string &symbol, const Token::Type of_type);

    // shifts pos
    Token consume(Token token);

    // Only a-z A-Z and _
    bool can_id_start_with(char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
    }

public:
    std::vector<Token> tokens;

    Lexer(const std::string &t = "") : source_text(t) {}

    // Non-pure, shifts pos. Returns no value if end of source_text
    std::optional<Token> parse_token();

    std::vector<Token> tokenize();

    nlohmann::json serialize_to_json();

    std::vector<Token> load_from_json_str(std::string source);
};