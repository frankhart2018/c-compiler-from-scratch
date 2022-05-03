#include "chibicc.h"

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
            case TK_EOF: {
                printf("TK_EOF\n");
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