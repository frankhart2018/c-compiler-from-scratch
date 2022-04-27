#ifndef CHIBICC_H
#define CHIBICC_H

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Consts
//
typedef struct {
    bool requires_le_function;
    bool requires_leq_function;
    bool requires_ne_function;
} Const;

Const init_const();

//
// tokenize.c
//

typedef enum {
    TK_PUNCT,       // Punctuators
    TK_NUM,         // Numeric literals
    TK_EOF,         // End of file
} TokenKind;

// Token type
typedef struct Token Token;
struct Token {
    TokenKind kind; // Token kind
    Token *next;    // Next token
    int val;        // If kind is TK_NUM, its value
    char *loc;      // Location of token
    int len;        // Length of token
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize(char *input);

//
// parse.c
//

typedef enum {
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_NEG,       // -
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_EXPR_STMT, // Expression statement
    ND_NUM,       // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
    NodeKind kind; // Node kind
    Node *next;    // Next node
    Node *lhs;     // Left-hand side
    Node *rhs;     // Right-hand side
    int val;       // If kind is ND_NUM, its value
};

Node *parse(Token *tok, Const *cons);

//
// codegen.c
//

void codegen(Node *node, Const *cons);

//
// helpers.c
//

void print_tokens(Token *tok);
void walk_ast(Node *node, int tablevel);

#endif