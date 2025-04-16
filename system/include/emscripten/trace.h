/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __EMSCRIPTEN_TRACING__

void emscripten_trace_configure(const char *collector_url, const char *application);

void emscripten_trace_configure_for_google_wtf(void);

void emscripten_trace_configure_for_test(void);

void emscripten_trace_set_enabled(bool enabled);

void emscripten_trace_set_session_username(const char *username);

void emscripten_trace_record_frame_start(void);

void emscripten_trace_record_frame_end(void);

void emscripten_trace_mark(const char *message);

void emscripten_trace_log_message(const char *channel, const char *message);

void emscripten_trace_report_error(const char *error);

void emscripten_trace_record_allocation(const void *address, int32_t size);

void emscripten_trace_record_reallocation(const void *old_address, const void *new_address, int32_t size);

void emscripten_trace_record_free(const void *address);

void emscripten_trace_annotate_address_type(const void *address, const char *type);

void emscripten_trace_associate_storage_size(const void *address, int32_t size);

void emscripten_trace_report_memory_layout(void);

void emscripten_trace_report_off_heap_data(void);

void emscripten_trace_enter_context(const char *name);

void emscripten_trace_exit_context(void);

void emscripten_trace_task_start(int task_id, const char *name);

void emscripten_trace_task_associate_data(const char *key, const char *value);

void emscripten_trace_task_suspend(const char *explanation);

void emscripten_trace_task_resume(int task_id, const char *explanation);

void emscripten_trace_task_end(void);

void emscripten_trace_close(void);

#else

#define emscripten_trace_configure(collector_url, application) ((void)0)
#define emscripten_trace_configure_for_google_wtf() ((void)0)
#define emscripten_trace_configure_for_test() ((void)0)
#define emscripten_trace_set_enabled(enabled) ((void)0)
#define emscripten_trace_set_session_username(username) ((void)0)
#define emscripten_trace_record_frame_start() ((void)0)
#define emscripten_trace_record_frame_end() ((void)0)
#define emscripten_trace_mark(message) ((void)0)
#define emscripten_trace_log_message(channel, message) ((void)0)
#define emscripten_trace_report_error(error) ((void)0)
#define emscripten_trace_record_allocation(address, size) ((void)0)
#define emscripten_trace_record_reallocation(old_address, new_address, size) ((void)0)
#define emscripten_trace_record_free(address) ((void)0)
#define emscripten_trace_annotate_address_type(address, type) ((void)0)
#define emscripten_trace_associate_storage_size(address, size) ((void)0)
#define emscripten_trace_report_memory_layout() ((void)0)
#define emscripten_trace_report_off_heap_data() ((void)0)
#define emscripten_trace_enter_context(name) ((void)0)
#define emscripten_trace_exit_context() ((void)0)
#define emscripten_trace_task_start(task_id, taskname) ((void)0)
#define emscripten_trace_task_associate_data(key, value) ((void)0)
#define emscripten_trace_task_suspend(explanation) ((void)0)
#define emscripten_trace_task_resume(task_id, explanation) ((void)0)
#define emscripten_trace_task_end() ((void)0)
#define emscripten_trace_close() ((void)0)

#endif

#ifdef __cplusplus
} // ~extern "C"
#endif
