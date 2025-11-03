// test.c
int foo(int n) {
    int x = 42;      
    int y = 3 + 5;   
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int z = i * 2;  
        sum += x + y;   
        sum += z;      
    }
    return sum;
}

