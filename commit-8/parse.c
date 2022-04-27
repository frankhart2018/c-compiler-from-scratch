#include "chibicc.h"

static Node *expr(Token **rest, Token *tok, Const *cons);
static Node *equality(Token **rest, Token *tok, Const *cons);
static Node *relational(Token **rest, Token *tok, Const *cons);
static Node *add(Token **rest, Token *tok, Const *cons);
static Node *mul(Token **rest, Token *tok, Const *cons);
static Node *unary(Token **rest, Token *tok, Const *cons);
static Node *primary(Token **rest, Token *tok, Const *cons);

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

Node *parse(Token *tok, Const *cons) {
    Node *node = expr(&tok, tok, cons);

    if (tok->kind != TK_EOF) {
        error_tok(tok, "Extra token!");
    }

    return node;
}