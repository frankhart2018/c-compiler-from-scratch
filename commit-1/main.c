#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    printf("LOAD %s\n", argv[1]);
    printf("SHOW\n");
    printf("HALT\n");
}