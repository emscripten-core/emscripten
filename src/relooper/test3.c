
#include "Relooper.h"

int main() {
  char buffer[10000];
  rl_set_output_buffer(buffer, sizeof(buffer));

  void *r = rl_new_relooper();
  void *ep = rl_new_block("ep", "the_var");
  rl_relooper_add_block(r, ep);
  void *LBB1 = rl_new_block("LBB1", "the_var");
  rl_relooper_add_block(r, LBB1);
  void *LBB2 = rl_new_block("LBB2", "the_var");
  rl_relooper_add_block(r, LBB2);
  void *LBB3 = rl_new_block("LBB3", "the_var");
  rl_relooper_add_block(r, LBB3);
  void *LBB4 = rl_new_block("LBB4", "the_var");
  rl_relooper_add_block(r, LBB4);
  void *LBB5 = rl_new_block("LBB5", "the_var");
  rl_relooper_add_block(r, LBB5);
  void *LBB6 = rl_new_block("LBB6", "the_var");
  rl_relooper_add_block(r, LBB6);

  rl_block_add_branch_to(ep, LBB1, "ep -> LBB1", NULL);
  rl_block_add_branch_to(ep, LBB3, NULL, NULL);
  rl_block_add_branch_to(LBB1, LBB2, "LBB1 -> LBB2", NULL);
  rl_block_add_branch_to(LBB1, LBB3, NULL, NULL);
  rl_block_add_branch_to(LBB2, LBB3, NULL, NULL);
  rl_block_add_branch_to(LBB3, LBB4, "LBB3 -> LBB4", NULL);
  rl_block_add_branch_to(LBB3, LBB6, NULL, NULL);
  rl_block_add_branch_to(LBB4, LBB5, "LBB4 -> LBB5", NULL);
  rl_block_add_branch_to(LBB4, LBB6, NULL, NULL);
  rl_block_add_branch_to(LBB5, LBB6, "LBB5 -> LBB6", NULL);
  rl_block_add_branch_to(LBB5, LBB5, NULL, NULL);

  rl_relooper_calculate(r, ep);
  rl_relooper_render(r);
  rl_delete_relooper(r);

  puts(buffer);
}

