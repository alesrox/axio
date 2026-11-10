#include "semantic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Type type_from_token(TokenType token) {
    switch (token) {
        case TOKEN_INT: return TYPE_INT;
        case TOKEN_FLOAT: return TYPE_FLOAT;
        case TOKEN_BOOL: return TYPE_BOOL;
        case TOKEN_STRING: return TYPE_STRING;
        default: return TYPE_ERROR;
    }
}

static char* copy_string(const char* text) {
    size_t length = strlen(text) + 1;
    char* copy = malloc(length);
    if (copy) memcpy(copy, text, length);
    return copy;
}

const char* semantic_type_name(Type type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_STRING: return "string";
        case TYPE_VOID: return "void";
        case TYPE_NULL: return "null";
        default: return "<error>";
    }
}

static void report_error(SemanticAnalyzer* a, int line, const char* message) {
    fprintf(stderr, "[Semantic Error] Line %d: %s\n", line, message);
    a->errors++;
}

static void enter_scope(SemanticAnalyzer* a) {
    if (a->depth + 1 >= a->scope_capacity) {
        int next = a->scope_capacity == 0 ? 8 : a->scope_capacity * 2;
        Scope* scopes = realloc(a->scopes, (size_t)next * sizeof(*scopes));
        if (!scopes) { fprintf(stderr, "Out of memory in semantic analyzer\n"); exit(EXIT_FAILURE); }
        a->scopes = scopes;
        a->scope_capacity = next;
    }

    a->depth++;
    memset(&a->scopes[a->depth], 0, sizeof(a->scopes[a->depth]));
}

static void clear_scope(Scope* scope) {
    for (int i = 0; i < scope->count; i++) {
        free(scope->symbols[i].name);
        free(scope->symbols[i].parameter_types);
    }
    free(scope->symbols);
    memset(scope, 0, sizeof(*scope));
}

static void leave_scope(SemanticAnalyzer* a) {
    if (a->depth >= 0) clear_scope(&a->scopes[a->depth--]);
}

static Symbol* find_in_scope(Scope* scope, const char* name) {
    for (int i = scope->count - 1; i >= 0; i--)
        if (strcmp(scope->symbols[i].name, name) == 0) return &scope->symbols[i];
    return NULL;
}

static Symbol* find_symbol(SemanticAnalyzer* a, const char* name) {
    for (int d = a->depth; d >= 0; d--) {
        Symbol* symbol = find_in_scope(&a->scopes[d], name);
        if (symbol) return symbol;
    }
    return NULL;
}

static Symbol* declare_symbol(
    SemanticAnalyzer* a, const char* name, Type type, bool is_function, int line
) {
    Scope* scope = &a->scopes[a->depth];
    if (find_in_scope(scope, name)) {
        char message[256];
        snprintf(message, sizeof(message), "'%s' is already declared in this scope", name);
        report_error(a, line, message);
        return NULL;
    }

    if (scope->count == scope->capacity) {
        int next = scope->capacity == 0 ? 8 : scope->capacity * 2;
        Symbol* symbols = realloc(scope->symbols, (size_t)next * sizeof(*symbols));
        if (!symbols) { fprintf(stderr, "Out of memory in semantic analyzer\n"); exit(EXIT_FAILURE); }
        scope->symbols = symbols;
        scope->capacity = next;
    }

    Symbol* symbol = &scope->symbols[scope->count++];
    memset(symbol, 0, sizeof(*symbol));
    symbol->name = copy_string(name);
    if (!symbol->name) { fprintf(stderr, "Out of memory in semantic analyzer\n"); exit(EXIT_FAILURE); }
    symbol->type = type;
    symbol->is_function = is_function;
    return symbol;
}

static bool is_numeric(Type type) { return type == TYPE_INT || type == TYPE_FLOAT; }
static bool same_or_numeric_pair(Type a, Type b) {
    return a == b || (is_numeric(a) && is_numeric(b));
}

static Type analyze_expr(SemanticAnalyzer* a, ASTNode* node);
static void analyze_stmt(SemanticAnalyzer* a, ASTNode* node);

