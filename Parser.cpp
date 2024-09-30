#include <numeric> // For std::accumulate

#include "Parser.h"

#include "Lexer.h"
#include "nlohmann/json.hpp"

std::optional<GlobalStatementNode> Parser::parse_global_statement_at(const uint32_t requested_pos)
{
    // Try as procedure
    std::optional<ProcedureDefinitionNode> try_proc = parse_procedure_definition_at(requested_pos);
    if (try_proc.has_value()) {
        return GlobalStatementNode(static_cast<ASTNode>(try_proc.value()));
    }

    // Try as global static var
    std::optional<StaticVarDefinitionNode> try_staticvar = parse_static_var_definition_at(requested_pos);
    if (try_staticvar.has_value()) {
        return GlobalStatementNode(static_cast<ASTNode>(try_staticvar.value()));
    }

    return std::optional<GlobalStatementNode>(); // Parsing Failed
}

std::optional<ProgramNode> Parser::parse_program()
{
    std::vector<GlobalStatementNode> globals{};
    uint32_t current_pos = 0;
    
    while (current_pos < tokens.size()) {
        std::optional<GlobalStatementNode> try_global = parse_global_statement_at(current_pos);
        if (!try_global.has_value()) {
            return std::optional<ProgramNode>();
        }

        globals.push_back(try_global.value());
    }

    return ProgramNode(globals);
}

std::optional<ProcedureDefinitionNode> Parser::parse_procedure_definition_at(const uint32_t requested_pos)
{
    uint32_t current_pos = requested_pos;

    // 'proc '
    if (get_token_at(current_pos).type != Token::Type::PROC)
        return std::optional<ProcedureDefinitionNode>(); // Failed to parse
    current_pos += 1;
    
    // ID
    Token id_token = get_token_at(current_pos);
    if (id_token.type != Token::Type::ID)
        return std::optional<ProcedureDefinitionNode>(); // Failed to parse
    current_pos += 1;

    // () // params
    std::optional<ParametersNode> params = parse_parameters_at(current_pos);
    if (!params.has_value())
        return std::optional<ProcedureDefinitionNode>(); // Failed to parse
    current_pos += params.value().get_token_length();
    
    // ->
    if (get_token_at(current_pos).type != Token::Type::RIGHTARROW)
        return std::optional<ProcedureDefinitionNode>(); // Failed to parse
    current_pos += 1;

    // return type
    std::optional<BasicType> ret_type = parse_basic_type_from_token(get_token_at(current_pos));
    if (!ret_type.has_value())
        return std::optional<ProcedureDefinitionNode>(); // Failed to parse
    current_pos += 1;

    // Block
    std::optional<BlockNode> block = parse_block_at(current_pos);
    if (!block.has_value())
        return std::optional<ProcedureDefinitionNode>(); // Failed to parse
    

    return ProcedureDefinitionNode(id_token.value, params.value(), ret_type.value(), block.value());
}

std::optional<StaticVarDefinitionNode> Parser::parse_static_var_definition_at(const uint32_t requested_pos)
{
    uint32_t current_pos = requested_pos;

    // expect 'staticvar '
    if (get_token_at(current_pos).type != Token::Type::STATICVAR)
        return std::optional<StaticVarDefinitionNode>(); // Failed to parse
    current_pos += 1;

    // expect id
    Token id_token = get_token_at(current_pos);
    if (id_token.type != Token::Type::ID)
        return std::optional<StaticVarDefinitionNode>(); // Failed to parse
    current_pos += 1;

    // expect ':'
    if (get_token_at(current_pos).type != Token::Type::COLON)
        return std::optional<StaticVarDefinitionNode>(); // Failed to parse
    current_pos += 1;

    // expect valid type
    std::optional<BasicType> ret_type = parse_basic_type_from_token(get_token_at(current_pos));
    if (!ret_type.has_value())
        return std::optional<StaticVarDefinitionNode>(); // Failed to parse
    current_pos += 1;

    // expect ';'
    if (get_token_at(current_pos).type != Token::Type::SEMICOLON)
        return std::optional<StaticVarDefinitionNode>(); // Failed to parse
    current_pos += 1;

    return StaticVarDefinitionNode(id_token.value, ret_type.value());
}

std::optional<ParameterNode> Parser::parse_parameter_at(const uint32_t pos)
{
    uint32_t current_pos = pos;

    // expect 'ID'
    Token id_token = get_token_at(current_pos);
    if (id_token.type != Token::Type::ID)
        return std::optional<ParameterNode>(); // Failed to parse
    current_pos += 1;

    // expect ':'
    if (get_token_at(current_pos).type != Token::Type::COLON)
        return std::optional<ParameterNode>(); // Failed to parse
    current_pos += 1;

    // expect valid type
    std::optional<BasicType> ret_type = parse_basic_type_from_token(get_token_at(current_pos));
    if (!ret_type.has_value())
        return std::optional<ParameterNode>(); // Failed to parse
    current_pos += 1;

    return ParameterNode(id_token.value, ret_type.value());
}

