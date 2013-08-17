
#include "Relooper.h"

int main() {
  char buffer[10000];

  Relooper::SetOutputBuffer(buffer, sizeof(buffer));

  printf("\n\n-- If pattern --\n\n");

  Block *b_a = new Block("// block A\n", "the_var");
  Block *b_b = new Block("// block B\n", "waka"); // never reached

  b_b->AddBranchTo(b_b, NULL);

  Relooper r;
  r.AddBlock(b_a);
  r.AddBlock(b_b);

  r.Calculate(b_a);
  printf("\n\n");
  r.Render();

  puts(buffer);

  printf("I did not crash even though I have dead code with a branch!\n");
}

