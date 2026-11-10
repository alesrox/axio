#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "parser.h"

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // ||
    PREC_AND,         // &&
    PREC_BITWISE_OR,  // |
    PREC_BITWISE_XOR, // ^
    PREC_BITWISE_AND, // &
    PREC_EQUALITY,    // ==, !=
    PREC_COMPARISON,  // <, >, <=, >=
    PREC_SHIFT,       // <<, >>
    PREC_TERM,        // +, -
    PREC_FACTOR,      // *, /, //, %
    PREC_UNARY,       // !, -, ~
    PREC_CALL         // ()
} Precedence;

static void error_at(Parser* parser, Token* token, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;

    fprintf(stderr, "[Sintax Error] Line %d", token->line);
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at the end of the file");
    } else if (token->type == TOKEN_ERROR) {
        // Error reportado por el lexer
    } else {
        fprintf(stderr, " in '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", message);
}

static void advance(Parser* parser) {
    parser->previous = parser->current;

    while (true) {
        parser->current = lexer_next_token(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;
        error_at(parser, &parser->current, parser->current.start);
    }
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    error_at(parser, &parser->current, message);
}

static bool match(Parser* parser, TokenType type) {
    if (parser->current.type != type) return false;
    advance(parser);
    return true;
}

// Expressions (Pratt Parsing)
typedef ASTNode* (*ParsePrefixFn)(Parser* parser);
typedef ASTNode* (*ParseInfixFn)(Parser* parser, ASTNode* left);

typedef struct {
    ParsePrefixFn prefix;
    ParseInfixFn  infix;
    Precedence    precedence;
} ParseRule;

static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_expression(Parser* parser, Precedence precedence);
static const ParseRule* get_rule(TokenType type);

static ASTNode* parse_null(Parser* parser) {
    return ast_create_literal_null(parser->previous.line);
}

static ASTNode* parse_boolean(Parser* parser) {
    bool val = (parser->previous.type == TOKEN_TRUE);
    return ast_create_literal_bool(val, parser->previous.line);
}

static ASTNode* parse_number(Parser* parser) {
    Token token = parser->previous;
    if (token.type == TOKEN_INT_LITERAL) {
        int64_t val = strtoll(token.start, NULL, 10);
        return ast_create_literal_int(val, token.line);
    } else {
        double val = strtod(token.start, NULL);
        return ast_create_literal_float(val, token.line);
    }
}

static ASTNode* parse_string(Parser* parser) {
    Token token = parser->previous;
    return ast_create_literal_string(token.start + 1, token.length - 2, token.line); // No quotes
}

static ASTNode* parse_variable(Parser* parser) {
    Token token = parser->previous;
    return ast_create_variable(token.start, token.length, token.line);
}

static ASTNode* parse_grouping(Parser* parser) {
    ASTNode* expr = parse_expression(parser, PREC_NONE);
    consume(parser, TOKEN_RPAREN, "')' was expected after expression.");
    return expr;
}

static ASTNode* parse_unary(Parser* parser) {
    TokenType op = parser->previous.type;
    int line = parser->previous.line;
    ASTNode* operand = parse_expression(parser, PREC_UNARY);
    return ast_create_unary(op, operand, line);
}

static ASTNode* parse_binary(Parser* parser, ASTNode* left) {
    TokenType op = parser->previous.type;
    int line = parser->previous.line;
    const ParseRule* rule = get_rule(op);
    
    ASTNode* right = parse_expression(parser, (Precedence)(rule->precedence + 1));

    return ast_create_binary(op, left, right, line);
}

static ASTNode* parse_call(Parser* parser, ASTNode* left) {
    int line = parser->previous.line;
    ASTNode* call_node = ast_create_call(left, line);

    if (parser->current.type != TOKEN_RPAREN) {
        do {
            ASTNode* arg = parse_expression(parser, PREC_NONE);
            ast_call_add_arg(call_node, arg);
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "')' was expected after arguments.");
    return call_node;
}

static ASTNode* parse_assign(Parser* parser, ASTNode* left) {
    int line = parser->previous.line;
    if (left == NULL) return NULL;

    if (left->type != AST_EXPR_VARIABLE) {
        switch (left->type) {
            case AST_LITERAL_INT:
            case AST_LITERAL_FLOAT:
            case AST_LITERAL_STRING:
            case AST_LITERAL_BOOL:
            case AST_LITERAL_NULL:
                error_at(parser, &parser->previous, 
                    "Cannot assign a value to literal.");
                break;

            case AST_EXPR_BINARY:
            case AST_EXPR_UNARY:
                error_at(parser, &parser->previous, 
                    "Cannot assign to expression. Maybe you meant '==' instead of '='?");
                break;

            case AST_CALL:
                error_at(parser, &parser->previous, 
                    "Cannot assign a value to function call.");
                break;

            default:
                error_at(parser, &parser->previous, 
                    "It must be assigned to a variable.");
                break;
        }
        return left;
    }

    ASTNode* value = parse_expression(parser, PREC_ASSIGNMENT);
    
    ASTNode* assign_node = ast_create_assignment(
        left->as.variable.name,
        left->as.variable.length,
        value,
        line
    );

    free(left); 
    return assign_node;
}

static const ParseRule rules[] = {
    // Delimiters and puntuaction
    [TOKEN_SEMICOLON]      = {NULL,           NULL,         PREC_NONE},
    [TOKEN_COMMA]          = {NULL,           NULL,         PREC_NONE},
    [TOKEN_DOT]            = {NULL,           NULL,         PREC_NONE},
    [TOKEN_ARROW]          = {NULL,           NULL,         PREC_NONE},

    // Grouping and blocks
    [TOKEN_LPAREN]         = {parse_grouping, parse_call,   PREC_CALL},
    [TOKEN_RPAREN]         = {NULL,           NULL,         PREC_NONE},
    [TOKEN_LBRACE]         = {NULL,           NULL,         PREC_NONE},
    [TOKEN_RBRACE]         = {NULL,           NULL,         PREC_NONE},
    [TOKEN_LBRACKET]       = {NULL,           NULL,         PREC_NONE},
    [TOKEN_RBRACKET]       = {NULL,           NULL,         PREC_NONE},

    // Operators
    [TOKEN_ASSIGN]         = {NULL,           parse_assign, PREC_ASSIGNMENT},
    [TOKEN_MINUS]          = {parse_unary,    parse_binary, PREC_TERM},
    [TOKEN_PLUS]           = {NULL,           parse_binary, PREC_TERM},
    [TOKEN_SLASH]          = {NULL,           parse_binary, PREC_FACTOR},
    [TOKEN_DOUBLE_SLASH]   = {NULL,           parse_binary, PREC_FACTOR},
    [TOKEN_STAR]           = {NULL,           parse_binary, PREC_FACTOR},
    [TOKEN_PERCENT]        = {NULL,           parse_binary, PREC_FACTOR},
    [TOKEN_NOT]            = {parse_unary,    NULL,         PREC_NONE},
    [TOKEN_TILDE]          = {parse_unary,    NULL,         PREC_NONE},
    [TOKEN_EQUAL]          = {NULL,           parse_binary, PREC_EQUALITY},
    [TOKEN_NOT_EQUAL]      = {NULL,           parse_binary, PREC_EQUALITY},
    [TOKEN_LESS]           = {NULL,           parse_binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]     = {NULL,           parse_binary, PREC_COMPARISON},
    [TOKEN_GREATER]        = {NULL,           parse_binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL]  = {NULL,           parse_binary, PREC_COMPARISON},
    [TOKEN_AND]            = {NULL,           parse_binary, PREC_AND},
    [TOKEN_OR]             = {NULL,           parse_binary, PREC_OR},
    [TOKEN_AMPERSAND]      = {NULL,           parse_binary, PREC_BITWISE_AND},
    [TOKEN_PIPE]           = {NULL,           parse_binary, PREC_BITWISE_OR},
    [TOKEN_CARET]          = {NULL,           parse_binary, PREC_BITWISE_XOR},
    [TOKEN_LSHIFT]         = {NULL,           parse_binary, PREC_SHIFT},
    [TOKEN_RSHIFT]         = {NULL,           parse_binary, PREC_SHIFT},

    // Literals and Identifiers
    [TOKEN_INT_LITERAL]    = {parse_number,   NULL,         PREC_NONE},
    [TOKEN_FLOAT_LITERAL]  = {parse_number,   NULL,         PREC_NONE},
    [TOKEN_STRING_LITERAL] = {parse_string,   NULL,         PREC_NONE},
    [TOKEN_TRUE]           = {parse_boolean,  NULL,         PREC_NONE},
    [TOKEN_FALSE]          = {parse_boolean,  NULL,         PREC_NONE},
    [TOKEN_NULL]            = {parse_null,      NULL,         PREC_NONE},
    [TOKEN_IDENTIFIER]     = {parse_variable, NULL,         PREC_NONE},
    [TOKEN_EOF]            = {NULL,           NULL,         PREC_NONE},
};

static const ParseRule* get_rule(TokenType type) {
    return &rules[type];
}

static ASTNode* parse_expression(Parser* parser, Precedence precedence) {
    advance(parser);
    ParsePrefixFn prefix_rule = get_rule(parser->previous.type)->prefix;
    if (prefix_rule == NULL) {
        error_at(parser, &parser->previous, "An expression was expected.");
        return NULL;
    }

    ASTNode* left = prefix_rule(parser);

    while (left != NULL) {
        const ParseRule* rule = get_rule(parser->current.type);

        if (rule->infix == NULL || precedence > rule->precedence) break;

        advance(parser);
        left = rule->infix(parser, left);
    }

    return left;
}

// Parser Statements
static ASTNode* parse_block(Parser* parser) {
    consume(parser, TOKEN_LBRACE, "{ was expected.");

    ASTNode* block = ast_create_block(parser->previous.line);

    while (
        parser->current.type != TOKEN_RBRACE &&
        parser->current.type != TOKEN_EOF
    ) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt != NULL) ast_block_add_stmt(block, stmt);
    }

    consume(parser, TOKEN_RBRACE, "'}' was expected.");
    return block;
}