std::optional<ParametersNode> Parser::parse_parameters_at(const uint32_t pos)
{
    uint32_t current_pos = pos;
    std::vector<ParameterNode> parameters_list{};

    std::optional try_param = parse_parameter_at(current_pos);
    if (try_param.has_value()) {
        parameters_list.push_back(try_param.value());

        while(true) {
            // expect ','
            if (get_token_at(current_pos).type != Token::Type::COMMA)
                break;
            current_pos += 1;  

            // expect Parameter
            try_param = parse_parameter_at(current_pos);
            if (!try_param.has_value()) 
                break;
            parameters_list.push_back(try_param.value());
            current_pos += try_param.value().get_token_length();  
        }
    }


    return ParametersNode(parameters_list);
}

std::optional<BlockNode> Parser::parse_block_at(const uint32_t requested_pos)
{
    uint32_t current_pos = requested_pos;

    // expect '{'
    if (get_token_at(current_pos).type != Token::Type::LCURLY)
        return std::optional<BlockNode>(); // Failed to parse
    current_pos += 1;

    // parse statements
    std::vector<StatementNode> statements;
    while (true) {
        std::optional<StatementNode> try_statement = parse_statement_at(current_pos);
        if (!try_statement.has_value())
            break;
        statements.push_back(try_statement.value());
    }
    
    // expect '}'
    if (get_token_at(current_pos).type != Token::Type::LCURLY)
        return std::optional<BlockNode>(); // Failed to parse
    current_pos += 1;


    return BlockNode(statements);
}

std::optional<AssignmentNode> Parser::parse_assignment_at(const uint32_t requested_pos)
{
    uint32_t current_pos = requested_pos;

    // expect ID
    Token id_token = get_token_at(current_pos);
    if (id_token.type != Token::Type::ID)
        return std::optional<AssignmentNode>(); // Failed to parse
    current_pos += 1;

    // expect '='
    if (get_token_at(current_pos).type != Token::Type::ASSIGN)
        return std::optional<AssignmentNode>(); // Failed to parse
    current_pos += 1;

    // expect expr
    std::optional<ExpressionNode> expr = parse_expression_at(current_pos);
    if (!expr.has_value())
        return std::optional<AssignmentNode>(); // Failed to parse
    

    return AssignmentNode(id_token.value, expr.value());
}

std::optional<StatementNode> Parser::parse_statement_at(const uint32_t requested_pos)
{
    uint32_t current_pos = requested_pos;

    // expect expr
    std::optional<ExpressionNode> expr = parse_expression_at(current_pos);
    if (!expr.has_value())
        return std::optional<StatementNode>(); // Failed to parse
    
    // expect ';'
    if (get_token_at(current_pos).type != Token::Type::SEMICOLON)
        return std::optional<StatementNode>(); // Failed to parse
    current_pos += 1;

    // Return Nothing
    return std::optional<StatementNode>();
}

std::optional<ExpressionNode> Parser::parse_expression_at(const uint32_t requested_pos)
{
    // Try assingment
    std::optional<AssignmentNode> try_assign = parse_assignment_at(requested_pos);
    if (try_assign.has_value()) {
        return ExpressionNode(try_assign.value());
    }

    // Try Sum
    std::optional<SumNode> try_sum = parse_sum_at(requested_pos);
    if (try_sum.has_value()) {
        return ExpressionNode(try_sum.value());
    }

    // Try Sub
    std::optional<SubNode> try_sub = parse_sub_at(requested_pos);
    if (try_sub.has_value()) {
        return ExpressionNode(try_sub.value());
    }

    // Try as single Term
    std::optional<TermNode> try_term = parse_term_at(requested_pos);
    if (try_term.has_value()) {
        return ExpressionNode(try_term.value());
    }

    return std::optional<ExpressionNode>();
}

std::optional<SumNode> Parser::parse_sum_at(const uint32_t pos)
{
    std::optional<TermNode> try_term = parse_term_at(pos);
    if (!try_term.has_value())
        return std::optional<SumNode>(); // Nothing

    if (get_token_at(pos + try_term.value().get_token_length()).type != Token::Type::PLUS)
        return std::optional<SumNode>(); // Nothing

    std::optional<ExpressionNode> try_expr = parse_expression_at(
        pos + try_term.value().get_token_length() + 1
    );
    if (!try_expr.has_value())
        return std::optional<SumNode>(); // Nothing
    
    return SumNode(try_term.value(), try_expr.value());
}

