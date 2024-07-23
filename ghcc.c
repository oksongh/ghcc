#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,  // 記号
    TK_NUM,       // 整数トークン
    TK_EOF,       // 入力終了トークン
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token* next;
    int val;    // kind == TK_NUMのときの数値
    char* str;  // トークン文字列
};

// 着目
Token* token;

void error(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号のときはトークンを読み進めてreturn true
// othrewise false;
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char op) {
    if (!consume(op)) {
        error("'%c'ではない", op);
    }
}
int expect_number() {
    if (token->kind != TK_NUM) {
        error("数ではない");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// link token list
Token* new_token(TokenKind kind, Token* cur, char* str) {
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;

    return tok;
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
        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("cannot tokeinze");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}

// 数値 (記号(+,-) 数値),...
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が不一致");
        return 1;
    }
    token = tokenize(argv[1]);

    char* p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main :\n");

    printf("    mov rax, %d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
        } else {
            expect('-');
            printf("    sub rax, %d\n", expect_number());
        }
    }

    printf("    ret\n");
    return 0;
}
