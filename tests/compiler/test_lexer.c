#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../../compiler/include/lexer.h"

typedef struct {
    const char* name;
    const char* source;
    const TokenType expected_tokens[30];
    int expected_count;
} TestCase;

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF:            return "EOF";
        case TOKEN_ERROR:          return "ERROR";
        case TOKEN_IDENTIFIER:     return "IDENTIFIER";
        case TOKEN_INT_LITERAL:    return "INT_LITERAL";
        case TOKEN_FLOAT_LITERAL:  return "FLOAT_LITERAL";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";
        case TOKEN_TRUE:           return "TRUE";
        case TOKEN_FALSE:          return "FALSE";
        case TOKEN_NIL:            return "NIL";
        case TOKEN_INT:            return "INT";
        case TOKEN_FLOAT:          return "FLOAT";
        case TOKEN_BOOL:           return "BOOL";
        case TOKEN_STRING:         return "STRING";
        case TOKEN_FUNC:           return "FUNC";
        case TOKEN_RETURN:         return "RETURN";
        case TOKEN_IF:             return "IF";
        case TOKEN_ELSE:           return "ELSE";
        case TOKEN_WHILE:          return "WHILE";
        case TOKEN_FOR:            return "FOR";
        case TOKEN_IN:             return "IN";
        case TOKEN_PLUS:           return "PLUS";
        case TOKEN_MINUS:          return "MINUS";
        case TOKEN_STAR:           return "STAR";
        case TOKEN_SLASH:          return "SLASH";
        case TOKEN_DOUBLE_SLASH:   return "SLASH_SLASH";
        case TOKEN_PERCENT:        return "PERCENT";
        case TOKEN_ASSIGN:         return "ASSIGN";
        case TOKEN_EQUAL:          return "EQUAL";
        case TOKEN_NOT_EQUAL:      return "NOT_EQUAL";
        case TOKEN_LESS:           return "LESS";
        case TOKEN_LESS_EQUAL:     return "LESS_EQUAL";
        case TOKEN_GREATER:        return "GREATER";
        case TOKEN_GREATER_EQUAL:  return "GREATER_EQUAL";
        case TOKEN_AND:            return "AND";
        case TOKEN_OR:             return "OR";
        case TOKEN_NOT:            return "NOT";
        case TOKEN_AMPERSAND:      return "AMPERSAND";
        case TOKEN_PIPE:           return "PIPE";
        case TOKEN_CARET:          return "CARET";
        case TOKEN_TILDE:          return "TILDE";
        case TOKEN_LSHIFT:         return "LSHIFT";
        case TOKEN_RSHIFT:         return "RSHIFT";
        case TOKEN_LPAREN:         return "LPAREN";
        case TOKEN_RPAREN:         return "RPAREN";
        case TOKEN_LBRACE:         return "LBRACE";
        case TOKEN_RBRACE:         return "RBRACE";
        case TOKEN_LBRACKET:       return "LBRACKET";
        case TOKEN_RBRACKET:       return "RBRACKET";
        case TOKEN_SEMICOLON:      return "SEMICOLON";
        case TOKEN_COMMA:          return "COMMA";
        case TOKEN_DOT:            return "DOT";
        case TOKEN_ARROW:          return "ARROW";
        default:                   return "UNKNOWN";
    }
}

static bool run_test_case(const TestCase* test) {
    Lexer lexer;
    lexer_init(&lexer, test->source);

    for (int i = 0; i < test->expected_count; i++) {
        Token token = lexer_next_token(&lexer);
        TokenType expected = test->expected_tokens[i];

        if (token.type != expected) {
            printf(" [FAIL] %s\n", test->name);
            printf(
                "        -> Token #%d mismatch. Expected: %s, Got: %s ('%.*s') at line %d\n",
                i + 1,
                token_type_to_string(expected),
                token_type_to_string(token.type),
                token.length, token.start,
                token.line
            );
            return false;
        }
    }

    printf(" [PASS] %s\n", test->name);
    return true;
}

