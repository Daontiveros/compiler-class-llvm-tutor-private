// dead_store1.c
#include <stdio.h>

int main() {
    int x = 0;    // Store 1
    x = 5;        // Store 2 (overwrites Store 1, Store 1 is dead)
    printf("%d\n", x);
    return 0;
}