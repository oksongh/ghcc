#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define DEBUG

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

// 着目するトークン
Token* token;

void eprint_token_list(Token* tok) {
    for (Token* cur = tok; cur != NULL; cur = cur->next) {
        char str[200];
        switch (cur->kind) {
            case TK_RESERVED:
                sprintf(str, ":%c:", cur->str[0]);
                break;
            case TK_NUM:
                sprintf(str, ":%d:", cur->val);
                break;
            case TK_EOF:
                sprintf(str, ":%s:", "eof");
                break;
            default:
                break;
        }
        fprintf(stderr, "%s", str);
        fprintf(stderr, "->");
    }
    fprintf(stderr, "\n");
}

void error(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

char* user_input;
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
        error_at(token->str, "'%c'ではない", op);
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
        if (strchr("+-*/()", *p)) {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "cannot tokeinze");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}

/*
    抽象構文木
 */
typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    int val;
};

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}
Node* new_node_num(int val) {
    Node* node = new_node(ND_NUM, NULL, NULL);
    node->val = val;
    return node;
}
Node* expr();
Node* mul();
Node* primary();

Node* expr() {
#ifdef DEBUG
    fprintf(stderr, "expr\n");
#endif
    Node* node = mul();

    while (true) {
        if (consume('+')) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume('-')) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node* mul() {
#ifdef DEBUG
    fprintf(stderr, "mul\n");
#endif

    Node* node = primary();
    while (true) {
        if (consume('*')) {
            node = new_node(ND_MUL, node, mul());
        } else if (consume('/')) {
            node = new_node(ND_DIV, node, mul());
        } else {
            return node;
        }
    }
}
Node* primary() {
#ifdef DEBUG
    fprintf(stderr, "primary\n");
#endif

    if (consume('(')) {
        Node* node = expr();
        expect(')');
        return node;
    }
    return new_node_num(expect_number());
}

void gen(Node* node) {
    if (node->kind == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("    add rax,rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax,rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax,rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
    }
    printf("    push rax\n");
}

// 数値 (記号(+,-) 数値),...
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が不一致");
        return 1;
    }
    user_input = argv[1];
    token = tokenize(argv[1]);
    eprint_token_list(token);
    Node* root = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(root);
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
