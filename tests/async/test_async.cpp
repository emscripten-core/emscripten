
#include <emscripten.h>
#include <emscripten/threading.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>

// std::launch enum can be specified
#ifndef STD_LAUNCH
#define STD_LAUNCH std::launch::async
#endif  // STD_LAUNCH 

volatile int result = 0;

template <typename _Iter>
int parallel_sum(_Iter begin, _Iter end)
{
  EM_ASM(Module['print']('entering parallel_sum'););

  auto len = end - begin;
  if (len < 1000)
      return std::accumulate(begin, end, 0);

  _Iter mid = begin + len/2;
  auto handle = std::async(STD_LAUNCH,
                           parallel_sum<_Iter>, mid, end);
  int sum = parallel_sum(begin, mid);
  return sum + handle.get();
}

int main()
{
  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    result = 1;
    REPORT_RESULT();
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  std::vector<int> v(10000, 1);
  result = parallel_sum(v.begin(), v.end()) != v.size();

#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
}
