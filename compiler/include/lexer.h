#ifndef LEXER_H
#define LEXER_H

typedef enum {
    // File and Control Tokens
    TOKEN_EOF,
    TOKEN_ERROR,

    // Literals and Identifiers
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NIL,

    // Keywords
    TOKEN_INT, TOKEN_FLOAT, TOKEN_BOOL, TOKEN_STRING,
    TOKEN_FUNC, TOKEN_RETURN,
    TOKEN_IF, TOKEN_ELSE,
    TOKEN_WHILE, TOKEN_FOR, TOKEN_IN,

    // Operators and Delimiters
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_DOUBLE_SLASH, TOKEN_PERCENT,
    TOKEN_ASSIGN, TOKEN_EQUAL, TOKEN_NOT_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL, TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_AND, TOKEN_OR, TOKEN_NOT,
    TOKEN_AMPERSAND, TOKEN_PIPE, TOKEN_CARET, TOKEN_TILDE,
    TOKEN_LSHIFT, TOKEN_RSHIFT,

    // Punctuation
    TOKEN_LPAREN, TOKEN_RPAREN,     // ( )
    TOKEN_LBRACE, TOKEN_RBRACE,     // { }
    TOKEN_LBRACKET, TOKEN_RBRACKET, // [ ]
    TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_DOT, TOKEN_ARROW // ; , . ->
} TokenType;

typedef struct {
    TokenType type;
    const char* start; // Pointer to the first character of the lexeme in the source text
    int length;        // Length of the lexeme Longitud del lexema
    int line;          // Current line in the source (to report errors)
} Token;

typedef struct {
    const char* source;
    const char* start;      // Start of the lexeme being processed
    const char* current;    // Current processing character in the source
    int line;               // Current line in the source (to report errors)
} Lexer;

void lexer_init(Lexer* lexer, const char* source);
Token lexer_next_token(Lexer* lexer);

#endif // LEXER_H