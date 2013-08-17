

#include <stdlib.h>
#include "Relooper.h"

int main() {
  #define SIZE (10*1024*1024)
  char *buffer = (char*)malloc(SIZE);
  Relooper::SetOutputBuffer(buffer, SIZE);

  Block *b0 = new Block("print('entry'); var label; var state; var decisions = [2, 2, 1, 3, 2, 2, 1, 3, 2, 3, 3, 1, 3, 2, 1]; var index = 0; function check() { if (index == decisions.length) throw 'HALT'; return decisions[index++] }", "the_var");
  Block *b1 = new Block("print(1); state = check();", "the_var");
  Block *b2 = new Block("print(2); state = check();", "the_var");
  Block *b3 = new Block("print(3); state = check();", "the_var");
  Block *b4 = new Block("print(4); state = check();", "the_var");
  b0->AddBranchTo(b2, "state == 2");
  b0->AddBranchTo(b4, NULL);
  b1->AddBranchTo(b1, NULL);
  b2->AddBranchTo(b2, NULL);
  b3->AddBranchTo(b1, NULL);
  b4->AddBranchTo(b4, "state == 4");
  b4->AddBranchTo(b3, NULL);

  Relooper r;
  r.AddBlock(b0);
  r.AddBlock(b1);
  r.AddBlock(b2);
  r.AddBlock(b3);
  r.AddBlock(b4);

  r.Calculate(b0);
  printf("\n\n");
  r.Render();

  puts(buffer);

  return 1;
}
