#include <stdio.h>

/* Store, "declare" (but jump over) the current B_* label */
#define IND_BLOCK(X)                        \
do {                                        \
            addrs[i] = &&B_##X;             \
            i++;                            \
            goto JMP_##X;                   \
    B_##X:                                  \
            printf(#X "\n");                \
            return 0;                       \
    JMP_##X:                                \
            ;                               \
} while (0)

/* Add an indirection block to enable token pasting */
#define SINGLE(X)   IND_BLOCK(X);

#define P2      SINGLE(__COUNTER__)    SINGLE(__COUNTER__)
#define P4      P2                     P2
#define P8      P4                     P4
#define P16     P8                     P8
#define P32     P16                    P16
#define P64     P32                    P32
#define P128    P64                    P64
#define P256    P128                   P128
#define P512    P256                   P256
#define P1024   P512                   P512

int main(int argc, char *argv[])
{
    const void *addrs[1024 + 512];
    int i = 0;

    /*
     * Repeat as many times as you want, but remember to update
     * the labels address array's size accordingly.
    */
    P1024;
    P512;

    /* jump back at the correct label */
    goto *addrs[(argc * argc) + 1000];
}
