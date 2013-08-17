
#include "Relooper.h"

int main() {
  char buffer[10000];
  rl_set_output_buffer(buffer, sizeof(buffer));

  void *r = rl_new_relooper();

  void *b19 = rl_new_block("//19", "the_var");
  rl_relooper_add_block(r, b19);
  void *b20 = rl_new_block("//20", "the_var");
  rl_relooper_add_block(r, b20);
  void *b21 = rl_new_block("//21", "the_var");
  rl_relooper_add_block(r, b21);
  void *b22 = rl_new_block("//22", "the_var");
  rl_relooper_add_block(r, b22);
  void *b23 = rl_new_block("//23", "the_var");
  rl_relooper_add_block(r, b23);
  void *b24 = rl_new_block("//24", "the_var");
  rl_relooper_add_block(r, b24);
  void *b28 = rl_new_block("//28", "the_var");
  rl_relooper_add_block(r, b28);

  rl_block_add_branch_to(b19, b20, " 1 ", NULL);
  rl_block_add_branch_to(b19, b22, NULL, NULL);
  rl_block_add_branch_to(b20, b21, " 1 ", NULL);
  rl_block_add_branch_to(b20, b22, NULL, NULL);
  rl_block_add_branch_to(b21, b23, NULL, NULL);
  rl_block_add_branch_to(b22, b23, NULL, NULL);
  rl_block_add_branch_to(b23, b24, " 1 ", NULL);
  rl_block_add_branch_to(b23, b28, NULL, NULL);

  rl_relooper_calculate(r, b19);
  rl_relooper_render(r);
  rl_delete_relooper(r);

  puts(buffer);
}

