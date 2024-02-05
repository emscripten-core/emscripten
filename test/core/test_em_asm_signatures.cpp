#include <emscripten.h>
#include <stdio.h>

int main()
{
  int ret = MAIN_THREAD_EM_ASM_INT(return 1);
  ret += MAIN_THREAD_EM_ASM_INT(return $0, 1);
  ret += MAIN_THREAD_EM_ASM_INT(return $0 + $1, 1, 2);
  ret += MAIN_THREAD_EM_ASM_INT(return $0 + $1 + $2, 1, 2, 3);
  ret += MAIN_THREAD_EM_ASM_INT(return $0 + $1 + $2 + $3, 1, 2, 3, 4);
  ret += MAIN_THREAD_EM_ASM_INT(return $0 + $1 + $2 + $3 + $4, 1, 2, 3, 4, 5);
  ret += MAIN_THREAD_EM_ASM_INT(return $0 + $1 + $2 + $3 + $4 + $5, 1, 2, 3, 4, 5, 6);
  ret += MAIN_THREAD_EM_ASM_INT(return $0 + $1 + $2 + $3 + $4 + $5 + $6, 1, 2, 3, 4, 5, 6, 7);
  ret += MAIN_THREAD_EM_ASM_INT(return $0 + $1 + $2 + $3 + $4 + $5 + $6 + $7, 1, 2, 3, 4, 5, 6, 7, 8);
  printf("ret: %d\n", ret);
  return ret;
}
