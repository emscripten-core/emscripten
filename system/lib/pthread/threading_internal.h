/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

typedef union em_variant_val
{
  int i;
  int64_t i64;
  float f;
  double d;
  void *vp;
  char *cp;
} em_variant_val;

// Proxied C/C++ functions support at most this many arguments. Dispatch is
// static/strongly typed by signature.
#define EM_QUEUED_CALL_MAX_ARGS 11
typedef struct em_queued_call
{
  int functionEnum;
  void *functionPtr;
  _Atomic uint32_t operationDone;
  em_variant_val args[EM_QUEUED_JS_CALL_MAX_ARGS];
  em_variant_val returnValue;

  // An optional pointer to a secondary data block that should be free()d when
  // this queued call is freed.
  void *satelliteData;

  // If true, the caller has "detached" itself from this call object and the
  // Emscripten main runtime thread should free up this em_queued_call object
  // after it has been executed. If false, the caller is in control of the
  // memory.
  int calleeDelete;
} em_queued_call;

typedef void (*em_func_v)(void);
typedef void (*em_func_vi)(int);
typedef void (*em_func_vf)(float);
typedef void (*em_func_vii)(int, int);
typedef void (*em_func_vif)(int, float);
typedef void (*em_func_vff)(float, float);
typedef void (*em_func_viii)(int, int, int);
typedef void (*em_func_viif)(int, int, float);
typedef void (*em_func_viff)(int, float, float);
typedef void (*em_func_vfff)(float, float, float);
typedef void (*em_func_viiii)(int, int, int, int);
typedef void (*em_func_viifi)(int, int, float, int);
typedef void (*em_func_vifff)(int, float, float, float);
typedef void (*em_func_vffff)(float, float, float, float);
typedef void (*em_func_viiiii)(int, int, int, int, int);
typedef void (*em_func_viffff)(int, float, float, float, float);
typedef void (*em_func_viiiiii)(int, int, int, int, int, int);
typedef void (*em_func_viiiiiii)(int, int, int, int, int, int, int);
typedef void (*em_func_viiiiiiii)(int, int, int, int, int, int, int, int);
typedef void (*em_func_viiiiiiiii)(int, int, int, int, int, int, int, int, int);
typedef void (*em_func_viiiiiiiiii)(int, int, int, int, int, int, int, int, int, int);
typedef void (*em_func_viiiiiiiiiii)(int, int, int, int, int, int, int, int, int, int, int);
typedef int (*em_func_i)(void);
typedef int (*em_func_ii)(int);
typedef int (*em_func_iii)(int, int);
typedef int (*em_func_iiii)(int, int, int);
typedef int (*em_func_iiiii)(int, int, int, int);
typedef int (*em_func_iiiiii)(int, int, int, int, int);
typedef int (*em_func_iiiiiii)(int, int, int, int, int, int);
typedef int (*em_func_iiiiiiii)(int, int, int, int, int, int, int);
typedef int (*em_func_iiiiiiiii)(int, int, int, int, int, int, int, int);
typedef int (*em_func_iiiiiiiiii)(int, int, int, int, int, int, int, int, int);