static Type analyze_call(SemanticAnalyzer* a, ASTNode* node) {
    ASTNode* callee = node->as.call.callee;
    if (!callee || callee->type != AST_EXPR_VARIABLE) {
        report_error(a, node->line, "only named functions can be called");
        for (int i = 0; i < node->as.call.arg_count; i++) analyze_expr(a, node->as.call.args[i]);
        return TYPE_ERROR;
    }
    const char* name = callee->as.variable.name;
    Symbol* fn = find_symbol(a, name);
    if (!fn) {
        char message[256]; snprintf(message, sizeof(message), "function '%s' is not declared", name);
        report_error(a, node->line, message);
        for (int i = 0; i < node->as.call.arg_count; i++) analyze_expr(a, node->as.call.args[i]);
        return TYPE_ERROR;
    }
    if (!fn->is_function) {
        char message[256]; snprintf(message, sizeof(message), "'%s' is not a function", name);
        report_error(a, node->line, message);
        return TYPE_ERROR;
    }
    if (node->as.call.arg_count != fn->parameter_count) {
        char message[256]; snprintf(
            message, sizeof(message), "function '%s' expects %d argument(s), got %d", 
            name, fn->parameter_count, node->as.call.arg_count
        );
        report_error(a, node->line, message);
    }
    for (int i = 0; i < node->as.call.arg_count; i++) {
        Type actual = analyze_expr(a, node->as.call.args[i]);
        if (i < fn->parameter_count && actual != TYPE_ERROR && actual != fn->parameter_types[i]) {
            char message[256]; snprintf(message, sizeof(message),
                "argument %d of '%s' has type %s; expected %s", i + 1, name,
                semantic_type_name(actual), semantic_type_name(fn->parameter_types[i]));
            report_error(a, node->as.call.args[i]->line, message);
        }
    }
    return fn->type;
}

static Type analyze_expr(SemanticAnalyzer* a, ASTNode* node) {
    if (!node) return TYPE_ERROR;
    switch (node->type) {
        case AST_LITERAL_INT: return TYPE_INT;
        case AST_LITERAL_FLOAT: return TYPE_FLOAT;
        case AST_LITERAL_STRING: return TYPE_STRING;
        case AST_LITERAL_BOOL: return TYPE_BOOL;
        case AST_LITERAL_NULL: return TYPE_NULL;
        case AST_EXPR_VARIABLE: {
            Symbol* symbol = find_symbol(a, node->as.variable.name);
            if (!symbol) {
                char message[256]; snprintf(message, sizeof(message), "variable '%s' is not declared", node->as.variable.name);
                report_error(a, node->line, message); return TYPE_ERROR;
            }
            if (symbol->is_function) {
                char message[256]; snprintf(message, sizeof(message), "function '%s' cannot be used as a variable", node->as.variable.name);
                report_error(a, node->line, message); return TYPE_ERROR;
            }
            return symbol->type;
        }
        case AST_EXPR_ASSIGNMENT: {
            Symbol* symbol = find_symbol(a, node->as.assignment.var_name);
            Type value = analyze_expr(a, node->as.assignment.value);
            if (!symbol) {
                char message[256]; snprintf(message, sizeof(message), "variable '%s' is not declared", node->as.assignment.var_name);
                report_error(a, node->line, message); return TYPE_ERROR;
            }
            if (symbol->is_function) { report_error(a, node->line, "cannot assign to a function"); return TYPE_ERROR; }
            if (value != TYPE_ERROR && value != symbol->type) {
                char message[256]; snprintf(message, sizeof(message), "cannot assign %s to %s variable '%s'",
                    semantic_type_name(value), semantic_type_name(symbol->type), node->as.assignment.var_name);
                report_error(a, node->line, message); return TYPE_ERROR;
            }
            return symbol->type;
        }
        case AST_CALL: return analyze_call(a, node);
        case AST_EXPR_UNARY: {
            Type operand = analyze_expr(a, node->as.unary.operand);
            if (operand == TYPE_ERROR) return TYPE_ERROR;
            TokenType op = node->as.unary.op;
            bool valid = (op == TOKEN_MINUS && is_numeric(operand)) ||
                        (op == TOKEN_NOT && operand == TYPE_BOOL) ||
                        (op == TOKEN_TILDE && operand == TYPE_INT);
            if (!valid) { report_error(a, node->line, "invalid operand type for unary operator"); return TYPE_ERROR; }
            return operand == TYPE_FLOAT && op == TOKEN_MINUS ? TYPE_FLOAT :
                (op == TOKEN_NOT ? TYPE_BOOL : TYPE_INT);
        }
        case AST_EXPR_BINARY: {
            Type left = analyze_expr(a, node->as.binary.left);
            Type right = analyze_expr(a, node->as.binary.right);
            if (left == TYPE_ERROR || right == TYPE_ERROR) return TYPE_ERROR;
            TokenType op = node->as.binary.op;
            if (op == TOKEN_PLUS && left == TYPE_STRING && right == TYPE_STRING) return TYPE_STRING;
            if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_STAR || op == TOKEN_SLASH ||
                op == TOKEN_DOUBLE_SLASH || op == TOKEN_PERCENT) {
                if (is_numeric(left) && is_numeric(right) && same_or_numeric_pair(left, right))
                    return left == TYPE_FLOAT || right == TYPE_FLOAT ? TYPE_FLOAT : TYPE_INT;
            } else if (op == TOKEN_AND || op == TOKEN_OR) {
                if (left == TYPE_BOOL && right == TYPE_BOOL) return TYPE_BOOL;
            } else if (op == TOKEN_AMPERSAND || op == TOKEN_PIPE || op == TOKEN_CARET ||
                    op == TOKEN_LSHIFT || op == TOKEN_RSHIFT) {
                if (left == TYPE_INT && right == TYPE_INT) return TYPE_INT;
            } else if (op == TOKEN_EQUAL || op == TOKEN_NOT_EQUAL) {
                if (same_or_numeric_pair(left, right)) return TYPE_BOOL;
            } else if (op == TOKEN_LESS || op == TOKEN_LESS_EQUAL || op == TOKEN_GREATER || op == TOKEN_GREATER_EQUAL) {
                if (is_numeric(left) && is_numeric(right)) return TYPE_BOOL;
            }
            report_error(a, node->line, "operator is not defined for these operand types");
            return TYPE_ERROR;
        }
        default:
            report_error(a, node->line, "invalid AST node in expression");
            return TYPE_ERROR;
    }
}

