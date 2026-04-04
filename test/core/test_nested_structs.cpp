#include <stdio.h>
#include "emscripten.h"

struct base {
  int x;
  float y;
  union {
    int a;
    float b;
  };
  char c;
};

struct hashtableentry {
  int key;
  base data;
};

struct hashset {
  typedef hashtableentry entry;
  struct chain { entry elem; chain *next; };
//  struct chainchunk { chain chains[100]; chainchunk *next; };
};

struct hashtable : hashset {
  hashtable() {
    base b;
    entry e;
    chain c;
    printf("*%zu,%ld,%ld,%ld,%ld,%ld|%zu,%ld,%ld,%ld,%ld,%ld,%ld,%ld|%zu,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld*\n",
      sizeof(base),
      long(&b.x) - long(&b),
      long(&b.y) - long(&b),
      long(&b.a) - long(&b),
      long(&b.b) - long(&b),
      long(&b.c) - long(&b),
      sizeof(hashtableentry),
      long(&e.key) - long(&e),
      long(&e.data) - long(&e),
      long(&e.data.x) - long(&e),
      long(&e.data.y) - long(&e),
      long(&e.data.a) - long(&e),
      long(&e.data.b) - long(&e),
      long(&e.data.c) - long(&e),
      sizeof(hashset::chain),
      long(&c.elem) - long(&c),
      long(&c.next) - long(&c),
      long(&c.elem.key) - long(&c),
      long(&c.elem.data) - long(&c),
      long(&c.elem.data.x) - long(&c),
      long(&c.elem.data.y) - long(&c),
      long(&c.elem.data.a) - long(&c),
      long(&c.elem.data.b) - long(&c),
      long(&c.elem.data.c) - long(&c)
    );
  }
};

struct B { char buffer[62]; int last; char laster; char laster2; };

struct Bits {
  unsigned short A : 1;
  unsigned short B : 1;
  unsigned short C : 1;
  unsigned short D : 1;
  unsigned short x1 : 1;
  unsigned short x2 : 1;
  unsigned short x3 : 1;
  unsigned short x4 : 1;
};

int main() {
  hashtable t;

  // Part 2 - the char[] should be compressed, BUT have a padding space at the end so the next
  // one is aligned properly. Also handle char; char; etc. properly.
  B b;
  printf("*%ld,%ld,%ld,%ld,%ld,%ld,%ld,%zu*\n", long(&b.buffer) - long(&b),
                                                long(&b.buffer[0]) - long(&b),
                                                long(&b.buffer[1]) - long(&b),
                                                long(&b.buffer[2]) - long(&b),
                                                long(&b.last) - long(&b),
                                                long(&b.laster) - long(&b),
                                                long(&b.laster2) - long(&b),
                                                sizeof(B));

  // Part 3 - bitfields, and small structures
  printf("*%zu*\n", sizeof(Bits));
  return 0;
}