std::optional<SubNode> Parser::parse_sub_at(const uint32_t pos)
{
    std::optional<TermNode> try_term = parse_term_at(pos);
    if (!try_term.has_value())
        return std::optional<SubNode>(); // Nothing

    if (get_token_at(pos + try_term.value().get_token_length()).type != Token::Type::MINUS)
        return std::optional<SubNode>(); // Nothing

    std::optional<ExpressionNode> try_expr = parse_expression_at(
        pos + try_term.value().get_token_length() + 1
    );
    if (!try_expr.has_value())
        return std::optional<SubNode>(); // Nothing
    
    return SubNode(try_term.value(), try_expr.value());
}

// Term -> (UnOp Primary) | Primary
std::optional<TermNode> Parser::parse_term_at(const uint32_t pos)
{
    if (
        get_token_at(pos).type == Token::Type::PLUS 
        || get_token_at(pos).type == Token::Type::MINUS
        || get_token_at(pos).type == Token::Type::TILDA
    ) {
        std::optional<PrimaryNode> try_primary = parse_primary_at(pos + 1);
        if (try_primary.has_value()) {
            switch (get_token_at(pos).type)
            {
            case Token::Type::PLUS: 
                return TermNode(try_primary.value(), UnaryOperator::PLUS);
            case Token::Type::MINUS: 
                return TermNode(try_primary.value(), UnaryOperator::MINUS);
            case Token::Type::TILDA: 
                return TermNode(try_primary.value(), UnaryOperator::NOT);
            default:
                break;
            }
        }
    }
    else {
        std::optional<PrimaryNode> try_primary = parse_primary_at(pos);
        if (try_primary.has_value()) {
            return TermNode(try_primary.value());
        }
    }

    // Return Nothing else
    return std::optional<TermNode>();
}

std::optional<PrimaryNode> Parser::parse_primary_at(const uint32_t pos)
{
    if (
        get_token_at(pos).type == Token::Type::ID
        || get_token_at(pos).type == Token::Type::NUMERIC_LITERAL
    ) {
        return PrimaryNode(get_token_at(pos));
    }
    
    // Return Nothing else
    return std::optional<PrimaryNode>();
}

Token Parser::get_token_at(const uint32_t pos)
{
    return Token();
}

std::optional<Parser::BasicType> Parser::parse_basic_type_from_token(const Token token)
{
    if (token.type != Token::Type::BASIC_TYPE)
        return std::optional<BasicType>();

    if (token.value == "u8")
        return BasicType::U8;
    else if (token.value == "u32")
        return BasicType::U32;
    else if (token.value == "nil")
        return BasicType::NIL;
    else {
        // Incorrect type
        return std::optional<Parser::BasicType>();
    }
}

uint32_t ExpressionNode::get_token_length() const
{
    return child.get_token_length();
}

uint32_t StatementNode::get_token_length() const
{
    return 1 + expr.get_token_length();
}

uint32_t AssignmentNode::get_token_length() const
{
    return 2 + expr.get_token_length();
}

uint32_t SumNode::get_token_length() const
{
    return left_term.get_token_length() + 1 + right_expr.get_token_length();
}

uint32_t SubNode::get_token_length() const
{
    return left_term.get_token_length() + 1 + right_expr.get_token_length();
}

uint32_t TermNode::get_token_length() const
{
    return primary.get_token_length() + (unOp == UnaryOperator::NONE ? 0 : 1);
}

uint32_t PrimaryNode::get_token_length() const
{
    return 1;
}

uint32_t BlockNode::get_token_length() const
{
    return 2 + std::accumulate(statements.begin(), statements.end(), 0, [](int acc, StatementNode st) {
        return acc + st.get_token_length();
    });
}

uint32_t ParameterNode::get_token_length() const
{
    return 3;
}

uint32_t ParametersNode::get_token_length() const
{
    // Summ all the lengths of parameters and add COMMAS between them
    return 2 + std::accumulate(params.begin(), params.end(), 0, [](int sum, ParameterNode p) {
        return sum + p.get_token_length();
    }) + std::min(0, static_cast<int>(params.size() - 1)); // TODO 
}

uint32_t GlobalStatementNode::get_token_length() const
{
    return child.get_token_length();
}

uint32_t ProcedureDefinitionNode::get_token_length() const
{
    return 3 + parameters.get_token_length() + instructions_block.get_token_length();
}

uint32_t StaticVarDefinitionNode::get_token_length() const
{
    return 5;
}

uint32_t ProgramNode::get_token_length() const
{
    return std::accumulate(global_statements.begin(), global_statements.end(), 0, [](int sum, GlobalStatementNode gs) {
        return sum += gs.get_token_length();
    });
}

uint32_t ASTNode::get_token_length() const
{
    return 0;
}

// nlohmann::json ASTNode::generate_json() {
//     return nlohmann::json::array();
// }