static void analyze_block_contents(SemanticAnalyzer* a, ASTNode* block, bool create_scope) {
    if (create_scope) enter_scope(a);
    for (int i = 0; i < block->as.block.count; i++) analyze_stmt(a, block->as.block.statements[i]);
    if (create_scope) leave_scope(a);
}

static void analyze_function(SemanticAnalyzer* a, ASTNode* node) {
    Symbol* fn = find_in_scope(&a->scopes[0], node->as.func_decl.name);
    if (!fn || !fn->is_function) return; /* duplicate declaration was reported during predeclaration */
    enter_scope(a);
    for (int i = 0; i < node->as.func_decl.param_count; i++) {
        Param* param = &node->as.func_decl.params[i];
        declare_symbol(a, param->name, type_from_token(param->type), false, node->line);
    }
    bool previous_inside = a->inside_function;
    Type previous_return = a->current_return_type;
    a->inside_function = true;
    a->current_return_type = fn->type;
    if (node->as.func_decl.body && node->as.func_decl.body->type == AST_STMT_BLOCK)
        analyze_block_contents(a, node->as.func_decl.body, true);
    a->inside_function = previous_inside;
    a->current_return_type = previous_return;
    leave_scope(a);
}

static void analyze_stmt(SemanticAnalyzer* a, ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case AST_STMT_BLOCK: analyze_block_contents(a, node, true); break;
        case AST_STMT_VAR_DECL: {
            Type type = type_from_token(node->as.var_decl.var_type);
            Type init = node->as.var_decl.initializer ? analyze_expr(a, node->as.var_decl.initializer) : type;
            if (type == TYPE_ERROR) { report_error(a, node->line, "unknown variable type"); break; }
            if (init != TYPE_ERROR && init != type) {
                char message[256]; snprintf(message, sizeof(message), "initializer for '%s' has type %s; expected %s",
                    node->as.var_decl.var_name, semantic_type_name(init), semantic_type_name(type));
                report_error(a, node->line, message);
            }
            declare_symbol(a, node->as.var_decl.var_name, type, false, node->line);
            break;
        }
        case AST_STMT_EXPR: (void)analyze_expr(a, node->as.expr_stmt.expression); break;
        case AST_STMT_IF: {
            Type condition = analyze_expr(a, node->as.if_stmt.condition);
            if (condition != TYPE_ERROR && condition != TYPE_BOOL) report_error(a, node->line, "if condition must have type bool");
            analyze_stmt(a, node->as.if_stmt.then_branch);
            analyze_stmt(a, node->as.if_stmt.else_branch);
            break;
        }
        case AST_STMT_WHILE: {
            Type condition = analyze_expr(a, node->as.while_stmt.condition);
            if (condition != TYPE_ERROR && condition != TYPE_BOOL) report_error(a, node->line, "while condition must have type bool");
            analyze_stmt(a, node->as.while_stmt.body); break;
        }
        case AST_STMT_FOR: {
            ASTNode* iterable = node->as.for_stmt.iterable;
            Type item_type = TYPE_ERROR;
            if (iterable && iterable->type == AST_CALL && iterable->as.call.callee &&
                iterable->as.call.callee->type == AST_EXPR_VARIABLE &&
                strcmp(iterable->as.call.callee->as.variable.name, "range") == 0) {
                if (iterable->as.call.arg_count < 1 || iterable->as.call.arg_count > 2)
                    report_error(a, iterable->line, "range expects one or two integer arguments");
                for (int i = 0; i < iterable->as.call.arg_count; i++) {
                    Type t = analyze_expr(a, iterable->as.call.args[i]);
                    if (t != TYPE_ERROR && t != TYPE_INT) report_error(a, iterable->as.call.args[i]->line, "range arguments must be int");
                }
                item_type = TYPE_INT;
            } else {
                (void)analyze_expr(a, iterable);
                report_error(a, node->line, "for currently supports range(...) iterables only");
            }
            enter_scope(a);
            declare_symbol(a, node->as.for_stmt.var_name, item_type, false, node->line);
            analyze_stmt(a, node->as.for_stmt.body);
            leave_scope(a);
            break;
        }
        case AST_STMT_RETURN: {
            Type value = node->as.return_stmt.value ? analyze_expr(a, node->as.return_stmt.value) : TYPE_VOID;
            if (!a->inside_function) report_error(a, node->line, "return is only valid inside a function");
            else if (value != TYPE_ERROR && value != a->current_return_type) {
                char message[256]; snprintf(message, sizeof(message), "return has type %s; expected %s",
                    semantic_type_name(value), semantic_type_name(a->current_return_type));
                report_error(a, node->line, message);
            }
            break;
        }
        case AST_FUNC_DECL:
            if (a->depth != 0) report_error(a, node->line, "function declarations are only allowed at program scope");
            else analyze_function(a, node);
            break;
        default: report_error(a, node->line, "invalid AST node in statement"); break;
    }
}

