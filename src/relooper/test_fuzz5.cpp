

#include <stdlib.h>
#include "Relooper.h"

int main() {
  #define SIZE (10*1024*1024)
  char *buffer = (char*)malloc(SIZE);
  Relooper::SetOutputBuffer(buffer, SIZE);

  Block *b0 = new Block("print('entry'); var label; var state; var decisions = [133, 98, 134, 143, 162, 187, 130, 87, 91, 49, 102, 47, 9, 132, 179, 176, 157, 25, 64, 161, 57, 107, 16, 167, 185, 45, 191, 180, 23, 131]; var index = 0; function check() { if (index == decisions.length) throw 'HALT'; return decisions[index++] }", "the_var");
  Block *b1 = new Block("print(1); state = check();", "the_var");
  Block *b2 = new Block("print(2); state = check();", "the_var");
  Block *b3 = new Block("print(3); state = check();", "the_var");
  Block *b4 = new Block("print(4); state = check();", "the_var");
  Block *b5 = new Block("print(5); state = check();", "the_var");
  Block *b6 = new Block("print(6); state = check();", "the_var");
  Block *b7 = new Block("print(7); state = check();", "the_var");
  Block *b8 = new Block("print(8); state = check();", "the_var");
  Block *b9 = new Block("print(9); state = check();", "the_var");
  b0->AddBranchTo(b7, NULL);
  b1->AddBranchTo(b4, "state % 2 == 0");
  b1->AddBranchTo(b6, NULL);
  b2->AddBranchTo(b1, NULL);
  b3->AddBranchTo(b3, NULL);
  b4->AddBranchTo(b2, NULL);
  b5->AddBranchTo(b1, NULL);
  b6->AddBranchTo(b7, "state % 2 == 0");
  b6->AddBranchTo(b6, NULL);
  b7->AddBranchTo(b8, "state % 3 == 0");
  b7->AddBranchTo(b2, "state % 3 == 1");
  b7->AddBranchTo(b3, NULL);
  b8->AddBranchTo(b4, "state % 2 == 0");
  b8->AddBranchTo(b6, NULL);
  b9->AddBranchTo(b7, "state % 2 == 0");
  b9->AddBranchTo(b8, NULL);

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
  r.AddBlock(b9);

  r.Calculate(b0);
  printf("\n\n");
  r.Render();

  puts(buffer);

  return 1;
}
