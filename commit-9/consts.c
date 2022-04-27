#include "chibicc.h"

Const init_const() {
    Const cons;
    cons.requires_le_function = false;
    cons.requires_leq_function = false;
    cons.requires_ne_function = false;

    return cons;
}