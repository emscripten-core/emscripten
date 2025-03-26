/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include "em_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// wget

void emscripten_async_wget(const char* url, const char* file, em_str_callback_func onload, em_str_callback_func onerror);

typedef void (*em_async_wget_onload_func)(void* userdata, void* data, int size);
void emscripten_async_wget_data(const char* url, void *userdata, em_async_wget_onload_func onload, em_arg_callback_func onerror);

typedef void (*em_async_wget2_onload_func)(unsigned handle, void* userdata, const char* data);
typedef void (*em_async_wget2_onstatus_func)(unsigned handle, void* userdata, int status);

int emscripten_async_wget2(const char* url, const char* file,  const char* requesttype, const char* param, void *userdata, em_async_wget2_onload_func onload, em_async_wget2_onstatus_func onerror, em_async_wget2_onstatus_func onprogress);

typedef void (*em_async_wget2_data_onload_func)(unsigned handle, void* userdata, void* data, unsigned size);
typedef void (*em_async_wget2_data_onerror_func)(unsigned handle, void* userdata, int status, const char* status_text);
typedef void (*em_async_wget2_data_onprogress_func)(unsigned handle, void* userdata, int loaded, int total);

int emscripten_async_wget2_data(const char* url, const char* requesttype, const char* param, void *arg, int free, em_async_wget2_data_onload_func onload, em_async_wget2_data_onerror_func onerror, em_async_wget2_data_onprogress_func onprogress);

void emscripten_async_wget2_abort(int handle);

// wget "sync"

int emscripten_wget(const char* url, const char* file);

void emscripten_wget_data(const char* url, void** pbuffer, int* pnum, int *perror);

#ifdef __cplusplus
}
#endif
