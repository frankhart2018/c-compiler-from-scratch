#include "chibicc.h"

// All local variable instances created during parsing are
// accumulated to this list.
Obj *locals;

static Node *compound_stmt(Token **rest, Token *tok, Const *cons);
static Node *expr(Token **rest, Token *tok, Const *cons);
static Node *expr_stmt(Token **rest, Token *tok, Const *cons);
static Node *assign(Token **rest, Token *tok, Const *cons);
static Node *equality(Token **rest, Token *tok, Const *cons);
static Node *relational(Token **rest, Token *tok, Const *cons);
static Node *add(Token **rest, Token *tok, Const *cons);
static Node *mul(Token **rest, Token *tok, Const *cons);
static Node *unary(Token **rest, Token *tok, Const *cons);
static Node *primary(Token **rest, Token *tok, Const *cons);

// Find a local variable by name
static Obj *find_var(Token *tok) {
    for (Obj *var = locals; var; var = var->next) {
        if (strlen(var->name) == tok->len && !strncmp(tok->loc, var->name, tok->len))
            return var;
    }

    return NULL;
}

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

static Node *new_var_node(Obj *var) {
    Node *node = new_node(ND_VAR);
    node->var = var;
    return node;
}

static Obj *new_lvar(char *name) {
    Obj *var = calloc(1, sizeof(Obj));
    var->name = name;
    var->next = locals;
    locals = var;
    return var;
}

// stmt = "return" expr ";" | "{" compound-stmt | expr-stmt
static Node *stmt(Token **rest, Token *tok, Const *cons) {
    if (equal(tok, "return")) {
        Node *node = new_unary(ND_RETURN, expr(&tok, tok->next, cons));
        *rest = skip(tok, ";");
        return node;
    }

    if (equal(tok, "{"))
        return compound_stmt(rest, tok->next, cons);

    return expr_stmt(rest, tok, cons);
}

// compound-stmt = stmt* "}"
static Node *compound_stmt(Token **rest, Token *tok, Const *cons) {
    Node head = {};
    Node *cur = &head;
    while (!equal(tok, "}"))
        cur = cur->next = stmt(&tok, tok, cons);

    Node *node = new_node(ND_BLOCK);
    node->body = head.next;
    *rest = tok->next;
    return node;
}

// expr-stmt = expr ";"
static Node *expr_stmt(Token **rest, Token *tok, Const *cons) {
    Node *node = new_unary(ND_EXPR_STMT, expr(&tok, tok, cons));
    *rest = skip(tok, ";");
    return node;
}

// expr = assign
static Node *expr(Token **rest, Token *tok, Const *cons) {
    return  assign(rest, tok, cons);
}

// assign = equality ("=" equality)?
static Node *assign(Token **rest, Token *tok, Const *cons) {
    Node *node = equality(&tok, tok, cons);

    if (equal(tok, "=")) {
        node = new_binary(ND_ASSIGN, node, equality(&tok, tok->next, cons));
    }

    *rest = tok;
    return node;
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

// primary = "(" expr ")" | ident | num
static Node *primary(Token **rest, Token *tok, Const *cons) {
    if (equal(tok, "(")) {
        Node *node = expr(&tok, tok->next, cons);
        *rest = skip(tok, ")");
        return node;
    }

    if (tok->kind == TK_IDENT) {
        Obj *var = find_var(tok);
        if (!var) {
            var = new_lvar(strndup(tok->loc, tok->len));
        }
        *rest = tok->next;
        return new_var_node(var);
    }

    if (tok->kind == TK_NUM) {
        Node *node = new_num(tok->val);
        *rest = tok->next;
        return node;
    }

    error_tok(tok, "Expected an expression");
    return NULL;
}

// program = stmt*
Function *parse(Token *tok, Const *cons) {
    tok = skip(tok, "{");

    Function *prog = calloc(1, sizeof(Function));
    prog->body = compound_stmt(&tok, tok, cons);
    prog->locals = locals;

    return prog;
}