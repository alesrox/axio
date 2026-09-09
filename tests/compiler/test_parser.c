#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../compiler/include/lexer.h"
#include "../../compiler/include/parser.h"
#include "../../compiler/include/ast.h"

typedef struct {
    const char* name;
    const char* source;
    bool (*validate)(ASTNode* root);
    bool should_fail;
} TestCase;

static bool run_test_case(const TestCase* test) {
    int stderr_copy = dup(STDERR_FILENO);
    
    int dev_null = open("/dev/null", O_WRONLY);
    if (dev_null != -1) {
        dup2(dev_null, STDERR_FILENO);
        close(dev_null);
    }

    Lexer lexer;
    lexer_init(&lexer, test->source);
    
    Parser parser;
    parser_init(&parser, &lexer);

    ASTNode* root = parse_program(&parser);

    fflush(stderr);
    dup2(stderr_copy, STDERR_FILENO);
    close(stderr_copy);

    if (test->should_fail) {
        if (parser.had_error) {
            printf(" [PASS] %s\n", test->name);
            if (root != NULL) ast_free(root);
            return true;
        } else {
            printf(" [FAIL] %s (Se esperaba un error sintáctico pero el parsing pasó)\n", test->name);
            if (root != NULL) ast_free(root);
            return false;
        }
    }

    if (parser.had_error || !test->validate(root)) {
        printf(" [FAIL] %s\n", test->name);
        if (root != NULL) ast_free(root);
        return false;
    }

    printf(" [PASS] %s\n", test->name);
    if (root != NULL) ast_free(root);
    return true;
}

// -----------------------------------------------------------------------------
// VALIDATIONS
// -----------------------------------------------------------------------------

static bool validate_func_decl(ASTNode* root) {
    if (root == NULL || root->type != AST_STMT_BLOCK || root->as.block.count == 0) return false;

    ASTNode* func_node = root->as.block.statements[0];
    if (func_node == NULL || func_node->type != AST_FUNC_DECL) return false;
    if (strncmp(func_node->as.func_decl.name, "sum", 3) != 0) return false;
    if (func_node->as.func_decl.param_count != 2) return false;

    return true;
}

static bool validate_func_call(ASTNode* root) {
    if (root == NULL || root->type != AST_STMT_BLOCK || root->as.block.count == 0) return false;

    ASTNode* stmt = root->as.block.statements[0];
    if (stmt == NULL || stmt->type != AST_STMT_EXPR) return false;

    ASTNode* assign = stmt->as.expr_stmt.expression;
    if (assign == NULL || assign->type != AST_EXPR_ASSIGNMENT) return false;

    ASTNode* call = assign->as.assignment.value;
    if (call == NULL || call->type != AST_CALL) return false;
    if (call->as.call.callee->type != AST_EXPR_VARIABLE) return false;
    if (strncmp(call->as.call.callee->as.variable.name, "suma", 4) != 0) return false;
    if (call->as.call.arg_count != 2) return false;

    return true;
}

static bool validate_var_decl(ASTNode* root) {
    if (root == NULL || root->type != AST_STMT_BLOCK || root->as.block.count == 0) return false;

    ASTNode* stmt = root->as.block.statements[0];
    if (stmt == NULL || stmt->type != AST_STMT_VAR_DECL) return false;

    if (stmt->as.var_decl.var_type != TOKEN_INT) return false;
    if (strncmp(stmt->as.var_decl.var_name, "my_var", 6) != 0) return false;

    ASTNode* initializer = stmt->as.var_decl.initializer;
    if (initializer == NULL || initializer->type != AST_LITERAL_INT) return false;
    if (initializer->as.int_val != 42) return false;

    return true;
}

static bool validate_if_stmt(ASTNode* root) {
    if (root == NULL || root->type != AST_STMT_BLOCK || root->as.block.count == 0) return false;

    ASTNode* stmt = root->as.block.statements[0];
    if (stmt == NULL || stmt->type != AST_STMT_IF) return false;

    ASTNode* cond = stmt->as.if_stmt.condition;
    if (cond == NULL || cond->type != AST_EXPR_BINARY) return false;

    if (stmt->as.if_stmt.then_branch == NULL || stmt->as.if_stmt.then_branch->type != AST_STMT_BLOCK) return false;
    if (stmt->as.if_stmt.else_branch == NULL || stmt->as.if_stmt.else_branch->type != AST_STMT_BLOCK) return false;

    return true;
}

static bool validate_while_stmt(ASTNode* root) {
    if (root == NULL || root->type != AST_STMT_BLOCK || root->as.block.count == 0) return false;

    ASTNode* stmt = root->as.block.statements[0];
    if (stmt == NULL || stmt->type != AST_STMT_WHILE) return false;

    ASTNode* cond = stmt->as.while_stmt.condition;
    if (cond == NULL || cond->type != AST_EXPR_BINARY) return false;

    if (stmt->as.while_stmt.body == NULL || stmt->as.while_stmt.body->type != AST_STMT_BLOCK) return false;

    return true;
}

