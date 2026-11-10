#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "semantic.h"
#include "lexer.h"

char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) return NULL;

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source.axio>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char* source = read_file(argv[1]);
    if (!source) {
        fprintf(stderr, "Could not read source file '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    Lexer lexer;
    lexer_init(&lexer, source);
    Parser parser;
    parser_init(&parser, &lexer);
    ASTNode* program = parse_program(&parser);

    int result = EXIT_SUCCESS;
    if (parser.had_error) {
        result = EXIT_FAILURE;
    } else {
        SemanticAnalyzer analyzer;
        semantic_init(&analyzer);
        int errors = semantic_analyze(&analyzer, program);
        if (errors != 0) result = EXIT_FAILURE;
        semantic_destroy(&analyzer);
    }

    ast_free(program);
    free(source);
    return result;
}