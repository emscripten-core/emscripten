#define _POSIX_SOURCE
#include <locale.h>
#include <ctype.h>
#include <stdio.h>

int main(void) {
    try {
        printf("*%i*\n", isdigit('0'));
        printf("*%i*\n", isdigit_l('0', LC_GLOBAL_LOCALE));
    }
    catch (...) {
        printf("EXCEPTION!\n");
    }
}