int main() {
    TestCase tests[] = {
        {
            .name = "1. Compact assignments and string literals",
            .source = "x = 10;\nfloat y=3.1415;\nstring msg = \"Hola \\\"Mundo\\\"\\n\";",
            .expected_tokens = {
                TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_INT_LITERAL, TOKEN_SEMICOLON,
                TOKEN_FLOAT, TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON,
                TOKEN_STRING, TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_STRING_LITERAL, TOKEN_SEMICOLON,
                TOKEN_EOF
            },
            .expected_count = 15
        },
        {
            .name = "2. Functions, single multiplication, and integer division (//)",
            .source = "func add(int a, int b) -> int {\n    return a*2 + b//3;\n}",
            .expected_tokens = {
                TOKEN_FUNC, TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_INT, TOKEN_IDENTIFIER,
                TOKEN_COMMA, TOKEN_INT, TOKEN_IDENTIFIER, TOKEN_RPAREN, TOKEN_ARROW,
                TOKEN_INT, TOKEN_LBRACE, TOKEN_RETURN, TOKEN_IDENTIFIER, TOKEN_STAR,
                TOKEN_INT_LITERAL, TOKEN_PLUS, TOKEN_IDENTIFIER, TOKEN_DOUBLE_SLASH,
                TOKEN_INT_LITERAL, TOKEN_SEMICOLON, TOKEN_RBRACE, TOKEN_EOF
            },
            .expected_count = 23
        },
        {
            .name = "3. Dense expression without spaces",
            .source = "if(a<=b&&x!=10.5){y>>2;}",
            .expected_tokens = {
                TOKEN_IF, TOKEN_LPAREN, TOKEN_IDENTIFIER, TOKEN_LESS_EQUAL, TOKEN_IDENTIFIER,
                TOKEN_AND, TOKEN_IDENTIFIER, TOKEN_NOT_EQUAL, TOKEN_FLOAT_LITERAL, TOKEN_RPAREN,
                TOKEN_LBRACE, TOKEN_IDENTIFIER, TOKEN_RSHIFT, TOKEN_INT_LITERAL, TOKEN_SEMICOLON,
                TOKEN_RBRACE, TOKEN_EOF
            },
            .expected_count = 16
        },
        {
            .name = "4. Loops and bitwise operations",
            .source = "for i in range(0, 10) {\n    val = (i & 1) | (i ^ 2) ~ 3;\n}",
            .expected_tokens = {
                TOKEN_FOR, TOKEN_IDENTIFIER, TOKEN_IN, TOKEN_IDENTIFIER, TOKEN_LPAREN,
                TOKEN_INT_LITERAL, TOKEN_COMMA, TOKEN_INT_LITERAL, TOKEN_RPAREN, TOKEN_LBRACE,
                TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_LPAREN, TOKEN_IDENTIFIER, TOKEN_AMPERSAND,
                TOKEN_INT_LITERAL, TOKEN_RPAREN, TOKEN_PIPE, TOKEN_LPAREN, TOKEN_IDENTIFIER,
                TOKEN_CARET, TOKEN_INT_LITERAL, TOKEN_RPAREN, TOKEN_TILDE, TOKEN_INT_LITERAL,
                TOKEN_SEMICOLON, TOKEN_RBRACE, TOKEN_EOF
            },
            .expected_count = 28
        },
        {
            .name = "5. Error detection (unterminated string)",
            .source = "string err = \"Esta cadena no cierra;",
            .expected_tokens = {
                TOKEN_STRING, TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_ERROR
            },
            .expected_count = 4
        }
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;

    printf("==================================================\n");
    printf("RUNNING LEXER AUTOMATED TEST SUITE\n");
    printf("==================================================\n");

    for (int i = 0; i < num_tests; i++) {
        if (run_test_case(&tests[i])) {
            passed++;
        }
    }

    printf("--------------------------------------------------\n");
    printf("Results: %d/%d tests passed.\n", passed, num_tests);
    printf("==================================================\n");

    return (passed == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}