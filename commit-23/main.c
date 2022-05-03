#include "chibicc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    Const cons = init_const();

    Token *tok = tokenize(argv[1]);
    Function *prog = parse(tok, &cons);

    // Traverse the AST to emit assembly.
    codegen(prog, &cons);

    return 0;
}