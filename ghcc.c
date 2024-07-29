#include <stdio.h>
#include "ghcc.h"

char* user_input;

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が不一致");
        return 1;
    }

    user_input = argv[1];

    parse(argv[1]);
    generate();
    return 0;
}
