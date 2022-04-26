#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Global variables
//

typedef struct {
    bool requires_le_function;
    bool requires_leq_function;
    bool requires_ne_function;
} Const;

Const init_const() {
    Const cons;
    cons.requires_le_function = false;
    cons.requires_leq_function = false;
    cons.requires_ne_function = false;

    return cons;
}

//
// Tokenizer
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

// Input string
static char *current_input;

// Reports an error and exits
static void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Reports an error location and exit.
static void verror_at(char *loc, char *fmt, va_list ap) {
    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, ""); // print pos spaces.
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

static void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

static void error_tok(Token *tok, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(tok->loc, fmt, ap);
}

// Consumes the current token if it matches `s`
static bool equal(Token *tok, char *op) {
    return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

// Ensure that the current token is `s`
static Token *skip(Token *tok, char *s) {
    if (!equal(tok, s)) 
        error_tok(tok, "Expected '%s'", s);

    return tok->next;
}

// Ensure that the current token is TK_NUM
static int get_number(Token *tok) {
    if (tok->kind != TK_NUM) 
        error_tok(tok, "Expected a number!");

    return tok->val;
}

// Create a new token
static Token *new_token(TokenKind kind, char *start, char *end) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = start;
    tok->len = end - start;
    return tok;
}

static bool startswith(char *p, char *q) {
    return strncmp(p, q, strlen(q)) == 0;
}

// Read a punctuator token from p and return its length
static int read_punct(char *p) {
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">="))
        return 2;

    return ispunct(*p) ? 1 : 0;
}

