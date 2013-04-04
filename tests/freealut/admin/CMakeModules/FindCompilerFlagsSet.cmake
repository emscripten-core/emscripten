IF(CMAKE_COMPILER_IS_GNUCC)
  #ADD_DEFINITIONS(-std=c99)

  IF(WARNINGS)
    SET(ADD_WFLAGS "${ADD_WFLAGS} -Wall -ansi -pedantic -W")

    IF(MORE_WARNINGS)
      SET(ADD_WFLAGS "${ADD_WFLAGS} -Waggregate-return -Wbad-function-cast -Wcast-align -Wcast-qual -Wdisabled-optimization -Wendif-labels -Winline -Wlong-long -Wmissing-declarations -Wmissing-noreturn -Wmissing-prototypes -Wnested-externs -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-compare -Wstrict-prototypes -Wwrite-strings")
    ENDIF(MORE_WARNINGS)

    # Should we use turn warnings into errors?
    IF(USE_WERROR)
      SET(ADD_WFLAGS "${ADD_WFLAGS} -Werror -pedantic-errors")
    ENDIF(USE_WERROR)
  ENDIF(WARNINGS)

  IF(OPTIMIZATION)
    SET(ADD_CFLAGS "${ADD_CFLAGS} -O2 -finline-functions -ffast-math")
  ENDIF(OPTIMIZATION)

  IF(PROFILE)
    SET(ADD_CFLAGS "${ADD_CFLAGS} -pg")
    SET(ADD_LDFLAGS "-pg")
  ELSE(PROFILE)

    IF(OPTIMIZATION)
      # -pg and -fomit-frame-pointer are incompatible
      SET(ADD_CFLAGS "${ADD_CFLAGS} -fomit-frame-pointer")
    ENDIF(OPTIMIZATION)
  ENDIF(PROFILE)
ELSE(CMAKE_COMPILER_IS_GNUCC)

  MESSAGE("Your compiler isn't fully supported yet - no flags set.")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
