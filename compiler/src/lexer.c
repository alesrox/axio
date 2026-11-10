#include "lexer.h"
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// Auxiliar Functions
static bool is_at_end(Lexer* lexer) {
    return *lexer->current == '\0';
}

static char advance(Lexer* lexer) {
    lexer->current++;
    return lexer->current[-1];
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer) || *lexer->current != expected) return false;
    lexer->current++;
    return true;
}

static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;

    return token;
}

static void trim(Lexer* lexer) {
    while (true) {
        char c = peek(lexer);

        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                advance(lexer);
                break;
            default:
                return;
        }
    }
}

static TokenType check_keyword(Lexer* lexer, int start, int length, const char* rest, TokenType type) {
    int lexeme_len = (int)(lexer->current - lexer->start);
    if (lexeme_len == start + length && strncmp(lexer->start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Lexer* lexer) {
    switch (lexer->start[0]) {
        case 'a': return check_keyword(lexer, 1, 2, "nd", TOKEN_AND);
        case 'b': return check_keyword(lexer, 1, 3, "ool", TOKEN_BOOL);
        case 'e': return check_keyword(lexer, 1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'l': return check_keyword(lexer, 2, 3, "oat", TOKEN_FLOAT);
                    case 'o': return check_keyword(lexer, 2, 1, "r", TOKEN_FOR);
                    case 'u': return check_keyword(lexer, 2, 2, "nc", TOKEN_FUNC);
                }
            }
            break;
        case 'i':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'f': return check_keyword(lexer, 2, 0, "", TOKEN_IF);
                    case 'n': 
                        if (lexer->current - lexer->start == 2) return TOKEN_IN;
                        return check_keyword(lexer, 2, 1, "t", TOKEN_INT);
                }
            }
            break;
        case 'n':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'i': return check_keyword(lexer, 2, 1, "l", TOKEN_NULL);
                    case 'o': return check_keyword(lexer, 2, 1, "t", TOKEN_NOT);
                }
            }
            break;
        case 'o': return check_keyword(lexer, 1, 1, "r", TOKEN_OR);
        case 'r': return check_keyword(lexer, 1, 5, "eturn", TOKEN_RETURN);
        case 's': return check_keyword(lexer, 1, 5, "tring", TOKEN_STRING);
        case 'w': return check_keyword(lexer, 1, 4, "hile", TOKEN_WHILE);
        case 'F': return check_keyword(lexer, 1, 4, "alse", TOKEN_FALSE);
        case 'T': return check_keyword(lexer, 1, 3, "rue", TOKEN_TRUE);
    }

    return TOKEN_IDENTIFIER;
}

static Token scan_identifier(Lexer* lexer) {
    while (isalnum(peek(lexer)) || peek(lexer) == '_') advance(lexer);
    return make_token(lexer, identifier_type(lexer));
}

static Token scan_number(Lexer* lexer) {
    while (isdigit(peek(lexer))) {
        advance(lexer);
    }

    if (peek(lexer) == '.' && isdigit(lexer->current[1])) {
        advance(lexer);

        while (isdigit(peek(lexer))) {
            advance(lexer);
        }

        return make_token(lexer, TOKEN_FLOAT_LITERAL);
    }

    return make_token(lexer, TOKEN_INT_LITERAL);
}

static Token scan_string(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        if (peek(lexer) == '\\') {
            advance(lexer);
            
            if (is_at_end(lexer)) return make_token(lexer, TOKEN_ERROR); // Unclosed string
        }

        advance(lexer);
    }
    
    if (is_at_end(lexer)) return make_token(lexer, TOKEN_ERROR); // Unclosed string

    advance(lexer);
    return make_token(lexer, TOKEN_STRING_LITERAL);
}

// Public Functions
void lexer_init(Lexer* lexer, const char* source) {
    lexer->source   = source;
    lexer->start    = source;
    lexer->current  = source;
    lexer->line     = 1;
}

Token lexer_next_token(Lexer* lexer) {
    trim(lexer);
    lexer->start = lexer->current;

    if (is_at_end(lexer)) return make_token(lexer, TOKEN_EOF);
    char c = advance(lexer);

    if (isalpha(c) || c == '_') return scan_identifier(lexer);
    if (isdigit(c)) return scan_number(lexer);

    switch (c) {
        case '"': return scan_string(lexer);

        case '+': return make_token(lexer, TOKEN_PLUS);
        case '-': return make_token(lexer, match(lexer, '>') ? TOKEN_ARROW : TOKEN_MINUS);
        case '*': return make_token(lexer, TOKEN_STAR);
        case '/': return make_token(lexer, match(lexer, '/') ? TOKEN_DOUBLE_SLASH : TOKEN_SLASH);
        case '%': return make_token(lexer, TOKEN_PERCENT);
        
        case '=': return make_token(lexer, match(lexer, '=') ? TOKEN_EQUAL : TOKEN_ASSIGN);
        case '!': return make_token(lexer, match(lexer, '=') ? TOKEN_NOT_EQUAL : TOKEN_NOT);

        case '<':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_LESS_EQUAL);
            if (match(lexer, '<')) return make_token(lexer, TOKEN_LSHIFT);
            return make_token(lexer, TOKEN_LESS);
        case '>':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_GREATER_EQUAL);
            if (match(lexer, '>')) return make_token(lexer, TOKEN_RSHIFT);
            return make_token(lexer, TOKEN_GREATER);

        case '&': return make_token(lexer, match(lexer, '&') ? TOKEN_AND : TOKEN_AMPERSAND);
        case '|': return make_token(lexer, match(lexer, '|') ? TOKEN_OR : TOKEN_PIPE);
        case '^': return make_token(lexer, TOKEN_CARET);
        case '~': return make_token(lexer, TOKEN_TILDE);

        case '(': return make_token(lexer, TOKEN_LPAREN);
        case ')': return make_token(lexer, TOKEN_RPAREN);
        case '{': return make_token(lexer, TOKEN_LBRACE);
        case '}': return make_token(lexer, TOKEN_RBRACE);
        case '[': return make_token(lexer, TOKEN_LBRACKET);
        case ']': return make_token(lexer, TOKEN_RBRACKET);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case '.': return make_token(lexer, TOKEN_DOT);
    }

    return make_token(lexer, TOKEN_ERROR);
}