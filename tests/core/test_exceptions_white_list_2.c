#include <stdio.h>

void throwhere(void) {
    throw(1);
}

void (*funptr)(void) = throwhere;

void nocatch(void) {
    try {
        funptr();
    }
    catch (...) {
        printf("ERROR\n");
    }
}

int main(void) {
    try {
        nocatch();
    }
    catch (...) {
        printf("SUCCESS\n");
    }
    return 0;
}

