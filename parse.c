#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "ghcc.h"

void error(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char* loc, char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", (int)(loc - user_input), " ");
    fprintf(stderr, "^");

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 着目するトークン
Token* token;

void eprint_token_list(Token* tok) {
    for (Token* cur = tok; cur != NULL; cur = cur->next) {
        char str[200];
        switch (cur->kind) {
            case TK_RESERVED:
            case TK_IDENT:
            case TK_IF:
            case TK_ELSE:
            case TK_FOR:
                snprintf(str, cur->len + 1 + 2, ":%.*s:", cur->len, cur->str);
                break;
            case TK_NUM:
                sprintf(str, ":%d:", cur->val);
                break;
            case TK_EOF:
                sprintf(str, ":%s:", "eof");
                break;
            case TK_RETURN:
                sprintf(str, ":%s:", "return");
                break;

            default:
                error_at(cur->str, "print token:unexpected token type");
                break;
        }
        fprintf(stderr, "%s", str);
        fprintf(stderr, "->");
    }
    fprintf(stderr, "\n");
}

bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_token_target(char c) {
    return is_alpha(c) || ('0' <= c && c <= '9') || (c == '_');
}

bool consume_keyword(TokenKind tk, Token** cur, char** p, char* keyword, int len) {
    // printf("%d&", strncmp(*p, keyword, len) == 0);
    // printf("%d\n\n", !is_token_target((*p)[len]));

    if (strncmp(*p, keyword, len) == 0 && !is_token_target((*p)[len])) {
        *cur = new_token(tk, *cur, *p, len);
        // fprintf(stderr, "^%.*s^\n", len, *p);
        *p += len;
        // fprintf(stderr, "^%.*s^\n", len, *p);
        return true;
    }
    return false;
}

// 次のトークンが期待している記号のときはトークンを読み進めてreturn true
// othrewise false;
bool consume(char* op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len || memcmp(op, token->str, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

Token* consume_token(TokenKind kind) {
    if (token->kind != kind) {
        return NULL;
    }

    Token* tmp = token;
    token = token->next;
    return tmp;
}

void expect(char* op) {
    if (!consume(op)) {
        error_at(token->str, "'%s'ではない", op);
    }
}
int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数ではない");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    // eprint_token_list(token);
    return token->kind == TK_EOF;
}

// link token list
Token* new_token(TokenKind kind, Token* cur, char* str, int len) {
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;

    cur->next = tok;
    return tok;
}

bool contains(char* p, int plen, char** strs, int n) {
    for (size_t i = 0; i < n; i++) {
        if (strncmp(p, strs[i], plen) == 0) {
            return true;
        }
    }
    return false;
}

// 文字列pをトークナイズ
Token* tokenize(char* p) {
    Token head;
    head.next = NULL;
    Token* cur = &head;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        char* ops_len2[] = {"<=", ">=", "==", "!="};
        if (contains(p, 2, ops_len2, sizeof(ops_len2) / sizeof(ops_len2[0]))) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>=;", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        if (consume_keyword(TK_RETURN, &cur, &p, "return", 6)) continue;

        if (consume_keyword(TK_IF, &cur, &p, "if", 2)) continue;

        if (consume_keyword(TK_ELSE, &cur, &p, "else", 4)) continue;

        if (consume_keyword(TK_FOR, &cur, &p, "for", 3)) continue;

        if (is_alpha(*p)) {
            char* start = p;

            while (is_alpha(*++p));
            // a 0
            // s p

            cur = new_token(TK_IDENT, cur, start, p - start);
            continue;
        }

        error_at(p, "cannot tokeinze");
    }
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
void parse(char* uin) {
    token = tokenize(uin);
    eprint_token_list(token);
    // exit(0);
}