void semantic_init(SemanticAnalyzer* analyzer) {
    memset(analyzer, 0, sizeof(*analyzer));
    analyzer->depth = -1;
}

int semantic_analyze(SemanticAnalyzer* analyzer, ASTNode* program) {
    if (!analyzer || !program || program->type != AST_STMT_BLOCK) return 1;
    analyzer->errors = 0;
    analyzer->inside_function = false;
    analyzer->current_return_type = TYPE_VOID;
    enter_scope(analyzer);

    // Register all function signatures first so forward calls and recursion work. (ONLY FUNCTIONS)
    for (int i = 0; i < program->as.block.count; i++) {
        ASTNode* node = program->as.block.statements[i];
        if (!node || node->type != AST_FUNC_DECL) continue;
        
        Type result = node->as.func_decl.return_type == TOKEN_EOF ? TYPE_VOID : type_from_token(node->as.func_decl.return_type);
        Symbol* fn = declare_symbol(analyzer, node->as.func_decl.name, result, true, node->line);
        if (!fn) continue;

        fn->parameter_count = node->as.func_decl.param_count;
        if (fn->parameter_count > 0) {
            fn->parameter_types = malloc((size_t)fn->parameter_count * sizeof(*fn->parameter_types));
            if (!fn->parameter_types) { fprintf(stderr, "Out of memory in semantic analyzer\n"); exit(EXIT_FAILURE); }
            for (int p = 0; p < fn->parameter_count; p++)
                fn->parameter_types[p] = type_from_token(node->as.func_decl.params[p].type);
        }
    }

    for (int i = 0; i < program->as.block.count; i++) analyze_stmt(analyzer, program->as.block.statements[i]);
    int errors = analyzer->errors;
    leave_scope(analyzer);
    return errors;
}

void semantic_destroy(SemanticAnalyzer* analyzer) {
    if (!analyzer) return;
    while (analyzer->depth >= 0) leave_scope(analyzer);
    free(analyzer->scopes);
    memset(analyzer, 0, sizeof(*analyzer));
    analyzer->depth = -1;
}
