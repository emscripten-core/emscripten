// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "tick.h"

// #define BENCHMARK_FOREIGN_FUNCTION

#if defined(BENCHMARK_FOREIGN_FUNCTION) && defined(__EMSCRIPTEN__)
extern "C"
{
  int foreignFunctionThatTakesThreeParameters(int a, int b, int c);
}
#else
int foreignCounter = 0;
int __attribute__((noinline)) foreignFunctionThatTakesThreeParameters(int a, int b, int c)
{
  foreignCounter += a + b + c;
  return foreignCounter;
}
#endif
typedef int (*FuncPtrType)(int, int, int);
FuncPtrType pointerToFunction = 0;

int numRunsDone = 0;
const int totalRuns = 1000;
tick_t accumulatedTicks = 0;
tick_t allTicks[totalRuns] = {};

double averageBestPercentileMsecs(double p)
{
  tick_t acc = 0;
  int numSamples = (int)(totalRuns*p);
  for(int i = 0; i < numSamples; ++i)
    acc += allTicks[i];
  return acc * 1000.0 / numSamples / ticks_per_sec(); 
}

double averageWorstPercentileMsecs(double p)
{
  tick_t acc = 0;
  int numSamples = (int)(totalRuns*p);
  for(int i = 0; i < numSamples; ++i)
    acc += allTicks[totalRuns-1-i];
  return acc * 1000.0 / numSamples / ticks_per_sec(); 
}

int counter = 0;
void __attribute__((noinline)) main_loop()
{
  tick_t t0 = tick();
  for(int i = 0; i < 500000; ++i)
  {
#if BENCHMARK_FUNCTION_POINTER
    counter += pointerToFunction(i, i+1, i+2);
#else
    counter += foreignFunctionThatTakesThreeParameters(i, i+1, i+2);
#endif
  }
  tick_t t1 = tick();
  allTicks[numRunsDone] = t1 - t0;
  ++numRunsDone;
  accumulatedTicks += t1 - t0;
  printf("Run %d: %f msecs.\n", numRunsDone, (t1 - t0) * 1000.0 / ticks_per_sec());
  if (numRunsDone >= totalRuns)
  {
    double accumulatedTimeMsecs = accumulatedTicks * 1000.0 / ticks_per_sec();
    std::sort(allTicks, allTicks + totalRuns);
    printf("Total: %d runs, avg. %f msecs per run. Counter: %d.\n", totalRuns, accumulatedTimeMsecs / totalRuns, counter);
    printf("Best run: %f msecs.\n", allTicks[0] * 1000.0 / ticks_per_sec());
    printf("1%% Best percentile: %f msecs. 1%% Worst percentile: %f msecs.\n", averageBestPercentileMsecs(0.01), averageWorstPercentileMsecs(0.01));
    printf("5%% Best percentile: %f msecs. 5%% Worst percentile: %f msecs.\n", averageBestPercentileMsecs(0.05), averageWorstPercentileMsecs(0.05));
    printf("10%% Best percentile: %f msecs. 10%% Worst percentile: %f msecs.\n", averageBestPercentileMsecs(0.10), averageWorstPercentileMsecs(0.10));
    printf("25%% Best percentile: %f msecs. 25%% Worst percentile: %f msecs.\n", averageBestPercentileMsecs(0.25), averageWorstPercentileMsecs(0.25));
    printf("50%% Best percentile: %f msecs. 50%% Worst percentile: %f msecs.\n", averageBestPercentileMsecs(0.50), averageWorstPercentileMsecs(0.50));
    printf("75%% Best percentile: %f msecs. 75%% Worst percentile: %f msecs.\n", averageBestPercentileMsecs(0.75), averageWorstPercentileMsecs(0.75));
    printf("95%% Best percentile: %f msecs. 95%% Worst percentile: %f msecs.\n", averageBestPercentileMsecs(0.95), averageWorstPercentileMsecs(0.95));
    printf("100%% Best/Worst percentile (==average over all samples): %f msecs.\n", averageBestPercentileMsecs(1.00));

    printf("Median run: %f msecs.\n", allTicks[totalRuns/2] * 1000.0 / ticks_per_sec());
    printf("Total time: %f\n", accumulatedTimeMsecs);
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
    exit(0);
  }
}

int main()
{
  // Insist dynamic initialization that the compiler can't possibly optimize away.
  pointerToFunction = (tick() == 0 && tick() == 1000000) ? 0 : &foreignFunctionThatTakesThreeParameters;

#if defined(__EMSCRIPTEN__) && !defined(BUILD_FOR_SHELL)
  emscripten_set_main_loop(main_loop, 0, 0);
#else
  for(int i = 0; i < totalRuns; ++i)
    main_loop();
#endif
}
