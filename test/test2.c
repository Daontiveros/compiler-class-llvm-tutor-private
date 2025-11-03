#include <stdio.h>

int main() {
    int sum = 0;
    int i;
    
    // Initialize i and the loop sum
    for (i = 0; i < 10; ++i) {
        sum += i;
    }

    printf("Sum: %d\n", sum);
    return 0;
}

