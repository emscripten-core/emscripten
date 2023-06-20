// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include "tick.h"

// Naively computes dst = a * b, where a is a matrix of size YxI, and b is a matrix of size IxX. The output matrix dst will be of size YxX.
void mul(float *dst, float *a, float *b, int Y, int I, int X)
{
  for(int y = 0; y < Y; ++y)
  {
    for(int x = 0; x < X; ++x)
    {
      float *A = a;
      float *AEnd = a + I;
      float *B = b + x;
      float acc = 0.f;
      while(A < AEnd)
      {
        acc += *A++ * *B;
        B += X;
      }
      *dst++ = acc;
    }
    a += I;
  }
}

float Sum(float *m, int A, int B)
{
  float acc = 0.f;
  for(int i = 0; i < A*B; ++i)
    acc += m[i];
  return acc;
}

void Init(float *m, int A, int B)
{
  for(int i = 0; i < A*B; ++i)
    m[i] = (float)i;
}

float sums = 0.0f;

void Benchmark(int Num, float *mDst, float *mA, float *mB, int A, int B, int C)
{
  for(int i = 0; i < Num; ++i)
  {
    mul(mDst, mA, mB, A, B, C);
    sums += Sum(mDst, A, C);
  }
}

int ENVIRONMENT_IS_WEB;
int A;
int B;
int C;
int NumFrames;
int NumFramesDone = 0;
int ItersPerFrame;
int Method;
float *mA;
float *mB;
float *mDst;
tick_t t0;

void Done()
{
  tick_t t1 = tick();
  printf("Ran %d matrix multiplications (across %d frames). Total elapsed: %f seconds. Checksum: %f.\n", NumFrames*ItersPerFrame, NumFrames, (double)(t1-t0)/ticks_per_sec(), sums);
}

void Tick()
{
  Benchmark(ItersPerFrame, mDst, mA, mB, A, B, C);
  if (++NumFramesDone >= NumFrames)
  {
    Done();
  }
}

int main(int argc, char **argv)
{
  if (argc < 7)
  {
    printf("Usage: matrix_multiply A B C NumFrames ItersPerFrame Method\n");
    printf("Method is one of: 0=setTimeout, 1=requestAnimationFrame, 2=setImmediate\n");
    A = 100;
    B = 20;
    C = 100;
    NumFrames = 10;
    ItersPerFrame = 1000;
    Method = 2;
    printf("Not enough parameters specified. Running with defaults: A=%d B=%d C=%d NumFrames=%d ItersPerFrame=%d Method=%d.\n", A, B, C, NumFrames, ItersPerFrame, Method);
  }
  else
  {
    A = atoi(argv[1]);
    B = atoi(argv[2]);
    C = atoi(argv[3]);
    NumFrames = atoi(argv[4]);
    ItersPerFrame = atoi(argv[5]);
    Method = atoi(argv[6]);
  }
  printf("Performing %d multiplications of matrices of size %dx%d and %dx%d. Distributing multiplication across %d animation frames (matrix muls per frame=%d).\n", NumFrames*ItersPerFrame, A, B, B, C, NumFrames, ItersPerFrame);

  mA = new float[A*B];
  mB = new float[B*C];
  mDst = new float[A*C];
  Init(mA, A, B);
  Init(mB, B, C);

  t0 = tick();

  if (!ENVIRONMENT_IS_WEB)
  {
    for(int i = 0; i < NumFrames; ++i)
      Tick();
  }
  if (NumFramesDone != NumFrames) {
    puts("Invalid number of frames");
    abort();
  }

  return 0;
}
