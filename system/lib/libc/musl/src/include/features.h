#ifndef FEATURES_H
#define FEATURES_H

#include "../../include/features.h"

#define weak __attribute__((__weak__))
#define hidden __attribute__((__visibility__("hidden")))
#ifdef __EMSCRIPTEN__
// Also define an emscripten_builtin_* alias for sanitizer interceptors (see interception.h).
#define weak_alias(old, new) \
	extern __typeof(old) new __attribute__((__weak__, __alias__(#old))); \
	extern hidden __typeof(old) emscripten_builtin_##new __attribute__((__weak__, __alias__(#old)))
#else
#define weak_alias(old, new) \
	extern __typeof(old) new __attribute__((__weak__, __alias__(#old)))
#endif

#endif
