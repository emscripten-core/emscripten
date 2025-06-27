//========================================================================
//
// GooLikely.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2008 Kees Cook <kees@outflux.net>
//
//========================================================================

#ifndef GOOLIKELY_H
#define GOOLIKELY_H

// XXX EMSCRIPTEN: Remove gcc-specific detection of __builtin_expect.
// #if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#ifdef __EMSCRIPTEN__
# define likely(x)      __builtin_expect((x), 1)
# define unlikely(x)    __builtin_expect((x), 0)
#else
# define likely(x)      (x)
# define unlikely(x)    (x)
#endif

#endif
