#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stdint.h>
#include "lexer.h"

typedef enum {
    // Literales y Expresiones Primarias
    AST_LITERAL_INT,
    AST_LITERAL_FLOAT,
    AST_LITERAL_STRING,
    AST_LITERAL_BOOL,
    AST_LITERAL_NULL,
    AST_EXPR_VARIABLE,

    // Expresiones Operacionales
    AST_EXPR_UNARY,         // -a, !b, ~c
    AST_EXPR_BINARY,        // a + b, x && y, i // j
    AST_EXPR_ASSIGNMENT,    // x = expr
    AST_CALL,

    // Sentencias (Statements)
    AST_STMT_EXPR,
    AST_STMT_BLOCK,
    AST_STMT_IF,
    AST_STMT_FOR,
    AST_STMT_WHILE,
    AST_STMT_RETURN,
    AST_STMT_VAR_DECL,
    AST_FUNC_DECL
} ASTNodeType;

typedef struct ASTNode ASTNode;

typedef struct {
    TokenType type;
    char* name;
} Param;

struct ASTNode {
    ASTNodeType type;
    int line;

    union {
        // Literals
        int64_t int_val;
        double float_val;
        bool bool_val;
        struct {
            char* value;
            int length;
        } string_val;

        // Expressions
        struct {
            char* name;
            int length;
        } variable;

        struct {
            TokenType op;
            ASTNode* operand;
        } unary;

        struct {
            TokenType op;
            ASTNode* left;
            ASTNode* right;
        } binary;

        struct {
            char* var_name;
            int name_length;
            ASTNode* value;
        } assignment;
        
        // Statements
        struct {
            TokenType var_type;
            char* var_name;
            int name_length;
            ASTNode* initializer;
        } var_decl;

        struct {
            ASTNode** statements;
            int count;
            int capacity;
        } block;

        struct {
            ASTNode* condition;
            ASTNode* then_branch;
            ASTNode* else_branch;
        } if_stmt;

        struct {
            char* var_name;
            int name_length;
            ASTNode* iterable;
            ASTNode* body;
        } for_stmt;

        struct {
            ASTNode* condition;
            ASTNode* body;
        } while_stmt;

        struct {
            ASTNode* value;
        } return_stmt;

        struct {
            ASTNode* expression;
        } expr_stmt;

        struct {
            ASTNode* callee;
            ASTNode** args;
            int arg_count;
        } call;

        struct {
            char* name;
            Param* params;
            int param_count;
            TokenType return_type; /* TOKEN_EOF means no explicit return type (void). */
            ASTNode* body;
        } func_decl;
    } as;
};

// Literal Constructors
ASTNode* ast_create_literal_int(int64_t value, int line);
ASTNode* ast_create_literal_float(double value, int line);
ASTNode* ast_create_literal_bool(bool value, int line);
ASTNode* ast_create_literal_string(const char* value, int length, int line);
ASTNode* ast_create_literal_null(int line);

// Expression Constructors
ASTNode* ast_create_variable(const char* name, int length, int line);
ASTNode* ast_create_unary(TokenType op, ASTNode* operand, int line);
ASTNode* ast_create_binary(TokenType op, ASTNode* left, ASTNode* right, int line);
ASTNode* ast_create_assignment(const char* var_name, int length, ASTNode* value, int line);

// Statement Constructors
ASTNode* ast_create_var_decl(TokenType var_type, const char* name, int length, ASTNode* init, int line);
ASTNode* ast_create_block(int line);
void ast_block_add_stmt(ASTNode* block_node, ASTNode* stmt);

ASTNode* ast_create_if(ASTNode* cond, ASTNode* then_branch, ASTNode* else_branch, int line);
ASTNode* ast_create_for(const char* var_name, int length, ASTNode* iterable, ASTNode* body, int line);
ASTNode* ast_create_while(ASTNode* cond, ASTNode* body, int line);
ASTNode* ast_create_return(ASTNode* value, int line);
ASTNode* ast_create_expr_stmt(ASTNode* expr, int line);

ASTNode* ast_create_call(ASTNode* callee, int line);
void     ast_call_add_arg(ASTNode* node, ASTNode* arg);

ASTNode* ast_create_func_decl(const char* name, int length, int line);
void     ast_func_add_param(ASTNode* node, TokenType type, const char* name, int length);
void     ast_func_set_body(ASTNode* node, ASTNode* body);

void ast_free(ASTNode* node);

#endif // AST_H
