#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    char *p = argv[1];
    
    printf("LOAD %ld\n", strtol(p, &p, 10));

    while (*p) {
        if (*p == '+') {
            p++;
            printf("LOAD %ld\n", strtol(p, &p, 10));
            printf("ADD\n");
            continue;
        }

        if (*p == '-') {
            p++;
            printf("LOAD %ld\n", strtol(p, &p, 10));
            printf("SUB\n");
            continue;
        }

        fprintf(stderr, "unexpected character: '%c'\n", *p);
        return 1;
    }

    printf("SHOW\n");
    printf("HALT\n");
}