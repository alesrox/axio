#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static ASTNode* allocate_node(ASTNodeType type, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "[AST Error] Out of memory allocating node at line %d\n", line);
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->line = line;
    return node;
}

// Literals Constructors
ASTNode* ast_create_literal_int(int64_t value, int line) {
    ASTNode* node = allocate_node(AST_LITERAL_INT, line);
    node->as.int_val = value;
    return node;
}

ASTNode* ast_create_literal_float(double value, int line) {
    ASTNode* node = allocate_node(AST_LITERAL_FLOAT, line);
    node->as.float_val = value;
    return node;
}

ASTNode* ast_create_literal_bool(bool value, int line) {
    ASTNode* node = allocate_node(AST_LITERAL_BOOL, line);
    node->as.bool_val = value;
    return node;
}

ASTNode* ast_create_literal_string(const char* value, int length, int line) {
    ASTNode* node = allocate_node(AST_LITERAL_STRING, line);
    node->as.string_val.value = strndup(value, length);
    node->as.string_val.length = length;
    return node;
}

ASTNode* ast_create_literal_nil(int line) {
    return allocate_node(AST_LITERAL_NIL, line);
}

// Expressions
ASTNode* ast_create_variable(const char* name, int length, int line) {
    ASTNode* node = allocate_node(AST_EXPR_VARIABLE, line);
    node->as.variable.name = strndup(name, length);
    node->as.variable.length = length;
    return node;
}

ASTNode* ast_create_unary(TokenType op, ASTNode* operand, int line) {
    ASTNode* node = allocate_node(AST_EXPR_UNARY, line);
    node->as.unary.op = op;
    node->as.unary.operand = operand;
    return node;
}

ASTNode* ast_create_binary(TokenType op, ASTNode* left, ASTNode* right, int line) {
    ASTNode* node = allocate_node(AST_EXPR_BINARY, line);
    node->as.binary.op = op;
    node->as.binary.left = left;
    node->as.binary.right = right;
    return node;
}

ASTNode* ast_create_assignment(const char* var_name, int length, ASTNode* value, int line) {
    ASTNode* node = allocate_node(AST_EXPR_ASSIGNMENT, line);
    node->as.assignment.var_name = strndup(var_name, length);
    node->as.assignment.name_length = length;
    node->as.assignment.value = value;
    return node;
}

// Statements
ASTNode* ast_create_var_decl(TokenType var_type, const char* name, int length, ASTNode* init, int line) {
    ASTNode* node = allocate_node(AST_STMT_VAR_DECL, line);
    node->as.var_decl.var_type = var_type;
    node->as.var_decl.var_name = strndup(name, length);
    node->as.var_decl.name_length = length;
    node->as.var_decl.initializer = init;
    return node;
}

ASTNode* ast_create_block(int line) {
    ASTNode* node = allocate_node(AST_STMT_BLOCK, line);
    node->as.block.statements = NULL;
    node->as.block.count = 0;
    node->as.block.capacity = 0;
    return node;
}

void ast_block_add_stmt(ASTNode* block_node, ASTNode* stmt) {
    if (!block_node || block_node->type != AST_STMT_BLOCK || !stmt) return;

    if (block_node->as.block.capacity < block_node->as.block.count + 1) {
        int old_capacity = block_node->as.block.capacity;
        block_node->as.block.capacity = old_capacity < 8 ? 8 : old_capacity * 2;
        block_node->as.block.statements = (ASTNode**)realloc(
            block_node->as.block.statements,
            sizeof(ASTNode*) * block_node->as.block.capacity
        );
        if (!block_node->as.block.statements) {
            fprintf(stderr, "[AST Error] Out of memory expanding block statement list\n");
            exit(EXIT_FAILURE);
        }
    }

    block_node->as.block.statements[block_node->as.block.count++] = stmt;
}

