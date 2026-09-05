#include "lexer.h"
#include <stdio.h>

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
    char* source = read_file("test.src");
    
    // Compilation...

    free(source);
    return 0;
}