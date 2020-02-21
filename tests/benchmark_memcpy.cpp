// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <algorithm>

#ifdef WIN32
#include <Windows.h>
#define aligned_alloc(align, size) _aligned_malloc((size), (align))
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "tick.h"

char dst[1024*1024*64+16] = {};
char src[1024*1024*64+16] = {};

uint8_t resultCheckSum = 0;

void __attribute__((noinline)) test_memcpy(int numTimes, int copySize)
{
	for(int i = 0; i < numTimes - 8; i += 8)
	{
		memcpy(dst, src, copySize); resultCheckSum += dst[copySize >> 1];
		memcpy(dst, src, copySize); resultCheckSum += dst[copySize >> 1];
		memcpy(dst, src, copySize); resultCheckSum += dst[copySize >> 1];
		memcpy(dst, src, copySize); resultCheckSum += dst[copySize >> 1];
		memcpy(dst, src, copySize); resultCheckSum += dst[copySize >> 1];
		memcpy(dst, src, copySize); resultCheckSum += dst[copySize >> 1];
		memcpy(dst, src, copySize); resultCheckSum += dst[copySize >> 1];
		memcpy(dst, src, copySize); resultCheckSum += dst[copySize >> 1];
	}
	numTimes &= 15;
	for(int i = 0; i < numTimes; ++i)
	{
		memcpy(dst, src, copySize); resultCheckSum += dst[copySize >> 1];
	}
}

std::vector<int> copySizes;
std::vector<double> results;

std::vector<int> testCases;

double totalTimeSecs = 0.0;

void test_case(int copySize)
{
	const int minimumCopyBytes = 1024*1024*64;

	int numTimes = (minimumCopyBytes + copySize-1) / copySize;
	if (numTimes < 8) numTimes = 8;

	tick_t bestResult = 1e9;

#ifndef NUM_TRIALS
#define NUM_TRIALS 5
#endif

	for(int i = 0; i < NUM_TRIALS; ++i)
	{
		double t0 = tick();
		test_memcpy(numTimes, copySize);
		double t1 = tick();
		if (t1 - t0 < bestResult) bestResult = t1 - t0;
		totalTimeSecs += (double)(t1 - t0) / ticks_per_sec();
	}
	unsigned long long totalBytesTransferred = numTimes * copySize;

	copySizes.push_back(copySize);

	tick_t ticksElapsed = bestResult;
	if (ticksElapsed > 0)
	{
		double seconds = (double)ticksElapsed / ticks_per_sec();
		double bytesPerSecond = totalBytesTransferred / seconds;
		double mbytesPerSecond = bytesPerSecond / (1024.0*1024.0);
		results.push_back(mbytesPerSecond);
	}
	else
	{
		results.push_back(0.0);
	}
}

void print_results()
{
	std::cout << "Test cases: " << std::endl;
	for(size_t i = 0; i < copySizes.size(); ++i)
	{
		std::cout << copySizes[i];
		if (i != copySizes.size()-1) std::cout << ", ";
		else std::cout << std::endl;
		if (i % 10 == 9) std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Test results: " << std::endl;
	for(size_t i = 0; i < results.size(); ++i)
	{
		std::cout << results[i];
		if (i != results.size()-1) std::cout << ", ";
		else std::cout << std::endl;
		if (i % 10 == 9) std::cout << std::endl;
	}

	std::cout << "Result checksum: " << (int)resultCheckSum << std::endl;
	std::cout << "Total time: " << totalTimeSecs << std::endl;
}

int numDone = 0;

void run_one()
{
	std::cout << (numDone+1) << "/" << (numDone+testCases.size()) << std::endl;
	++numDone;

	int copySize = testCases.front();
	testCases.erase(testCases.begin());
	test_case(copySize);
}

#ifdef __EMSCRIPTEN__
void main_loop()
{
	if (!testCases.empty())
	{
		run_one();
	}
	else
	{
		emscripten_cancel_main_loop();
		print_results();
	}
}
#endif

#ifndef MAX_COPY
#define MAX_COPY 32*1024*1024
#endif

#ifndef MIN_COPY
#define MIN_COPY 1
#endif

int main()
{
	for(int copySizeI = MIN_COPY; copySizeI < MAX_COPY; copySizeI <<= 1)
		for(int copySizeJ = 1; copySizeJ <= copySizeI; copySizeJ <<= 1)
		{
			testCases.push_back(copySizeI | copySizeJ);
		}

	std::sort(testCases.begin(), testCases.end());
#if defined(__EMSCRIPTEN__) && !defined(BUILD_FOR_SHELL)
	emscripten_set_main_loop(main_loop, 0, 0);
#else
	while(!testCases.empty()) run_one();
	print_results();
#endif
}