// Tokenize `p` and return new tokens
static Token *tokenize(void) {
    char *p = current_input;
    Token head = {};
    Token *cur = &head;

    while (*p) {
        // Skip whitespace characters
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Numeric literal
        if (isdigit(*p)) {
            cur = cur->next = new_token(TK_NUM, p, p);
            char *q = p;
            cur->val = strtoul(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        // Puncuators
        int punct_len = read_punct(p);
        if (punct_len) {
            cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
            p += cur->len;
            continue;
        }

        error_at(p, "Invalid token");
    }

    cur = cur->next = new_token(TK_EOF, p, p);
    return head.next;
}

//
// Parser
//

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NEG, // -
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_NUM, // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
    NodeKind kind; // Node kind
    Node *lhs;    // Left-hand side
    Node *rhs;    // Right-hand side
    int val;     // If kind is ND_NUM, its value
};

static Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_unary(NodeKind kind, Node *lhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    return node;
}

static Node *new_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

static Node *expr(Token **rest, Token *tok, Const *cons);
static Node *equality(Token **rest, Token *tok, Const *cons);
static Node *relational(Token **rest, Token *tok, Const *cons);
static Node *add(Token **rest, Token *tok, Const *cons);
static Node *mul(Token **rest, Token *tok, Const *cons);
static Node *unary(Token **rest, Token *tok, Const *cons);
static Node *primary(Token **rest, Token *tok, Const *cons);

// expr = equality
static Node *expr(Token **rest, Token *tok, Const *cons) {
    return  equality(rest, tok, cons);
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **rest, Token *tok, Const *cons) {
    Node *node = relational(&tok, tok, cons);

    for (;;) {
        if (equal(tok, "==")) {
            node = new_binary(ND_EQ, node, relational(&tok, tok->next, cons));
            continue;
        }

        if (equal(tok, "!=")) {
            cons->requires_ne_function = true;
            node = new_binary(ND_NE, node, relational(&tok, tok->next, cons));
            continue;
        }

        *rest = tok;
        return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **rest, Token *tok, Const *cons) {
    Node *node = add(&tok, tok, cons);

    for (;;) {
        if (equal(tok, "<")) {
            cons->requires_le_function = true;
            node = new_binary(ND_LT, node, add(&tok, tok->next, cons));
            continue;
        }

        if (equal(tok, "<=")) {
            cons->requires_leq_function = true;
            node = new_binary(ND_LE, node, add(&tok, tok->next, cons));
            continue;
        }

        if (equal(tok, ">")) {
            cons->requires_le_function = true;
            node = new_binary(ND_LT, add(&tok, tok->next, cons), node);
            continue;
        }

        if (equal(tok, ">=")) {
            cons->requires_leq_function = true;
            node = new_binary(ND_LE, add(&tok, tok->next, cons), node);
            continue;
        }

        *rest = tok;
        return node;
    }
}

// add = mul ("+" mul | "-" mul)*
static Node *add(Token **rest, Token *tok, Const *cons) {
    Node *node = mul(&tok, tok, cons);

    for (;;) {
        if (equal(tok, "+")) {
        node = new_binary(ND_ADD, node, mul(&tok, tok->next, cons));
        continue;
        }

        if (equal(tok, "-")) {
        node = new_binary(ND_SUB, node, mul(&tok, tok->next, cons));
        continue;
        }

        *rest = tok;
        return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **rest, Token *tok, Const *cons) {
    Node *node = unary(&tok, tok, cons);

    for (;;) {
        if (equal(tok, "*")) {
            node = new_binary(ND_MUL, node, unary(&tok, tok->next, cons));
            continue;
        }

        if (equal(tok, "/")) {
            node = new_binary(ND_DIV, node, unary(&tok, tok->next, cons));
            continue;
        }

        *rest = tok;
        return node;
    }
}

// unary = ("+" | "-") unary | primary
static Node *unary(Token **rest, Token *tok, Const *cons) {
    if (equal(tok, "+")) 
        return unary(rest, tok->next, cons);
    
    if (equal(tok, "-"))
        return new_unary(ND_NEG, unary(rest, tok->next, cons));

    return primary(rest, tok, cons);
}

// primary = "(" expr ")" | num
static Node *primary(Token **rest, Token *tok, Const *cons) {
    if (equal(tok, "(")) {
        Node *node = expr(&tok, tok->next, cons);
        *rest = skip(tok, ")");
        return node;
    }

    if (tok->kind == TK_NUM) {
        Node *node = new_num(tok->val);
        *rest = tok->next;
        return node;
    }

    error_tok(tok, "Expected an expression");
    return NULL;
}

//
// Code generator
//

static void gen_expr(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("LOAD %d\n", node->val);
            return;
        case ND_NEG:
            gen_expr(node->lhs);
            printf("NEG\n");
            return;
        default:
            break;
    }

    gen_expr(node->lhs);
    gen_expr(node->rhs);

    switch (node->kind) {
        case ND_ADD:
            printf("ADD\n");
            return;
        case ND_SUB:
            printf("SUB\n");
            return;
        case ND_MUL:
            printf("MUL\n");
            return;
        case ND_DIV:
            printf("DIV\n");
            return;
        case ND_EQ:
            printf("EQU\n");
            return;
        case ND_NE:
            printf("CALL %%ne\n");
            return;
        case ND_LT:
            printf("CALL %%le\n");
            return;
        case ND_LE:
            printf("CALL %%leq\n");
            return;
        default:
            error("Unexpected node kind %d", node->kind);
    }


    error("Invalid expression!");
}

void print_tokens(Token *tok) {
    for (; tok; tok = tok->next) {
        switch (tok->kind) {
            case TK_PUNCT: {
                printf("TK_PUNCT\n");
                break;
            }
            case TK_NUM: {
                printf("TK_NUM(%d)\n", tok->val);
                break;
            }
            default: {
                printf("TK\n");
            }
        }
    }
}

void walk_ast(Node *node, int tablevel) {
    while (node) {
        switch (node->kind) {
            case ND_ADD:
            case ND_SUB:
            case ND_MUL:
            case ND_DIV:
            case ND_NEG:
                printf("NodeKind: %d\n", node->kind);
                printf("Left: ");
                walk_ast(node->lhs, tablevel+1);
                printf("\nRight: ");
                walk_ast(node->rhs, tablevel+1);
                printf("\n");
                return;
            case ND_NUM:
                printf("\t%d\n", node->val);
                return;
            default:
                error("Unexpected node kind %d", node->kind);
        }
    }
}

void compile_relational_functions(Const cons) {
    if (cons.requires_le_function) {
        printf("%%le\n");
        printf("SUB\n");
        printf("JN %%less\n");
        printf("POP R0\n");
        printf("LOAD 0\n");
        printf("JMP %%end\n");
        printf("%%less\n");
        printf("POP R0\n");
        printf("LOAD 1\n");
        printf("%%end\n");
        printf("RET\n");
    }

    if (cons.requires_leq_function) {
        printf("%%leq\n");
        printf("SUB\n");
        printf("JN %%lesseq\n");
        printf("JZ %%lesseq\n");
        printf("POP R0\n");
        printf("LOAD 0\n");
        printf("JMP %%end\n");
        printf("%%lesseq\n");
        printf("POP R0\n");
        printf("LOAD 1\n");
        printf("%%end\n");
        printf("RET\n");
    }

    if (cons.requires_ne_function) {
        printf("%%ne\n");
        printf("EQU\n");
        printf("JZ %%neq\n");
        printf("POP R0\n");
        printf("LOAD 0\n");
        printf("JMP %%end\n");
        printf("%%neq\n");
        printf("POP R0\n");
        printf("LOAD 1\n");
        printf("%%end\n");
        printf("RET\n");
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    Const cons = init_const();

    // Tokenize and parse
    current_input = argv[1];
    Token *tok = tokenize();
    Node *node = expr(&tok, tok, &cons);

    // walk_ast(node, 0);

    if (tok->kind != TK_EOF) {
        error_tok(tok, "Extra token");
    }

    printf("JMP %%start\n");

    // Compile relational functions
    compile_relational_functions(cons);

    printf("%%start\n");
    gen_expr(node);
    printf("SHOW\n");
    printf("HALT\n");

    // assert(depth == 0);

    return 0;
}