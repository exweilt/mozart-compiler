#include "Lexer.h"

// Not all of them are here
std::vector<std::pair<std::string, Token::Type>> Token::symbol_to_token_type_map = {
    {"proc ", Token::Type::PROC},
    {"staticvar ", Token::Type::STATICVAR},
    {":=", Token::Type::ASSIGN},
    {":", Token::Type::COLON},
    {";", Token::Type::SEMICOLON},
    {"return ", Token::Type::RETURN},
    {"->", Token::Type::RIGHTARROW},
    {"(", Token::Type::LPAREN},
    {")", Token::Type::RPAREN},
    {"{", Token::Type::LCURLY},
    {"}", Token::Type::RCURLY},
    {"+", Token::Type::PLUS},
    {"-", Token::Type::MINUS},
    {"*", Token::Type::ASTERISK},
    {"/", Token::Type::SLASH},
    {"=", Token::Type::EQUAL},
};

std::vector<std::string> Token::basic_data_types = {
    "u8",
    "u32",
    "nil",
};

bool Lexer::starts_with_at_pos(const std::string &prefix)
{
    if (pos + prefix.length() > source_text.length())
        return false;

    std::string rest = source_text.substr(pos);

    return rest.starts_with(prefix);
}

std::optional<Token> Lexer::try_symbol_as_token(const std::string &symbol, const Token::Type of_type)
{
    if (starts_with_at_pos(symbol))
        return Token(of_type, symbol);
    else
        return std::optional<Token>();
}

Token Lexer::consume(Token token)
{
    pos += token.value.length();
    assert(pos <= source_text.length());

    return token;
}

std::optional<Token> Lexer::parse_token()
{
    // Skip spaces and newlines
    while (source_text[pos] == ' ' || source_text[pos] == '\n' || source_text[pos] == '\0') {
        if (pos + 1 >= source_text.length())
            return std::optional<Token>();
        pos += 1;
    }

    // Strip comments
    if (source_text[pos] == '#') {
        while (source_text[pos++] != '\n');
    }

    // Handling simple ones
    for (const auto &bind : Token::symbol_to_token_type_map) {
        if (!starts_with_at_pos(bind.first))
            continue;

        return consume(Token(bind.second, bind.first));
    }

    // Handling numerical tokens
    unsigned int number_len = 0;
    for (; pos + number_len < source_text.length(); number_len++) {
        if (!std::isdigit(static_cast<unsigned char>(source_text[pos + number_len])))
            break;
    }
    if (number_len > 0) {
        return consume(Token(Token::Type::NUMERIC_LITERAL, source_text.substr(pos, number_len)));
    }

    // Handling identifiers
    if (can_id_start_with(source_text[pos])) {
        unsigned int identifier_len = 1;
        for (; pos + identifier_len < source_text.length(); identifier_len++) {
            if (!can_id_start_with(source_text[pos + identifier_len]) 
                && !std::isdigit(static_cast<unsigned char>(source_text[pos + identifier_len]))) {
                break;
                }
        }
        std::string id_symbol = source_text.substr(pos, identifier_len);
        
        //using data_types = Token::basic_data_types;
        if (std::find(Token::basic_data_types.begin(), Token::basic_data_types.end(), id_symbol) != Token::basic_data_types.end()) {
            // Treat as basic data type
            return consume(Token(Token::Type::BASIC_TYPE, id_symbol));
        }

        return consume(Token(Token::Type::ID, id_symbol));
    }

    return std::optional<Token>(); // return nothing
}

std::vector<Token> Lexer::tokenize()
{
    while (true) {
        std::optional<Token> token = parse_token();
        if (!token.has_value())
            break;

        tokens.push_back(token.value());
    }
    return tokens;
}

nlohmann::json Lexer::serialize_to_json()
{
    nlohmann::json json_array = nlohmann::json::array();
    for (const auto &token : tokens) {

        nlohmann::json json_object = {
            {"type", magic_enum::enum_name(token.type)},
            {"value", token.value}
        };

        // Add the JSON object to the JSON array
        json_array.push_back(json_object);
    }
    return json_array;
}

std::vector<Token> Lexer::load_from_json_str(std::string source)
{
    // Flush the current tokens
    tokens.clear();

    nlohmann::json json_array = nlohmann::json::parse(source, nullptr, false);
    if (json_array.is_discarded()) {
        std::cerr << "Could not parse the tokens, json structure is not correct.";
        exit(EXIT_FAILURE);
    }
    for (const auto &json_item : json_array) {
        assert(json_item.contains("type") && json_item.contains("value"));

        Token parsed_token{};
        std::optional<Token::Type> type = magic_enum::enum_cast<Token::Type>(json_item["type"].get<std::string>());
        std::string value = json_item["value"].get<std::string>();

        if (!type.has_value()) {
            std::cerr << "Could not parse the token <<" << json_item["type"] << ">>."
                << std::endl << "There is no such token type.";
            exit(EXIT_FAILURE);
        }

        parsed_token.type = type.value();
        parsed_token.value = value;

        tokens.push_back(parsed_token);
    }
    return tokens;
}
