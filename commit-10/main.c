#include "chibicc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    Const cons = init_const();

    Token *tok = tokenize(argv[1]);
    Node *node = parse(tok, &cons);
    codegen(node, &cons);

    return 0;
}