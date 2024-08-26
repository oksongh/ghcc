#ifndef GHCC_STRING_HEADER_FILE
#define GHCC_STRING_HEADER_FILE

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    int len;
    char* chars;
} string;

string* new_string(char* chars, int len);

bool null_terminated_equals(char* chars, string* str);
bool string_equals(string* str1, string* str2);
#endif  // GHCC_STRING_HEADER_FILE
