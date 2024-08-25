#include "gh_string.h"

string* new_string(char* chars, int len) {
    string* str = calloc(1, sizeof(string));
    str->chars = chars;
    str->len = len;

    return str;
}

bool null_terminated_cmp(char* chars, string* str) {
    return (strlen(chars) == str->len) && (memcmp(chars, str->chars, str->len) == 0);
}

bool string_cmp(string* str1, string* str2) {
    return (str1->len == str2->len) &&
           (memcmp(str1->chars, str2->chars, str1->len) == 0);
}
