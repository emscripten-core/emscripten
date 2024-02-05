#define _GNU_SOURCE

#include <search.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

static int tab[12];

static int cnt = 0;

static int compare(const void *pa, const void *pb) {
    if (*(int *) pa < *(int *) pb)
        return -1;
    if (*(int *) pa > *(int *) pb)
        return 1;
    return 0;
}

static void action(const void *nodep, VISIT which, int depth) {
    int *datap;

    switch (which) {
    case preorder:
        break;
    case postorder:
        datap = *(int **) nodep;
        assert(*datap == tab[cnt++]);
        printf("%6d\n", *datap);
        break;
    case endorder:
        break;
    case leaf:
        datap = *(int **) nodep;
        assert(*datap == tab[cnt++]);
        printf("%6d\n", *datap);
        break;
    }
}

int main(void) {
    int  ptr[12];
    void *val = NULL;
    void *root = NULL;

    for (int i = 0; i < 12; i++) {
        ptr[i] = i & 0xff;
        tab[i] = ptr[i];
        val = tsearch(&ptr[i], &root, compare);
        assert(val);
    }
    twalk(root, action);
    tdestroy(root, free);
    return 0;
} 
