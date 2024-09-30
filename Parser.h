#pragma once

#include <numeric> // For std::accumulate

#include <vector>
#include <optional>

#include "Lexer.h"
#include "nlohmann/json.hpp"

enum class UnaryOperator {
    NONE,
    PLUS,
    MINUS,
    NOT
};

class ASTNode {
public:
    // To make the class polymorph to use dynamic_cast
    virtual ~ASTNode() = default;

    virtual uint32_t get_token_length() const;
    // virtual nlohmann::json generate_json() const;
};

// Forwards
class ProgramNode;
class GlobalStatementNode;
class ProcedureDefinitionNode;
class StaticVarDefinitionNode;
class ParametersNode;
class BlockNode;
class StatementNode;
class ExpressionNode;
class AssignmentNode;
class SumNode;
class SubNode;
class TermNode;
class PrimaryNode;
class ParameterNode;


class Parser {
public:
    enum class BasicType {
        NIL,
        U8,
        U32,
    };

    std::optional<ProgramNode> parse_program();

    Parser(std::vector<Token> t) : tokens(t) {};
private:
    std::vector<Token> tokens;

    std::optional<GlobalStatementNode> parse_global_statement_at(const uint32_t pos);
    std::optional<ProcedureDefinitionNode> parse_procedure_definition_at(const uint32_t pos);
    std::optional<StaticVarDefinitionNode> parse_static_var_definition_at(const uint32_t pos);
    std::optional<ParameterNode> parse_parameter_at(const uint32_t pos);
    std::optional<ParametersNode> parse_parameters_at(const uint32_t pos);
    std::optional<BlockNode> parse_block_at(const uint32_t pos);
    std::optional<StatementNode> parse_statement_at(const uint32_t pos);
    std::optional<ExpressionNode> parse_expression_at(const uint32_t pos);
    std::optional<AssignmentNode> parse_assignment_at(const uint32_t pos);
    std::optional<SumNode> parse_sum_at(const uint32_t pos);
    std::optional<SubNode> parse_sub_at(const uint32_t pos);
    std::optional<TermNode> parse_term_at(const uint32_t pos);
    std::optional<PrimaryNode> parse_primary_at(const uint32_t pos);

    Token get_token_at(const uint32_t pos);
    std::optional<BasicType> parse_basic_type_from_token(const Token token);

};

// TODO: slicind
class ExpressionNode : public ASTNode {
    ASTNode child;
public:
    ExpressionNode(ASTNode c) : child(c) {};

    virtual uint32_t get_token_length() const override;
    // virtual nlohmann::json generate_json() const override;
};

class StatementNode : public ASTNode {
    ExpressionNode expr;
    bool is_return_statement = false;
public:
    StatementNode(ExpressionNode e, bool ret_st = false) : expr(e), is_return_statement(ret_st) {};

    virtual uint32_t get_token_length() const override;
    // virtual nlohmann::json generate_json() const override;

};

class PrimaryNode : public ASTNode {
    Token token;
public:
    PrimaryNode(Token t) : token(t) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};

class TermNode : public ASTNode {
    PrimaryNode primary;
    UnaryOperator unOp = UnaryOperator::NONE;
public:
    TermNode(PrimaryNode p, UnaryOperator uo = UnaryOperator::NONE) : primary(p), unOp(uo) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};

class AssignmentNode : public ASTNode {
    std::string id;
    ExpressionNode expr;
public:
    AssignmentNode(std::string ID, ExpressionNode e) : id(ID), expr(e) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};

class SumNode : public ASTNode {
    TermNode left_term;
    ExpressionNode right_expr;
public:
    SumNode(TermNode t, ExpressionNode e) : left_term(t), right_expr(e) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};

class SubNode : public ASTNode {
    TermNode left_term;
    ExpressionNode right_expr;
public:
    SubNode(TermNode t, ExpressionNode e) : left_term(t), right_expr(e) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};




class BlockNode : public ASTNode {
    std::vector<StatementNode> statements;

public:
    BlockNode(std::vector<StatementNode> s) : statements(s) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};


class ParameterNode : public ASTNode {
    std::string param_id;
    Parser::BasicType param_type;
public:
    ParameterNode(std::string id, Parser::BasicType t) : param_id(id), param_type(t) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};

class ParametersNode : public ASTNode {
    std::vector<ParameterNode> params;
public:
    ParametersNode(std::vector<ParameterNode> p) : params(p) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};

class GlobalStatementNode : public ASTNode {
    ASTNode child;
public:
    GlobalStatementNode(ASTNode c) : child(c) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};

class ProcedureDefinitionNode : public ASTNode {
    std::string proc_id;
    ParametersNode parameters;
    Parser::BasicType return_type;
    BlockNode instructions_block;
public:
    ProcedureDefinitionNode(std::string id, ParametersNode p, Parser::BasicType ret, BlockNode b)
        : proc_id(id), parameters(p), return_type(ret), instructions_block(b) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};

class StaticVarDefinitionNode : public ASTNode {
    std::string var_id;
    Parser::BasicType var_type;

public:
    StaticVarDefinitionNode(std::string new_id, Parser::BasicType new_type)
        : var_id(new_id), var_type(new_type) {};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};


class ProgramNode : public ASTNode {
    std::vector<GlobalStatementNode> global_statements;
public:
    ProgramNode(std::vector<GlobalStatementNode> glob_s) : global_statements(glob_s){};

    virtual uint32_t get_token_length() const override;
    // nlohmann::json generate_json() const override;
};

