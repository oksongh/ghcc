#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "ghcc.h"

/*
    抽象構文木
 */

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

LVar* locals;
LVar* find_lvar(Token* tok) {
    for (LVar* var = locals; var; var = var->next) {
        if (var->len == tok->len && memcmp(var->name, tok->str, var->len) == 0) {
            return var;
        }
    }
    return NULL;
}

Node* code[100];

void program() {
    size_t i = 0;
    for (; !at_eof(); i++) {
        code[i] = stmt();
    }
    code[i] = NULL;
}

Node* stmt() {
    Node* node = expr();
    consume(";");
    return node;
}

Node* expr() {
    return assign();
}
Node* assign() {
    Node* node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }

    return node;
}

Node* equality() {
    Node* node = relational();
    while (true) {
        if (consume("==")) {
            node = new_node(ND_EQU, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NEQ, node, relational());
        } else {
            return node;
        }
    }
}
Node* relational() {
    // todo
    Node* node = add();
    while (true) {
        if (consume("<")) {
            node = new_node(ND_LSS, node, add());

        } else if (consume("<=")) {
            node = new_node(ND_LEQ, node, add());

        } else if (consume(">")) {
            node = new_node(ND_LSS, add(), node);

        } else if (consume(">=")) {
            node = new_node(ND_LEQ, add(), node);

        } else {
            return node;
        }
    }
}

// mul ('+' mul | '-' mul)*
Node* add() {
    Node* node = mul();

    while (true) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

// unary ('*' unary | '\' unary)*
Node* mul() {
    Node* node = unary();
    while (true) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}
// "'+'? primary
Node* unary() {
    if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    consume("+");
    return primary();
}

Node* primary() {
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }
    Token* tok = consume_ident();

    if (tok) {
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        LVar* lvar = find_lvar(tok);

        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;

            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = 8 + (locals ? locals->offset : 0);
            node->offset = lvar->offset;
            locals = lvar;
        }

        return node;
    }

    return new_node_num(expect_number());
}

void gen_lval(Node* node) {
    if (node->kind != ND_LVAR) {
        error("not left value");
    }
    // スタック上の変数のポインタを、計算に使うためにスタックの天辺にロード
    // ポインタなので代入にも計算にも使える
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen(Node* node) {
    switch (node->kind) {
        case ND_NUM:
            printf("    push %d\n", node->val);
            return;

        case ND_LVAR:
            gen_lval(node);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;

        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("    pop rdi\n");  // right value
            printf("    pop rax\n");  // left value (pointer)
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    // a <op> b
    printf("    pop rdi\n");  // b
    printf("    pop rax\n");  // a

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
        case ND_EQU:
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax,al\n");
            break;
        case ND_LEQ:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax,al\n");
            break;
        case ND_LSS:
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax,al\n");
            break;
        case ND_NEQ:
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax,al\n");
            break;
        default:
            error("unexpected state");
    }
    printf("    push rax\n");
}

void generate() {
    program();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域確保
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", 26 * 8);

    for (size_t i = 0; code[i]; i++) {
        gen(code[i]);

        // 計算した値が残っているためpop
        printf("    pop rax\n");
    }

    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}
