/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#ifndef EMTEST_PORT_NUMBER
#error "EMTEST_PORT_NUMBER not defined"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void EMSCRIPTEN_KEEPALIVE _ReportResult(int result, int sync)
{
  EM_ASM({
    reportResultToServer($0, $1, $2);
  }, result, sync, EMTEST_PORT_NUMBER);
}

void EMSCRIPTEN_KEEPALIVE _MaybeReportResult(int result, int sync)
{
  EM_ASM({
    maybeReportResultToServer($0, $1, $2);
  }, result, sync, EMTEST_PORT_NUMBER);
}

#ifdef __cplusplus
}
#endif

#endif // __EMSCRIPTEN__
