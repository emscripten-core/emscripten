# - MACRO_OPTIONAL_FIND_PACKAGE() combines FIND_PACKAGE() with an OPTION()
# MACRO_OPTIONAL_FIND_PACKAGE( <name> [QUIT] )
# This macro is a combination of OPTION() and FIND_PACKAGE(), it
# works like FIND_PACKAGE(), but additionally it automatically creates
# an option name WITH_<name>, which can be disabled via the cmake GUI.
# or via -DWITH_<name>=OFF
# The standard <name>_FOUND variables can be used in the same way
# as when using the normal FIND_PACKAGE()

# Copyright (c) 2006-2010 Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# This is just a helper macro to set a bunch of variables empty.
# We don't know whether the package uses UPPERCASENAME or CamelCaseName, so we try both:
macro(_MOFP_SET_EMPTY_IF_DEFINED _name _var)
   if(DEFINED ${_name}_${_var})
      set(${_name}_${_var} "")
   endif(DEFINED ${_name}_${_var})

   string(TOUPPER ${_name} _nameUpper)
   if(DEFINED ${_nameUpper}_${_var})
      set(${_nameUpper}_${_var}  "")
   endif(DEFINED ${_nameUpper}_${_var})
endmacro(_MOFP_SET_EMPTY_IF_DEFINED _package _var)


macro (MACRO_OPTIONAL_FIND_PACKAGE _name )
   option(WITH_${_name} "Search for ${_name} package" ON)
   if (WITH_${_name})
      find_package(${_name} ${ARGN})
   else (WITH_${_name})
      string(TOUPPER ${_name} _nameUpper)
      set(${_name}_FOUND FALSE)
      set(${_nameUpper}_FOUND FALSE)

      _mofp_set_empty_if_defined(${_name} INCLUDE_DIRS)
      _mofp_set_empty_if_defined(${_name} INCLUDE_DIR)
      _mofp_set_empty_if_defined(${_name} INCLUDES)
      _mofp_set_empty_if_defined(${_name} LIBRARY)
      _mofp_set_empty_if_defined(${_name} LIBRARIES)
      _mofp_set_empty_if_defined(${_name} LIBS)
      _mofp_set_empty_if_defined(${_name} FLAGS)
      _mofp_set_empty_if_defined(${_name} DEFINITIONS)
   endif (WITH_${_name})
endmacro (MACRO_OPTIONAL_FIND_PACKAGE)

