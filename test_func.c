#include <stdio.h>

int testnoarg() {
    printf("OKOK\n");
    return 100;
}
int testIdentity(int x) {
    return x;
}

int testAdd(int a, int b) {
    return a + b;
}

int testWeightedSum(int a1, int a2, int a3, int a4, int a5, int a6) {
    printf("%d %d %d %d %d %d\n", a1, a2, a3, a4, a5, a6);
    return a1 + a2 / 2 + a3 / 3 + a4 / 4 + a5 / 5 + a6 / 6;
}
