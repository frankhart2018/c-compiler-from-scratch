#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Reports an error and exits
static void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Consumes the current token if it matches `s`
static bool equal(Token *tok, char *op) {
    return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

// Ensure that the current token is `s`
static Token *skip(Token *tok, char *s) {
    if (!equal(tok, s)) 
        error("Expected '%s'", s);

    return tok->next;
}

// Ensure that the current token is TK_NUM
static int get_number(Token *tok) {
    if (tok->kind != TK_NUM) 
        error("Expected a number!");

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

// Tokenize `p` and return new tokens
static Token *tokenize(char *p) {
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

        // Puncuator
        if (*p == '+' || *p == '-') {
            cur = cur->next = new_token(TK_PUNCT, p, p + 1);
            p++;
            continue;
        }

        error("Invalid token");
    }

    cur = cur->next = new_token(TK_EOF, p, p);
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    Token *tok = tokenize(argv[1]);
    
    // The first token must be a number
    printf("LOAD %d\n", get_number(tok));
    tok = tok->next;

    // Followed by either '+ <number>' or '- <number>'
    while (tok->kind != TK_EOF) {
        if (equal(tok, "+")) {
            printf("LOAD %d\n", get_number(tok->next));
            printf("ADD\n");
            tok = tok->next->next;
            continue;
        }

        tok = skip(tok, "-");
        printf("LOAD %d\n", get_number(tok));
        printf("SUB\n");
        tok = tok->next;
    }

    printf("SHOW\n");
    printf("HALT\n");
}