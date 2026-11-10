#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_NIL,
    TYPE_ERROR
} Type;

typedef struct {
    char* name;
    Type type;
    Type* parameter_types;
    int parameter_count;
    bool is_function;
} Symbol;

typedef struct {
    Symbol* symbols;
    int count;
    int capacity;
} Scope;

typedef struct {
    Scope* scopes;
    int depth;
    int scope_capacity;
    int errors;
    Type current_return_type;
    bool inside_function;
} SemanticAnalyzer;

void semantic_init(SemanticAnalyzer* analyzer);
int semantic_analyze(SemanticAnalyzer* analyzer, ASTNode* program);
void semantic_destroy(SemanticAnalyzer* analyzer);
const char* semantic_type_name(Type type);

#endif