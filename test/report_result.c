/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "report_result.h"

#if defined __EMSCRIPTEN__ && !defined EMTEST_NODE
#include <emscripten.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined __EMSCRIPTEN__ && !defined EMTEST_NODE

void _ReportResult(int result) {
  EM_ASM(reportResultToServer($0), result);
}

void _MaybeReportResult(int result) {
  EM_ASM(maybeReportResultToServer($0), result);
}

#else

static bool reported = false;

void _ReportResult(int result) {
  if (reported) {
    printf("ERROR: result already reported\n");
    exit(1);
  }
  reported = true;
  printf("RESULT: %d\n", result);
}

void _MaybeReportResult(int result) {
  if (!reported) _ReportResult(result);
}

#endif // __EMSCRIPTEN__ && !defined EMTEST_NODE

#ifdef __cplusplus
}
#endif

