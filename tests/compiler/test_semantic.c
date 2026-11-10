#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

typedef struct {
    const char* name;
    const char* source;
    int expected_errors;
} SemanticTest;

static bool run_case(const SemanticTest* test, size_t index) {
    int saved_stderr = dup(STDERR_FILENO);
    int dev_null = open("/dev/null", O_WRONLY);
    if (saved_stderr != -1 && dev_null != -1) dup2(dev_null, STDERR_FILENO);
    if (dev_null != -1) close(dev_null);

    Lexer lexer;
    lexer_init(&lexer, test->source);

    Parser parser;
    parser_init(&parser, &lexer);
    ASTNode* program = parse_program(&parser);

    if (parser.had_error) {
        fflush(stderr);
        if (saved_stderr != -1) { dup2(saved_stderr, STDERR_FILENO); close(saved_stderr); }
        printf(" [FAIL] %zu. %s (parser rejected test program)\n", index, test->name);
        ast_free(program);
        return false;
    }

    SemanticAnalyzer analyzer;
    semantic_init(&analyzer);
    int errors = semantic_analyze(&analyzer, program);
    semantic_destroy(&analyzer);
    fflush(stderr);
    if (saved_stderr != -1) { dup2(saved_stderr, STDERR_FILENO); close(saved_stderr); }
    ast_free(program);

    if (errors != test->expected_errors) {
        printf(" [FAIL] %zu. %s (expected %d semantic error(s), got %d)\n",
               index, test->name, test->expected_errors, errors);
        return false;
    }

    printf(" [PASS] %zu. %s\n", index, test->name);
    return true;
}

int main(void) {
    const SemanticTest tests[] = {
        {
            "nested scopes can read outer variables",
            "int outer = 1; { int inner = outer; outer = inner; }",
            0
        },
        {
            "inner variable is unavailable after its block",
            "{ int inner = 1; } inner = 2;",
            1
        },
        {
            "same-scope redeclaration is rejected",
            "int value = 1; int value = 2;",
            1
        },
        {
            "inner scope may shadow outer variable",
            "int value = 1; { float value = 2.0; value = 3.0; } value = 2;",
            0
        },
        {
            "undeclared variable is rejected",
            "int value = missing;",
            1
        },
        {
            "assignment type mismatch is rejected",
            "int value = 1; value = True;",
            1
        },
        {
            "valid function call and return type",
            "func add(int a, int b) -> int { return a + b; } int result = add(2, 3);",
            0
        },
        {
            "forward function call works",
            "int result = twice(4); func twice(int n) -> int { return n * 2; }",
            0
        },
        {
            "wrong function argument count is rejected",
            "func one(int n) -> int { return n; } int result = one();",
            1
        },
        {
            "wrong function argument type is rejected",
            "func flag(bool b) -> bool { return b; } bool result = flag(1);",
            1
        },
        {
            "return type mismatch is rejected",
            "func count() -> int { return True; }",
            1
        },
        {
            "return outside function is rejected",
            "return 1;",
            1
        },
        {
            "if condition must be boolean",
            "if (1) { int value = 2; }",
            1
        },
        {
            "range loop iterator has local scope",
            "for i in range(0, 4) { int copy = i; }",
            0
        },
        {
            "range arguments must be integers",
            "for i in range(True) { int copy = i; }",
            1
        }
    };

    int failed = 0;
    size_t count = sizeof(tests) / sizeof(tests[0]);
    printf("==================================================\n");
    printf("RUNNING SEMANTIC ANALYZER AUTOMATED TEST SUITE\n");
    printf("==================================================\n");
    for (size_t i = 0; i < count; i++)
        if (!run_case(&tests[i], i + 1)) failed++;

    printf("--------------------------------------------------\n");
    printf("Results: %zu/%zu tests passed.\n", count - (size_t)failed, count);
    printf("==================================================\n");
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
