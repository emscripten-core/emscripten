
#include "Relooper.h"

int main() {
  char buffer[10000];

  if (1) {
    Relooper::SetOutputBuffer(buffer, sizeof(buffer));

    printf("\n\n-- If pattern --\n\n");

    Block *b_a = new Block("// block A\n");
    Block *b_b = new Block("// block B\n");
    Block *b_c = new Block("// block C\n");

    b_a->AddBranchTo(b_b, "check == 10", "atob();");
    b_a->AddBranchTo(b_c, NULL, "atoc();");

    b_b->AddBranchTo(b_c, NULL, "btoc();");

    Relooper r;
    r.AddBlock(b_a);
    r.AddBlock(b_b);
    r.AddBlock(b_c);

    r.Calculate(b_a);
    printf("\n\n");
    r.Render();

    puts(buffer);
  }

  if (1) {
    Relooper::SetOutputBuffer(buffer, sizeof(buffer));

    printf("\n\n-- If-else pattern --\n\n");

    Block *b_a = new Block("// block A\n");
    Block *b_b = new Block("// block B\n");
    Block *b_c = new Block("// block C\n");
    Block *b_d = new Block("// block D\n");

    b_a->AddBranchTo(b_b, "check == 15");
    b_a->AddBranchTo(b_c, NULL);

    b_b->AddBranchTo(b_d, NULL);

    b_c->AddBranchTo(b_d, NULL);

    Relooper r;
    r.AddBlock(b_a);
    r.AddBlock(b_b);
    r.AddBlock(b_c);
    r.AddBlock(b_d);

    r.Calculate(b_a);
    printf("\n\n");
    r.Render();

    puts(buffer);
  }

  if (1) {
    Relooper::SetOutputBuffer(buffer, sizeof(buffer));

    printf("\n\n-- Loop + tail pattern --\n\n");

    Block *b_a = new Block("// block A\nvar check = maybe();\n");
    Block *b_b = new Block("// block B\n");
    Block *b_c = new Block("// block C\n");

    b_a->AddBranchTo(b_b, NULL);

    b_b->AddBranchTo(b_a, "check == 41");
    b_b->AddBranchTo(b_c, NULL);

    Relooper r;
    r.AddBlock(b_a);
    r.AddBlock(b_b);
    r.AddBlock(b_c);

    r.Calculate(b_a);
    printf("\n\n");
    r.Render();

    puts(buffer);
  }

  if (1) {
    Relooper::SetOutputBuffer(buffer, sizeof(buffer));

    printf("\n\n-- Loop with phi to head \n\n");

    void *block_map[10000];
    void *rl = rl_new_relooper();
    void *b1 = rl_new_block("// code 1");
    block_map[1] = b1;
    rl_relooper_add_block(rl, block_map[1]);
    void *b2 = rl_new_block("// code 2");
    block_map[2] = b2;
    rl_relooper_add_block(rl, block_map[2]);
    void *b3 = rl_new_block("// code 3");
    block_map[3] = b3;
    rl_relooper_add_block(rl, block_map[3]);
    void *b4 = rl_new_block("// code 4");
    block_map[4] = b4;
    rl_relooper_add_block(rl, block_map[4]);
    void *b5 = rl_new_block("// code 5");
    block_map[5] = b5;
    rl_relooper_add_block(rl, block_map[5]);
    void *b6 = rl_new_block("// code 6");
    block_map[6] = b6;
    rl_relooper_add_block(rl, block_map[6]);
    void *b7 = rl_new_block("// code 7");
    block_map[7] = b7;
    rl_relooper_add_block(rl, block_map[7]);
    rl_block_add_branch_to(block_map[1], block_map[2], NULL, "var $i_0 = 0;var $x_0 = 5; ");
    rl_block_add_branch_to(block_map[2], block_map[3], "$2", NULL);
    rl_block_add_branch_to(block_map[2], block_map[7], NULL, "var $x_1 = $x_0; ");
    rl_block_add_branch_to(block_map[3], block_map[4], "$6", NULL);
    rl_block_add_branch_to(block_map[3], block_map[2], NULL, "var $i_0 = $7;var $x_0 = $5; ");
    rl_block_add_branch_to(block_map[4], block_map[5], "$10", NULL);
    rl_block_add_branch_to(block_map[4], block_map[6], NULL, NULL);
    rl_block_add_branch_to(block_map[5], block_map[6], NULL, NULL);
    rl_block_add_branch_to(block_map[6], block_map[7], NULL, "var $x_1 = $13; ");
    rl_relooper_calculate(rl, block_map[1]);
    rl_relooper_render(rl);
    rl_delete_relooper(rl);
    puts(buffer);
  }

  if (1) {
    Relooper::SetOutputBuffer(buffer, sizeof(buffer));

    printf("\n\n-- phi on split dead ends --\n\n");

    Block *b_a = new Block("// block A...................................................................................................\n");
    Block *b_b = new Block("// block B...................................................................................................\n");
    Block *b_c = new Block("// block C...................................................................................................\n");
    Block *b_d = new Block("// block D\n"); // small and splittable!
    Block *b_e = new Block("// block E\n");

    b_a->AddBranchTo(b_b, "chak()", "atob();");
    b_a->AddBranchTo(b_c, NULL, "atoc();");

    b_b->AddBranchTo(b_d, NULL, "btod();");

    b_c->AddBranchTo(b_d, NULL, "ctod2();");

    Relooper r;
    r.AddBlock(b_a);
    r.AddBlock(b_b);
    r.AddBlock(b_c);
    r.AddBlock(b_d);
    r.AddBlock(b_e);

    r.Calculate(b_a);
    printf("\n\n");
    r.Render();

    puts(buffer);
  }

  if (1) {
    Relooper::SetOutputBuffer(buffer, sizeof(buffer));

    printf("\n\n-- Unbalanced with a dead end --\n\n");

    Block *b_a = new Block("// block A\n");
    Block *b_b = new Block("// block B\n");
    Block *b_c = new Block("return C;\n");
    Block *b_d = new Block("// block D\n");

    b_a->AddBranchTo(b_b, "check == 10");
    b_a->AddBranchTo(b_c, NULL); // c is a dead end

    b_b->AddBranchTo(b_d, NULL);

    b_d->AddBranchTo(b_b, NULL);

    Relooper r;
    r.AddBlock(b_a);
    r.AddBlock(b_b);
    r.AddBlock(b_c);
    r.AddBlock(b_d);

    r.Calculate(b_a);
    printf("\n\n");
    r.Render();

    puts(buffer);
  }
}

