#define VAR_DEFINED

#define VAR_42 42

#define VAR_A 0
#define VAR_B 0
#define VAR_C 1
#define VAR_D 1

var LibraryPreprocess = {
#ifdef VAR_DEFINED 
  foo: function() { return 1; },
#endif
#ifndef VAR_NOT_DEFINED
  foo2: function() { return 1; },
#endif
#if VAR_DEFINED
  foo3: function() { return 1; },
#endif
#if VAR_42 == 42
  foo4: function() { return 1; },
#endif
#if VAR_42
  foo5: function() { return 1; },
#endif
#if !(VAR_42 == 43)
  foo6: function() { return 1; },
#endif
#if VAR_NOT_DEFINED
compile error
#endif
#ifdef VAR_NOT_DEFINED
compile error
#endif
#ifndef VAR_DEFINED
compile error
#endif
#if VAR_42 > 42 || VAR_42 < 42
compile error
#endif
#if VAR_A || VAR_B
compile error
#endif
#if VAR_A && VAR_B
compile error
#endif
#if !VAR_A
  foo7: function() { return 1; },
#endif
#if !!VAR_A
compile error
#endif
#if !VAR_C || !VAR_D
compile error
#endif
#if !VAR_C && !VAR_D
compile error
#endif
#if VAR_C && !VAR_A
  foo8: function() { return 1; },
#endif
#if !(VAR_C && !VAR_A)
compile error
#endif
#if defined(VAR_NOT_DEFINED)
compile error
#endif
#if !defined(VAR_DEFINED)
compile error
#endif
#if defined(VAR_NOT_DEFINED) || defined(VAR_A)
  foo9: function() { return 1; },
#endif
#ifdef VAR_DEFINED
#elif defined(VAR_D)
#else
compile_error
#endif
#ifdef VAR_NOT_DEFINED
#elif defined(VAR_E)
#else
  foo10: function() { return 1; },
#endif

  dummy: 0
};

#warning This is a test warning message.

mergeInto(LibraryManager.library, LibraryPreprocess);
