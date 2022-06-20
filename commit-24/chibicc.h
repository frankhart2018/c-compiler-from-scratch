#ifndef CHIBICC_H
#define CHIBICC_H

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;
typedef struct Node Node;

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

// Token
typedef enum {
    TK_IDENT,       // Identifiers
    TK_PUNCT,       // Punctuators
    TK_KEYWORD,     // Keywords
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
bool consume(Token **rest, Token *tok, char *str);
Token *tokenize(char *input);

//
// parse.c
//

// Local variable
typedef struct Obj Obj;
struct Obj {
    Obj *next;
    char *name; // Variable name
    Type *ty;   // Type
};

// Function
typedef struct Function Function;
struct Function {
    Node *body;
    Obj *locals;
    int stack_size;
};

// AST node
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
    ND_ASSIGN,    // =
    ND_ADDR,      // unary &
    ND_DEREF,     // unary *
    ND_RETURN,    // "return"
    ND_IF,        // "if"
    ND_FOR,       // "for" or "while"
    ND_BLOCK,     // { ... }
    ND_FUNCALL,   // Function call
    ND_EXPR_STMT, // Expression statement
    ND_VAR,       // Variable
    ND_NUM,       // Integer
} NodeKind;

// AST node type
struct Node {
    NodeKind kind; // Node kind
    Node *next;    // Next node
    Type *ty;
    Token *tok;    // Representative token

    Node *lhs;     // Left-hand side
    Node *rhs;     // Right-hand side

    // "if" or "for" statement
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    // Block
    Node *body;

    // Function call
    char *funcname;
    Node *args;

    Obj *var;      // If kind is ND_VAR, its object
    int val;       // If kind is ND_NUM, its value
};

Function *parse(Token *tok, Const *cons);

//
// type.c
//

typedef enum {
    TY_INT,
    TY_PTR,
} TypeKind;

struct Type {
    TypeKind kind;

    // Pointer
    Type *base;

    // Declaration
    Token *name;
};

extern Type *ty_int;

bool is_integer(Type *ty);
Type *pointer_to(Type *base);
void add_type(Node *node);

//
// codegen.c
//

void codegen(Function *prog, Const *cons);

//
// helpers.c
//

void print_tokens(Token *tok);
void walk_ast(Node *node, int tablevel);

#endif