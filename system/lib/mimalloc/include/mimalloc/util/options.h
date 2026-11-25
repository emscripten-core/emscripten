/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_UTIL_OPTIONS_H
#define MI_UTIL_OPTIONS_H

#include "../types.h"

void  _mi_fputs(mi_output_fun* out, void* arg, const char* prefix, const char* message);
void  _mi_fprintf(mi_output_fun* out, void* arg, const char* fmt, ...);
void  _mi_raw_message(const char* fmt, ...);
void  _mi_warning_message(const char* fmt, ...);
void  _mi_verbose_message(const char* fmt, ...);
void  _mi_trace_message(const char* fmt, ...);
void  _mi_options_init(void);
void  _mi_options_post_init(void);
long  _mi_option_get_fast(mi_option_t option);
void  _mi_error_message(int err, const char* fmt, ...);
mi_decl_noreturn mi_decl_cold void _mi_fatal_error(const char* fmt, ...);  // always shown, then aborts

mi_block_t* _mi_error_block_next_is_corrupted(const mi_page_t* page, const mi_block_t* block, mi_encoded_t x);
void  _mi_error_block_next_is_invalid(const mi_page_t* page, const mi_block_t* block, const mi_block_t* next);
mi_decl_noreturn void _mi_error_list_corrupted(const void* node);
void* _mi_error_ptr_auth_failed(mi_ptr_signed_t x, mi_addr_t salt);               // slow path: returns NULL for x==0, and reports EFAULT otherwise
void  _mi_error_ptr_pac_unaligned(const void* p);                                 // fatal error for an unaligned pointer with memory tagging

/* -----------------------------------------------------------
  Options (exposed for the debugger)
----------------------------------------------------------- */
typedef enum mi_option_init_e {
  MI_OPTION_UNINIT,       // not yet initialized (and the value is not yet set to the default)
  MI_OPTION_PENDING,      // the value is set (to the default) but not yet read from the environment
  MI_OPTION_DEFAULTED,    // not found in the environment, use default value
  MI_OPTION_INITIALIZED   // found in environment or set explicitly
} mi_option_init_t;

// Static (read-only) option description
typedef struct mi_option_desc_s {
  long              default_value; // the default value
  mi_option_t       option; // for debugging: the option index should match the option
  const char*       name;   // option name without `mimalloc_` prefix
  const char*       legacy_name; // potential legacy option name
} mi_option_desc_t;

// Current option value (stored in `_mi_process.options`); atomic as options can be set concurrently with readers.
typedef struct mi_option_state_s {
  _Atomic(mi_ssize_t) value;  // the value (a `long`)
  _Atomic(size_t)     init;   // the `mi_option_init_t` state: is it initialized yet? (from the environment)
} mi_option_state_t;


#endif
