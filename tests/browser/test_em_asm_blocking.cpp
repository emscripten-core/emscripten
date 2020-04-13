#include <emscripten.h>
#include <stdio.h>
#include <thread>

std::atomic<int> ret;

void foo() {
  int len = MAIN_THREAD_EM_ASM_INT({
    var elem = document.getElementById('elem');
    return elem.innerText.length;
  });
  atomic_store(&ret, len);
}

int main() {
  std::thread t(foo);
  t.join();
  printf("ret: %d\n", atomic_load(&ret));
#ifdef REPORT_RESULT
  REPORT_RESULT(atomic_load(&ret));
#endif
  return atomic_load(&ret);
}
