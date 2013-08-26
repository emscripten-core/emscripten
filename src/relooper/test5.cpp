
#include "Relooper.h"

int main() {
  char buffer[10000];
  rl_set_output_buffer(buffer, sizeof(buffer));

  void *r = rl_new_relooper();

  void *b0 = rl_new_block("//0", "the_var");
  rl_relooper_add_block(r, b0);
  void *b1 = rl_new_block("//1", "the_var");
  rl_relooper_add_block(r, b1);
  void *b2 = rl_new_block("//2", "the_var");
  rl_relooper_add_block(r, b2);
  void *b3 = rl_new_block("//3", "the_var");
  rl_relooper_add_block(r, b3);
  void *b4 = rl_new_block("//4", "the_var");
  rl_relooper_add_block(r, b4);
  void *b5 = rl_new_block("//5", "the_var");
  rl_relooper_add_block(r, b5);

  rl_block_add_branch_to(b0, b1, "check(0)", NULL);
  rl_block_add_branch_to(b0, b4, NULL, "goingFrom0to4();");
  rl_block_add_branch_to(b1, b1, "check(1)", NULL);
  rl_block_add_branch_to(b1, b2, NULL, NULL);
  rl_block_add_branch_to(b2, b2, "check(2)", NULL);
  rl_block_add_branch_to(b2, b3, NULL, NULL);
  rl_block_add_branch_to(b4, b4, "check(4)", NULL);
  rl_block_add_branch_to(b4, b5, NULL, NULL);
  rl_block_add_branch_to(b5, b3, "check(5)", NULL);
  rl_block_add_branch_to(b5, b5, NULL, NULL);

  rl_relooper_calculate(r, b0);
  rl_relooper_render(r);
  rl_delete_relooper(r);

  puts(buffer);
}

