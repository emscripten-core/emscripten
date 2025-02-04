#include <stdio.h>
int test_wrapper(void);

int f() {
    printf("f\n");
    return test_wrapper();
}