static ASTNode* parse_func_decl(Parser* parser) {
    consume(parser, TOKEN_IDENTIFIER, "An identifier was expected.");
    Token name = parser->previous;

    consume(parser, TOKEN_LPAREN, "'(' was expected behind function name.");

    ASTNode* func_node = ast_create_func_decl(name.start, name.length, name.line);

    if (parser->current.type != TOKEN_RPAREN) {
        do {
            if (
                parser->current.type != TOKEN_INT &&
                parser->current.type != TOKEN_FLOAT &&
                parser->current.type != TOKEN_BOOL &&
                parser->current.type != TOKEN_STRING
            ) {
                error_at(parser, &parser->current, "A parameter type was expected.");
                break;
            }

            TokenType param_type = parser->current.type;
            advance(parser);

            Token param_name = parser->current;
            consume(parser, TOKEN_IDENTIFIER, "A parameter name was expected.");

            ast_func_add_param(func_node, param_type, param_name.start, param_name.length);
        } while(match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "')' was expected after parameters.");

    if (match(parser, TOKEN_ARROW)) {
        if (parser->current.type != TOKEN_INT && parser->current.type != TOKEN_FLOAT &&
            parser->current.type != TOKEN_BOOL && parser->current.type != TOKEN_STRING) {
            error_at(parser, &parser->current, "A return type was expected after '->'.");
        } else {
            func_node->as.func_decl.return_type = parser->current.type;
            advance(parser);
        }
    }

    ASTNode* body = parse_block(parser);
    ast_func_set_body(func_node, body);

    return func_node;
}

static ASTNode* parse_return_stmt(Parser* parser) {
    int line = parser->previous.line;
    ASTNode* value = NULL;

    if (parser->current.type != TOKEN_SEMICOLON)
        value = parse_expression(parser, PREC_NONE);

    consume(parser, TOKEN_SEMICOLON, "';' was expected after a return statement");
    
    return ast_create_return(value, line);
}

static ASTNode* parse_var_decl(Parser* parser) {
    TokenType var_type = parser->previous.type;
    consume(parser, TOKEN_IDENTIFIER, "Name variable was expected.");
    Token name = parser->previous;

    ASTNode* initializer = NULL;
    if (match(parser, TOKEN_ASSIGN)) {
        initializer = parse_expression(parser, PREC_NONE);
    }

    consume(parser, TOKEN_SEMICOLON, "';' was expected at the end of declaration.");
    return ast_create_var_decl(var_type, name.start, name.length, initializer, name.line);
}

static ASTNode* parse_if_stmt(Parser* parser) {
    int line = parser->previous.line;
    consume(parser, TOKEN_LPAREN, "'(' was expected after 'if'.");
    ASTNode* condition = parse_expression(parser, PREC_NONE);
    consume(parser, TOKEN_RPAREN, "')' was expected after condition.");

    // consume(parser, TOKEN_LBRACE, "'{'.");
    ASTNode* then_branch = parse_block(parser);
    ASTNode* else_branch = NULL;

    if (match(parser, TOKEN_ELSE)) {
        if (match(parser, TOKEN_IF)) {
            else_branch = parse_if_stmt(parser);
        } else {
            // consume(parser, TOKEN_LBRACE, "'{'.");
            else_branch = parse_block(parser);
        }
    }

    return ast_create_if(condition, then_branch, else_branch, line);
}

static ASTNode* parse_for_stmt(Parser* parser) {
    int line = parser->previous.line;
    consume(parser, TOKEN_IDENTIFIER, "The 'for' loop's iterator variable was expected.");
    Token var_name = parser->previous;

    consume(parser, TOKEN_IN, "The keyword 'in' was expected.");
    ASTNode* iterable = parse_expression(parser, PREC_NONE);

    // consume(parser, TOKEN_LBRACE, "'{'.");
    ASTNode* body = parse_block(parser);

    return ast_create_for(var_name.start, var_name.length, iterable, body, line);
}

static ASTNode* parse_while_stmt(Parser* parser) {
    int line = parser->previous.line;
    consume(parser, TOKEN_LPAREN, "'(' was expected after 'while'.");
    ASTNode* condition = parse_expression(parser, PREC_NONE);
    consume(parser, TOKEN_RPAREN, "')' was expected after the condition.");

    // consume(parser, TOKEN_LBRACE, "'{'.");
    ASTNode* body = parse_block(parser);

    return ast_create_while(condition, body, line);
}

static ASTNode* parse_statement(Parser* parser) {
    if (match(parser, TOKEN_FUNC)) {
        return parse_func_decl(parser);
    }
    if (match(parser, TOKEN_RETURN)) {
        return parse_return_stmt(parser);
    }
    if (match(parser, TOKEN_INT) || match(parser, TOKEN_FLOAT) || 
        match(parser, TOKEN_BOOL) || match(parser, TOKEN_STRING)) {
        return parse_var_decl(parser);
    }
    if (match(parser, TOKEN_IF)) {
        return parse_if_stmt(parser);
    }
    if (match(parser, TOKEN_FOR)) {
        return parse_for_stmt(parser);
    }
    if (match(parser, TOKEN_WHILE)) {
        return parse_while_stmt(parser);
    }
    if (parser->current.type == TOKEN_LBRACE) {
        return parse_block(parser);
    }

    ASTNode* expr = parse_expression(parser, PREC_NONE);
    consume(parser, TOKEN_SEMICOLON, "';' was expected at the end of a expression.");

    return ast_create_expr_stmt(expr, parser->previous.line);
}

// Parser
void parser_init(Parser* parser, Lexer* lexer) {
    memset(parser, 0, sizeof(Parser));
    parser->lexer = lexer;

    advance(parser);
}

ASTNode* parse_program(Parser* parser) {
    ASTNode* root = ast_create_block(1);

    while(parser->current.type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt != NULL) ast_block_add_stmt(root, stmt);
    }

    return root;
}