#if !defined(GHCC_HEADER_FILE)
#define GHCC_HEADER_FILE

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char* user_input;
void error(char* fmt, ...);
void error_at(char* loc, char* fmt, ...);

int ghcc(char* code);

typedef enum {
    TK_RETURN,
    TK_RESERVED,  // 記号
    TK_IDENT,     // 識別子
    TK_NUM,       // 整数トークン
    TK_EOF,       // 入力終了トークン
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token* next;
    int val;    // kind == TK_NUMのときの数値
    char* str;  // トークン文字列
    int len;    // トークン長
};

void eprint_token_list(Token* tok);

bool is_alpha(char c);

bool is_token_target(char c);

// 次のトークンが期待している記号のときはトークンを読み進めてreturn true
// othrewise false;
bool consume(char* op);

Token* consume_token();

void expect(char* op);
int expect_number();

bool at_eof();

// link token list
Token* new_token(TokenKind kind, Token* cur, char* str, int len);

bool contains(char* p, int plen, char** strs, int n);

// 文字列pをトークナイズ
Token* tokenize(char* p);
void parse(char* uin);

typedef enum {
    ND_EQU,  // ==
    ND_NEQ,  // !=
    ND_LEQ,  // <=
    ND_LSS,  // <
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_ASSIGN,  // assign
    ND_STMT,    // statement
    ND_LVAR,    // local variable
    ND_RETURN
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    int val;
    int offset;
};

typedef struct LVar LVar;
struct LVar {
    LVar* next;
    char* name;  // 変数名
    int len;     // 変数名の長さ
    int offset;  // RBPからのoffset
};

Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);

LVar* find_lvar(Token* tok);

void program();
Node* stmt();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();

void gen(Node* node);

void generate();

#endif  // GHCC_HEADER_FILE
