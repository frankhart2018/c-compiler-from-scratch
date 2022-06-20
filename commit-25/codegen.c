#include "chibicc.h"

static char *argreg[] = {"R0", "R1", "R2", "R3", "R4", "R5", "R6"};
static Function *current_fn;

static void gen_expr(Node *node);

static int count(void) {
    static int i = 1;
    return i++;
}

static void gen_var(Node *node) {
    switch (node->kind) {
        case ND_VAR:
            printf("LOAD &%s\n", node->var->name);
            return;
        case ND_DEREF:
            gen_expr(node->lhs);
            return;
        default:
            error("Unexpected node kind %d", node->kind);
    }

    error_tok(node->tok, "Not an lvalue!");
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
        case ND_DEREF:
            gen_expr(node->lhs);
            printf("DEREF\n");
            return;
        case ND_ADDR:
            printf("LOAD $%s\n", node->lhs->var->name);
            return;
        case ND_ASSIGN:
            gen_expr(node->rhs);
            switch (node->lhs->kind) {
                case ND_VAR:
                    printf("POP &%s\n", node->lhs->var->name);
                    return;
                case ND_DEREF:
                    switch (node->lhs->lhs->kind) {
                        case ND_VAR:
                            printf("POP *%s\n", node->lhs->lhs->var->name);
                            return;
                        case ND_ADD:
                        case ND_SUB:
                            gen_expr(node->lhs->lhs);
                            printf("POP &lval\n");
                            printf("POP *lval\n");
                            return;
                        default:
                            error("Unexpected node kind %d", node->lhs->lhs->kind);
                    }
                default:
                    error("Invalid node kind %d", node->lhs->kind);
            }
            return;
        case ND_FUNCALL: {
            int nargs = 0;
            for (Node *arg = node->args; arg; arg = arg->next) {
                gen_expr(arg);
                nargs++;
            }

            printf("CALL %%%s\n", node->funcname);
            return;
        }
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


    error_tok(node->tok, "Invalid expression!");
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
        case ND_IF: {
            int c = count();
            gen_expr(node->cond);
            printf("JZ %%l.else.%d\n", c);
            gen_stmt(node->then);
            printf("JMP %%l.end.%d\n", c);
            printf("%%l.else.%d\n", c);
            if (node->els)
                gen_stmt(node->els);
            printf("%%l.end.%d\n", c);
            return;
        }
        case ND_FOR: {
            int c = count();
            if (node->init) {
                gen_stmt(node->init);
            }
            printf("%%.l.begin.%d\n", c);
            if (node->cond) {
                gen_expr(node->cond);
                printf("JZ %%l.end.%d\n", c);
            }
            gen_stmt(node->then);
            if (node->inc) {
                gen_expr(node->inc);
            }
            printf("JMP %%.l.begin.%d\n", c);
            printf("%%l.end.%d\n", c);
        }
        case ND_BLOCK:
            for (Node *n = node->body; n; n = n->next) 
                gen_stmt(n);

            return;
        case ND_RETURN: 
            gen_expr(node->lhs);
            printf("JMP %%l.return.%s\n", current_fn->name);
            return;
        case ND_EXPR_STMT:
            gen_expr(node->lhs);
            return;
        default:
            error("Unexpected node kind %d", node->kind);
    }

    error_tok(node->tok, "Invalid statement!");
}

void codegen(Function *prog, Const *cons) {
    printf("JMP %%start\n");

    compile_relational_functions(cons);
    
    for (Function *fn = prog; fn; fn = fn->next) {
        printf("%%%s\n", fn->name);
        current_fn = fn;

        gen_stmt(fn->body);

        printf("%%l.return.%s\n", fn->name);
        printf("RET\n");
    }

    printf("%%start\n");
    printf("CALL %%main\n");
    printf("SHOW\n");
    printf("HALT\n");
}