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

Node* new_node_nth(NodeKind kind, Node* lhs, Node* rhs, Node* ths, Node* fhs) {
    Node* node = new_node(kind, lhs, rhs);
    node->ths = ths;
    node->fhs = fhs;
    return node;
}

Node* new_node_num(int val) {
    Node* node = new_node(ND_NUM, NULL, NULL);
    node->val = val;
    return node;
}

LVar* locals = NULL;
LVar* new_lvar_char(LVar* next, char* name, int len, int offset) {
    return new_lvar_str(next, new_string(name, len), offset);
}

LVar* new_lvar_str(LVar* next, string* name, int offset) {
    LVar* lvar = calloc(1, sizeof(LVar));
    lvar->next = next;
    lvar->name = name;
    lvar->offset = offset;
    return lvar;
}

LVar* find_lvar(Token* tok) {
    for (LVar* var = locals; var; var = var->next) {
        if (string_equals(var->name, tok->str)) {
            return var;
        }
    }
    return NULL;
}

void lvar_to_node(LVar* lvar, Node* node) {
    node->name = lvar->name;
    node->offset = lvar->offset;
}

// s:null終端済み
void define_label(char* s, int label_num) {
    printf("%s%03d", s, label_num);
    printf(":\n");
}

// s:null終端済み
void call_label(char* s, int label_num) {
    printf("%s%03d", s, label_num);
    printf("\n");
}
void debug_printf(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    fprintf(stderr, "DEBUG:");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

char* nodekind_to_str(NodeKind nk) {
    switch (nk) {
        case ND_EQU:
            return "ND_EQU";
        case ND_NEQ:
            return "ND_NEQ";
        case ND_LEQ:
            return "ND_LEQ";
        case ND_LSS:
            return "ND_LSS";
        case ND_ADD:
            return "ND_ADD";
        case ND_SUB:
            return "ND_SUB";
        case ND_MUL:
            return "ND_MUL";
        case ND_DIV:
            return "ND_DIV";
        case ND_NUM:
            return "ND_NUM";
        case ND_ASSIGN:
            return "ND_ASSIGN";
        case ND_STMT:
            return "ND_STMT";
        case ND_LVAR:
            return "ND_LVAR";
        case ND_RETURN:
            return "ND_RETURN";
        case ND_IF:
            return "ND_IF";
        case ND_FOR:
            return "ND_FOR";
        case ND_WHILE:
            return "ND_WHILE";
        case ND_BLOCK:
            return "ND_BLOCK";
        case ND_CALL_FUNC:
            return "ND_CALL_FUNC";
        case ND_DEF_FUNC:
            return "ND_DEF_FUNC";
        default:
            error("unexpected node kind:%d", nk);
    }
}

Node* code[100];

void program() {
    code[0] = def_func();

    size_t i = 1;
    for (; !at_eof(); i++) {
        code[i] = def_func();
    }
    code[i] = NULL;
}
Node* def_func() {
    // local変数初期化
    locals = NULL;

    Node* node = new_node(ND_DEF_FUNC, NULL, NULL);
    Token* func_name = consume_token(TK_IDENT);
    if (!func_name) {
        error("def func: not found indentifier");
    }
    node->name = func_name->str;
    expect("(");

    // 引数あり
    if (!consume(")")) {
        Node* args = new_node(ND_ELEM, NULL, NULL);
        node->lhs = args;

        // 最初の引数
        args->lhs = new_node(ND_LVAR, NULL, NULL);
        Token* tok_ident = consume_token(TK_IDENT);
        LVar* lvar = new_lvar_str(locals, tok_ident->str, 8);
        locals = lvar;

        lvar_to_node(lvar, args->lhs);

        for (Node* cur = args; !consume(")"); cur = cur->rhs) {
            expect(",");

            Token* tok_ident_ = consume_token(TK_IDENT);
            if (find_lvar(tok_ident_)) {
                error("redefined argument");
            }
            LVar* lvar_ = new_lvar_str(locals, tok_ident_->str, 8 + locals->offset);
            locals = lvar_;

            Node* argi = new_node(ND_LVAR, NULL, NULL);
            lvar_to_node(lvar_, argi);
            cur->rhs = new_node(ND_ELEM, argi, NULL);
        }
    }
    // block
    expect("{");

    Node* block = new_node(ND_BLOCK, NULL, NULL);
    node->rhs = block;

    for (Node* cur = block; !consume("}"); cur = cur->rhs) {
        cur->rhs = new_node(ND_ELEM, stmt(), NULL);
    }

    node->locals = locals;
    return node;
}

Node* stmt() {
    if (consume("{")) {
        Node* head = new_node(ND_BLOCK, NULL, NULL);

        for (Node* cur = head; !consume("}"); cur = cur->rhs) {
            cur->rhs = new_node(ND_ELEM, stmt(), NULL);
        }
        return head;
    }

    if (consume_token(TK_RETURN)) {
        Node* node = new_node(ND_RETURN, expr(), NULL);
        expect(";");
        return node;
    }

    if (consume_token(TK_IF)) {
        consume("(");
        Node* cond = expr();
        consume(")");

        Node* then = stmt();

        Node* else_ = NULL;
        if (consume_token(TK_ELSE)) {
            else_ = stmt();
        }

        return new_node_nth(ND_IF, cond, then, else_, NULL);
    }

    if (consume_token(TK_FOR)) {
        consume("(");

        Node *init = NULL, *cond = NULL, *update = NULL;

        if (!consume(";")) {
            init = expr();
            expect(";");
        }

        if (!consume(";")) {
            cond = expr();
            expect(";");
        }

        if (!consume(")")) {
            update = expr();
            expect(")");
        }
        Node* loop = stmt();
        return new_node_nth(ND_FOR, init, cond, update, loop);
    }

    if (consume_token(TK_WHILE)) {
        expect("(");
        Node* cond = expr();
        expect(")");
        Node* loop = stmt();

        return new_node(ND_WHILE, cond, loop);
    }

    Node* node = expr();
    expect(";");
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
    Token* tok_ident = consume_token(TK_IDENT);

    // number
    if (!tok_ident) {
        return new_node_num(expect_number());
    }

    // call func
    if (consume("(")) {
        Node* node = new_node(ND_CALL_FUNC, NULL, NULL);
        node->name = tok_ident->str;

        // no args
        if (consume(")")) {
            return node;
        }

        Node* args = new_node(ND_ELEM, expr(), NULL);
        for (Node* cur = args; !consume(")"); cur = cur->rhs) {
            expect(",");
            cur->rhs = new_node(ND_ELEM, expr(), NULL);
        }
        node->rhs = args;
        return node;
    }
    // local variable
    Node* node = new_node(ND_LVAR, NULL, NULL);
    LVar* lvar = find_lvar(tok_ident);
    if (lvar) {
        lvar_to_node(lvar, node);

    } else {
        lvar = new_lvar_str(locals, tok_ident->str, 8 + (locals ? locals->offset : 0));
        locals = lvar;
        lvar_to_node(lvar, node);
    }
    return node;
}

// 変数呼び出し時
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

static int jmp_label = 0;
void gen(Node* node) {
    // printf("# gen %s\n", nodekind_to_str(node->kind));
    int label_num = jmp_label++;

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

        case ND_RETURN:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;

        case ND_IF:
            // avoid to redefine local val:cond
            {
                Node* cond = node->lhs;
                Node* then = node->rhs;
                Node* else_ = node->ths;

                gen(cond);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");

                if (else_ == NULL) {
                    printf("    je .Lend%03d\n", label_num);
                    gen(then);
                } else {
                    printf("    je .Lelse%03d\n", label_num);
                    gen(then);
                    printf("    jmp .Lend%03d\n", label_num);
                    printf(".Lelse%03d:\n", label_num);
                    gen(else_);
                }

                printf(".Lend%03d:\n", label_num);
            }
            return;

        case ND_FOR:
            // avoid to redefine local val:cond
            {
                Node* init = node->lhs;
                Node* cond = node->rhs;
                Node* update = node->ths;
                Node* loop = node->fhs;

                if (init) {
                    gen(init);
                }
                define_label(".Lbegin", label_num);

                if (cond) {
                    gen(cond);
                    // pop, compare, 0ならgoto end;
                    printf("    pop rax\n");
                    printf("    cmp rax, 0\n");
                    call_label("    je .Lend", label_num);
                }

                if (loop) {
                    gen(loop);
                }

                if (update) {
                    gen(update);
                }

                call_label("    jmp .Lbegin", label_num);
                define_label(".Lend", label_num);
            }
            return;
        case ND_WHILE:
            // avoid to redefine local val:cond
            {
                Node* cond = node->lhs;
                Node* loop = node->rhs;

                define_label(".Lbegin", label_num);

                gen(cond);

                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                call_label("    je .Lend", label_num);

                gen(loop);
                call_label("    jmp .Lbegin", label_num);
                define_label(".Lend", label_num);
            }
            return;
        case ND_BLOCK:
            for (Node* cur = node->rhs; cur && cur->lhs; cur = cur->rhs) {
                gen(cur->lhs);
            }
            return;
        case ND_CALL_FUNC:

            if (node->rhs != NULL) {
                char* regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

                int cnt = 0;
                for (Node* cur = node->rhs; cur != NULL; cur = cur->rhs) {
                    // push evaluated cur to top
                    gen(cur->lhs);
                    cnt++;

                    if (cnt > 6) {
                        error("too many argument:%.*s", node->name->len, node->name->chars);
                    }
                }

                // i:1-indexed
                for (int i = cnt; 0 < i; i--) {
                    printf("    pop %s\n", regs[i - 1]);
                }
            }
            // ref:https://github.com/moizumi99/m99cc/blob/master/codegen.c#L227
            // 16バイトアライメント
            printf("    push r12\n");  // 後で使うr12を保存
            printf("    mov r12, rsp\n");
            // 16の倍数は,2進数で下4桁が0。~でnot,andでrsp下4桁が0になり不変or減少
            printf("    and rsp, ~0x0f\n");
            printf("    call %.*s\n", node->name->len, node->name->chars);  // 返り値はraxに
            printf("    mov rsp, r12\n");
            printf("    pop r12\n");   // r12を復元
            printf("    push rax\n");  // 戻り値
            return;
        case ND_DEF_FUNC:
            //
            {
                string* name = node->name;
                Node* args = node->lhs;
                Node* block = node->rhs;

                int variables_size = 0;
                for (Node* cur = args; cur; cur = cur->rhs) {
                    variables_size++;
                }
                for (LVar* cur = node->locals; cur; cur = cur->next) {
                    variables_size++;
                }

                printf("# define %.*s\n", name->len, name->chars);

                printf("%.*s:\n", name->len, name->chars);
                // prologue
                printf("    push rbp\n");
                printf("    mov rbp, rsp\n");
                // 引数と変数用の領域確保
                printf("    sub rsp, %d\n", variables_size * 8);

                // 引数代入
                if (args) {
                    char* regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
                    int i = 0;
                    for (Node* cur = args; cur; cur = cur->rhs) {
                        printf("    mov rax, rbp\n");
                        printf("    sub rax, %d\n", cur->lhs->offset);
                        printf("    mov [rax], %s\n", regs[i]);
                        i++;
                    }
                }

                printf("# end prologue\n");
                // statementsを生成
                for (Node* cur = block->rhs; cur; cur = cur->rhs) {
                    gen(cur->lhs);

                    // 計算した値がスタックに残っているためpopでリセット
                    printf("    pop rax\n");
                }
                // epilogue
                printf("    mov rsp, rbp\n");
                printf("    pop rbp\n");
                printf("    ret\n");
            }
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

    for (size_t i = 0; code[i]; i++) {
        gen(code[i]);
    }
}