ASTNode* ast_create_if(ASTNode* cond, ASTNode* then_branch, ASTNode* else_branch, int line) {
    ASTNode* node = allocate_node(AST_STMT_IF, line);
    node->as.if_stmt.condition = cond;
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* ast_create_while(ASTNode* cond, ASTNode* body, int line) {
    ASTNode* node = allocate_node(AST_STMT_WHILE, line);
    node->as.while_stmt.condition = cond;
    node->as.while_stmt.body = body;
    return node;
}

ASTNode* ast_create_for(const char* var_name, int length, ASTNode* iterable, ASTNode* body, int line) {
    ASTNode* node = allocate_node(AST_STMT_FOR, line);
    node->as.for_stmt.var_name = strndup(var_name, length);
    node->as.for_stmt.name_length = length;
    node->as.for_stmt.iterable = iterable;
    node->as.for_stmt.body = body;
    return node;
}

ASTNode* ast_create_return(ASTNode* value, int line) {
    ASTNode* node = allocate_node(AST_STMT_RETURN, line);
    node->as.return_stmt.value = value;
    return node;
}

ASTNode* ast_create_expr_stmt(ASTNode* expr, int line) {
    ASTNode* node = allocate_node(AST_STMT_EXPR, line);
    node->as.expr_stmt.expression = expr;
    return node;
}

ASTNode* ast_create_call(ASTNode* callee, int line) {
    ASTNode* node = allocate_node(AST_CALL, line);
    node->as.call.callee = callee;
    node->as.call.args = NULL;
    node->as.call.arg_count = 0;
    return node;
}

void ast_call_add_arg(ASTNode* node, ASTNode* arg) {
    if (!node || node->type != AST_CALL || !arg) return;
    node->as.call.arg_count++;
    node->as.call.args = (ASTNode**)realloc(
        node->as.call.args, 
        sizeof(ASTNode*) * node->as.call.arg_count
    );
    node->as.call.args[node->as.call.arg_count - 1] = arg;
}

ASTNode* ast_create_func_decl(const char* name, int length, int line) {
    ASTNode* node = allocate_node(AST_FUNC_DECL, line);
    node->as.func_decl.name = strndup(name, length);
    node->as.func_decl.params = NULL;
    node->as.func_decl.param_count = 0;
    node->as.func_decl.return_type = TOKEN_EOF;
    node->as.func_decl.body = NULL;
    return node;
}

void ast_func_add_param(ASTNode* node, TokenType type, const char* name, int length) {
    if (!node || node->type != AST_FUNC_DECL) return;
    node->as.func_decl.param_count++;
    node->as.func_decl.params = (Param*)realloc(
        node->as.func_decl.params, 
        sizeof(Param) * node->as.func_decl.param_count
    );
    Param* param = &node->as.func_decl.params[node->as.func_decl.param_count - 1];
    param->type = type;
    param->name = strndup(name, length);
}

void ast_func_set_body(ASTNode* node, ASTNode* body) {
    if (!node || node->type != AST_FUNC_DECL) return;
    node->as.func_decl.body = body;
}

// free memory
void ast_free(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_LITERAL_INT:
        case AST_LITERAL_FLOAT:
        case AST_LITERAL_BOOL:
        case AST_LITERAL_NIL:
            break;

        case AST_LITERAL_STRING:
            free(node->as.string_val.value);
            break;

        case AST_EXPR_VARIABLE:
            free(node->as.variable.name);
            break;

        case AST_EXPR_UNARY:
            ast_free(node->as.unary.operand);
            break;

        case AST_EXPR_BINARY:
            ast_free(node->as.binary.left);
            ast_free(node->as.binary.right);
            break;

        case AST_EXPR_ASSIGNMENT:
            free(node->as.assignment.var_name);
            ast_free(node->as.assignment.value);
            break;

        case AST_STMT_VAR_DECL:
            free(node->as.var_decl.var_name);
            ast_free(node->as.var_decl.initializer);
            break;

        case AST_STMT_BLOCK:
            for (int i = 0; i < node->as.block.count; i++) {
                ast_free(node->as.block.statements[i]);
            }
            free(node->as.block.statements);
            break;

        case AST_STMT_IF:
            ast_free(node->as.if_stmt.condition);
            ast_free(node->as.if_stmt.then_branch);
            ast_free(node->as.if_stmt.else_branch);
            break;

        case AST_STMT_WHILE:
            ast_free(node->as.while_stmt.condition);
            ast_free(node->as.while_stmt.body);
            break;

        case AST_STMT_FOR:
            free(node->as.for_stmt.var_name);
            ast_free(node->as.for_stmt.iterable);
            ast_free(node->as.for_stmt.body);
            break;

        case AST_STMT_RETURN:
            ast_free(node->as.return_stmt.value);
            break;

        case AST_STMT_EXPR:
            ast_free(node->as.expr_stmt.expression);
            break;

        case AST_CALL:
            ast_free(node->as.call.callee);
            for (int i = 0; i < node->as.call.arg_count; i++) {
                ast_free(node->as.call.args[i]);
            }
            free(node->as.call.args);
            break;

        case AST_FUNC_DECL:
            free(node->as.func_decl.name);
            for (int i = 0; i < node->as.func_decl.param_count; i++) {
                free(node->as.func_decl.params[i].name);
            }
            free(node->as.func_decl.params);
            ast_free(node->as.func_decl.body);
            break;
    }

    free(node);
}
