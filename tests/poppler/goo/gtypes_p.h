/*
 * gtypes_p.h
 *
 * Some useful simple types.
 *
 * Copyright (C) 2011 Adrian Johnson <ajohnson@redneon.com>
 */

#ifndef GTYPES_P_H
#define GTYPES_P_H

#include "config.h"

/*
 * Define precise integer types.
 */
#if HAVE_STDINT_H
#include <stdint.h>
#elif _MSC_VER
typedef signed   __int8  int8_t;
typedef unsigned __int8  uint8_t;
typedef signed   __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed   __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed   __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

#endif
