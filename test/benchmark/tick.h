#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#ifdef WIN32
#include <Windows.h>
#define tick_t unsigned long long
#define aligned_alloc(align, size) _aligned_malloc((size), (align))
#endif

#ifdef __EMSCRIPTEN__
#define tick emscripten_get_now
#define tick_t double
tick_t ticks_per_sec() { return 1000.0; }
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#define tick_t unsigned long long
#define tick mach_absolute_time
tick_t ticks_per_sec()
{
  mach_timebase_info_data_t timeBaseInfo;
  mach_timebase_info(&timeBaseInfo);
  return 1000000000ULL * (uint64_t)timeBaseInfo.denom / (uint64_t)timeBaseInfo.numer;
}
#elif defined(_POSIX_MONOTONIC_CLOCK)
#define tick_t unsigned long long
inline tick_t tick()
{
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (tick_t)t.tv_sec * 1000 * 1000 * 1000 + (tick_t)t.tv_nsec;
}
tick_t ticks_per_sec()
{
  return 1000 * 1000 * 1000;
}
#elif defined(_POSIX_C_SOURCE)
#include <sys/time.h>
#define tick_t unsigned long long
inline tick_t tick()
{
  timeval t;
  gettimeofday(&t, NULL);
  return (tick_t)t.tv_sec * 1000 * 1000 + (tick_t)t.tv_usec;
}
tick_t ticks_per_sec()
{
  return 1000 * 1000;
}
#elif defined(WIN32)
#define tick_t unsigned long long
inline tick_t tick()
{
  LARGE_INTEGER ddwTimer;
  QueryPerformanceCounter(&ddwTimer);
  return ddwTimer.QuadPart;
}
tick_t ticks_per_sec()
{
  LARGE_INTEGER ddwTimerFrequency;
  QueryPerformanceFrequency(&ddwTimerFrequency);
  return ddwTimerFrequency.QuadPart;
}
#else
#error No tick_t
#endif
