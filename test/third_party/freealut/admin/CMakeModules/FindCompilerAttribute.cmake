SET(VAR HAVE_ATTRIBUTE)

CHECK_C_SOURCE_COMPILES(
"void  foo (int bar __attribute__((unused)) ) { }
static void baz (void) __attribute__((unused));
static void baz (void) { }
int main(){}
" ${VAR})
IF(${VAR})
  ADD_DEFINE("HAVE___ATTRIBUTE__ 1")
ENDIF(${VAR})
