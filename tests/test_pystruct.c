/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

// Based on CPython code
union PyGC_Head {
    struct {
        union PyGC_Head *gc_next;
        union PyGC_Head *gc_prev;
        size_t gc_refs;
    } gc;
    long double dummy;  /* force worst-case alignment */
} ;

struct gc_generation {
    PyGC_Head head;
    int threshold; /* collection threshold */
    int count; /* count of allocations or collections of younger
                  generations */
};

#define NUM_GENERATIONS 3
#define GEN_HEAD(n) (&generations[n].head)

/* linked lists of container objects */
static struct gc_generation generations[NUM_GENERATIONS] = {
    /* PyGC_Head,                               threshold,      count */
    {{{GEN_HEAD(0), GEN_HEAD(0), 0}},           700,            0},
    {{{GEN_HEAD(1), GEN_HEAD(1), 0}},           10,             0},
    {{{GEN_HEAD(2), GEN_HEAD(2), 0}},           10,             0},
};

int main()
{
  gc_generation *n = NULL;
  printf("*%d,%d,%d,%d,%d,%d,%d,%d*\n",
    (int)(&n[0]),
    (int)(&n[0].head),
    (int)(&n[0].head.gc.gc_next),
    (int)(&n[0].head.gc.gc_prev),
    (int)(&n[0].head.gc.gc_refs),
    (int)(&n[0].threshold), (int)(&n[0].count), (int)(&n[1])
  );
  printf("*%d,%d,%d*\n",
    (int)(&generations[0]) ==
    (int)(&generations[0].head.gc.gc_next),
    (int)(&generations[0]) ==
    (int)(&generations[0].head.gc.gc_prev),
    (int)(&generations[0]) ==
    (int)(&generations[1])
  );
  int x1 = (int)(&generations[0]);
  int x2 = (int)(&generations[1]);
  printf("*%d*\n", x1 == x2);
  for (int i = 0; i < NUM_GENERATIONS; i++) {
    PyGC_Head *list = GEN_HEAD(i);
    printf("%d:%d,%d\n", i, (int)list == (int)(list->gc.gc_prev), (int)list ==(int)(list->gc.gc_next));
  }
  printf("*%d,%d,%d*\n", sizeof(PyGC_Head), sizeof(gc_generation), int(GEN_HEAD(2)) - int(GEN_HEAD(1)));
}
