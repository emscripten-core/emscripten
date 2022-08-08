/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Define REPORT_RESULT and REPORT_RESULT_SYNC for using in test code
 */

#ifndef REPORT_RESULT_H_
#define REPORT_RESULT_H_

#ifndef __ASSEMBLER__ // Emit this file only to C/C++ language headers and not when preprocessing .S files

#ifdef __cplusplus
extern "C" {
#endif

void _ReportResult(int result, int sync);
void _MaybeReportResult(int result, int sync);

#ifdef __cplusplus
}
#endif

#if defined __EMSCRIPTEN__ && defined __EMSCRIPTEN_PTHREADS__ && !defined(__EMSCRIPTEN_WASM_WORKERS__)
  #include <emscripten.h>
  #include <emscripten/threading.h>
  #define REPORT_RESULT(result) emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VII, _ReportResult, (result), 0)
  #define REPORT_RESULT_SYNC(result) emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VII, _ReportResult, (result), 1)
  #define MAYBE_REPORT_RESULT(result) emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VII, _MaybeReportResult, (result), 0)
  #define MAYBE_REPORT_RESULT_SYNC(result) emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VII, _MaybeReportResult, (result), 1)
#else
  #define REPORT_RESULT(result) _ReportResult((result), 0)
  #define REPORT_RESULT_SYNC(result) _ReportResult((result), 1)
  #define MAYBE_REPORT_RESULT(result) _MaybeReportResult((result), 0)
  #define MAYBE_REPORT_RESULT_SYNC(result) _MaybeReportResult((result), 1)
#endif

#endif

#endif // REPORT_RESULT_H_
