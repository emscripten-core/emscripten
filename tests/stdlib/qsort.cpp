#include <stdlib.h>
#include <stdio.h>

static int intcmp (const void *a, const void *b) {
    return *(int *) a - *(int *) b;
}

static int intcmp_s (const void *a, const void *b, void *ctx) {
    (*(int *) ctx)++;
    return intcmp(a, b);
}

int main (void) {
    int arr1[] = {1, 5, 3, 16, 8, 2, 9, 13};
    int arr2[] = {6, 10, 3, 4, 9, 8, 3, 6};
    int arr3[] = {7, 9, 12, 4, 1, 13, 8, 5};
    int elems_sorted = 0;

    qsort(arr1, 8, sizeof(int), intcmp);
    qsort_s(arr2, 8, sizeof(int), intcmp_s, &elems_sorted);
    qsort_r(arr3, 8, sizeof(int), intcmp_s, &elems_sorted);

    for (int i = 0; i < 8; i++) {
        if (i) printf(", ");
        printf("%d", arr1[i]);
    }
    printf("\n");
    for (int i = 0; i < 8; i++) {
        if (i) printf(", ");
        printf("%d", arr2[i]);
    }
    printf("\n");
    for (int i = 0; i < 8; i++) {
        if (i) printf(", ");
        printf("%d", arr3[i]);
    }
    printf("\n");
    exit(0);
}
