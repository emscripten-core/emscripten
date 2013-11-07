

#include <stdlib.h>
#include "Relooper.h"

int main() {
  #define SIZE (10*1024*1024)
  char *buffer = (char*)malloc(SIZE);
  Relooper::SetOutputBuffer(buffer, SIZE);

  Block *b0 = new Block("print('entry'); var label; var state; var decisions = [4, 1, 7, 2, 6, 6, 8]; var index = 0; function check() { if (index == decisions.length) throw 'HALT'; return decisions[index++] }", "the_var");
  Block *b1 = new Block("print(1); state = check();", "the_var");
  Block *b2 = new Block("print(2); state = check();", "the_var");
  Block *b3 = new Block("print(3); state = check();", "the_var");
  Block *b4 = new Block("print(4); state = check();", "the_var");
  Block *b5 = new Block("print(5); state = check();", "the_var");
  Block *b6 = new Block("print(6); state = check();", "the_var");
  Block *b7 = new Block("print(7); state = check();", "the_var");
  Block *b8 = new Block("print(8); state = check();", "the_var");
  b0->AddBranchTo(b5, NULL);
  b1->AddBranchTo(b3, NULL);
  b2->AddBranchTo(b1, NULL);
  b3->AddBranchTo(b8, "state == 8");
  b3->AddBranchTo(b1, NULL);
  b4->AddBranchTo(b3, "state == 3");
  b4->AddBranchTo(b1, NULL);
  b5->AddBranchTo(b6, NULL);
  b6->AddBranchTo(b7, "state == 7");
  b6->AddBranchTo(b1, NULL);
  b7->AddBranchTo(b2, NULL);
  b8->AddBranchTo(b4, "state == 4");
  b8->AddBranchTo(b2, NULL);

  Relooper r;
  r.AddBlock(b0);
  r.AddBlock(b1);
  r.AddBlock(b2);
  r.AddBlock(b3);
  r.AddBlock(b4);
  r.AddBlock(b5);
  r.AddBlock(b6);
  r.AddBlock(b7);
  r.AddBlock(b8);

  r.Calculate(b0);
  printf("\n\n");
  r.Render();

  puts(buffer);

  return 1;
}
