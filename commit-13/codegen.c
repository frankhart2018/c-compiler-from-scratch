#include "chibicc.h"

static void gen_var(Node *node) {
    if (node->kind == ND_VAR) {
        printf("LOAD &%s\n", node->var->name);
        return;
    }

    error("Not an lvalue!");
}

static void gen_expr(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("LOAD %d\n", node->val);
            return;
        case ND_NEG:
            gen_expr(node->lhs);
            printf("NEG\n");
            return;
        case ND_VAR:
            gen_var(node);
            return;
        case ND_ASSIGN:
            gen_expr(node->rhs);
            printf("POP &%s\n", node->lhs->var->name);
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

void compile_relational_functions(Const *cons) {
    if (cons->requires_le_function) {
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

    if (cons->requires_leq_function) {
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

    if (cons->requires_ne_function) {
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

static void gen_stmt(Node *node) {
    switch (node->kind) {
        case ND_BLOCK:
            for (Node *n = node->body; n; n = n->next) 
                gen_stmt(n);
            return;
        case ND_RETURN: 
            gen_expr(node->lhs);
            printf("JMP %%l.return\n");
            return;
        case ND_EXPR_STMT:
            gen_expr(node->lhs);
            return;
        default:
            error("Unexpected node kind %d", node->kind);
    }

    error("Invalid statement!");
}

void codegen(Function *prog, Const *cons) {
    printf("JMP %%start\n");

    compile_relational_functions(cons);
    printf("%%start\n");
    
    gen_stmt(prog->body);

    printf("%%l.return\n");
    printf("SHOW\n");
    printf("HALT\n");
}