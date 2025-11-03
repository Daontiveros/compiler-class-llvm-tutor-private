#include <stdio.h>

int main() {
    int sum = 0;
    for (int i = 0; i < 5; ++i) { 
        for (int j = 0; j < 10; ++j) {
            sum += i * j; 
        }
    }
    return 0;
}