static bool validate_for_stmt(ASTNode* root) {
    if (root == NULL || root->type != AST_STMT_BLOCK || root->as.block.count == 0) return false;

    ASTNode* stmt = root->as.block.statements[0];
    if (stmt == NULL || stmt->type != AST_STMT_FOR) return false;

    if (strncmp(stmt->as.for_stmt.var_name, "i", 1) != 0) return false;

    ASTNode* iterable = stmt->as.for_stmt.iterable;
    if (iterable == NULL || iterable->type != AST_CALL) return false;
    if (iterable->as.call.callee == NULL || iterable->as.call.callee->type != AST_EXPR_VARIABLE) return false;
    if (strncmp(iterable->as.call.callee->as.variable.name, "range", 5) != 0) return false;

    if (stmt->as.for_stmt.body == NULL || stmt->as.for_stmt.body->type != AST_STMT_BLOCK) return false;

    return true;
}

static bool validate_binary_precedence(ASTNode* root) {
    if (root == NULL || root->type != AST_STMT_BLOCK || root->as.block.count == 0) return false;

    ASTNode* stmt = root->as.block.statements[0];
    if (stmt == NULL || stmt->type != AST_STMT_EXPR) return false;

    ASTNode* assign = stmt->as.expr_stmt.expression;
    if (assign == NULL || assign->type != AST_EXPR_ASSIGNMENT) return false;

    ASTNode* expr = assign->as.assignment.value;
    if (expr == NULL || expr->type != AST_EXPR_BINARY) return false;
    if (expr->as.binary.op != TOKEN_PLUS) return false;

    ASTNode* right = expr->as.binary.right;
    if (right == NULL || right->type != AST_EXPR_BINARY) return false;
    if (right->as.binary.op != TOKEN_STAR) return false;

    return true;
}

static bool validate_unary_and_grouping(ASTNode* root) {
    if (root == NULL || root->type != AST_STMT_BLOCK || root->as.block.count == 0) return false;

    ASTNode* stmt = root->as.block.statements[0];
    if (stmt == NULL || stmt->type != AST_STMT_EXPR) return false;

    ASTNode* assign = stmt->as.expr_stmt.expression;
    if (assign == NULL || assign->type != AST_EXPR_ASSIGNMENT) return false;

    ASTNode* expr = assign->as.assignment.value;
    if (expr == NULL || expr->type != AST_EXPR_UNARY) return false;
    if (expr->as.unary.op != TOKEN_MINUS) return false;

    ASTNode* operand = expr->as.unary.operand;
    if (operand == NULL || operand->type != AST_EXPR_BINARY) return false;
    if (operand->as.binary.op != TOKEN_PLUS) return false;

    return true;
}

static bool validate_return_stmt(ASTNode* root) {
    if (root == NULL || root->type != AST_STMT_BLOCK || root->as.block.count == 0) return false;

    ASTNode* stmt = root->as.block.statements[0];
    if (stmt == NULL || stmt->type != AST_STMT_RETURN) return false;

    ASTNode* val = stmt->as.return_stmt.value;
    if (val == NULL || val->type != AST_LITERAL_INT) return false;
    if (val->as.int_val != 0) return false;

    return true;
}

// -----------------------------------------------------------------------------
// MAIN & TEST SUITE
// -----------------------------------------------------------------------------

int main(void) {
    TestCase tests[] = {
        // Control Structures and declarations
        {
            .name = "1. Function declarations with parameters and body",
            .source = "func sum(int a, int b) { return a + b; }",
            .validate = validate_func_decl,
            .should_fail = false
        },
        {
            .name = "2. Function calls with expressions and argument lists",
            .source = "resultado = suma(10, 20);",
            .validate = validate_func_call,
            .should_fail = false
        },
        {
            .name = "3. Variable declaration with explicit type and initialization",
            .source = "int my_var = 42;",
            .validate = validate_var_decl,
            .should_fail = false
        },
        {
            .name = "4. Conditional statement with then and else branches",
            .source = "if (x > 0) { y = 1; } else { y = 0; }",
            .validate = validate_if_stmt,
            .should_fail = false
        },
        {
            .name = "5. While loop with condition and block body",
            .source = "while (i < 10) { i = i + 1; }",
            .validate = validate_while_stmt,
            .should_fail = false
        },
        {
            .name = "6. For loop over range construct",
            .source = "for i in range(0, 10) { print(i); }",
            .validate = validate_for_stmt,
            .should_fail = false
        },
        {
            .name = "7. Binary operator precedence (addition and multiplication)",
            .source = "res = x + y * z;",
            .validate = validate_binary_precedence,
            .should_fail = false
        },

        // Grouping, unary and returns
        {
            .name = "8. Unary minus with grouping expression",
            .source = "res = -(a + b);",
            .validate = validate_unary_and_grouping,
            .should_fail = false
        },
        {
            .name = "9. Return statement with expression value",
            .source = "return 0;",
            .validate = validate_return_stmt,
            .should_fail = false
        },

        // Error Cases
        {
            .name = "10. Error handling: Missing closing parenthesis in condition",
            .source = "if (x > 0 { y = 1; }",
            .validate = NULL,
            .should_fail = true
        },
        {
            .name = "11. Error handling: Invalid assignment target (assigning to literal)",
            .source = "42 = x;",
            .validate = NULL,
            .should_fail = true
        },
        {
            .name = "12. Error handling: Missing semicolon at end of statement",
            .source = "int x = 10",
            .validate = NULL,
            .should_fail = true
        }
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;

    printf("==================================================\n");
    printf("RUNNING PARSER AUTOMATED TEST SUITE\n");
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