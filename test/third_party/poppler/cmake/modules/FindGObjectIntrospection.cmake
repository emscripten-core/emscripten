# - try to find gobject-introspection
#
# Once done this will define
#
#  INTROSPECTION_FOUND - system has gobject-introspection
#  INTROSPECTION_SCANNER - the gobject-introspection scanner, g-ir-scanner
#  INTROSPECTION_COMPILER - the gobject-introspection compiler, g-ir-compiler
#  INTROSPECTION_GENERATE - the gobject-introspection generate, g-ir-generate
#  INTROSPECTION_GIRDIR
#  INTROSPECTION_TYPELIBDIR
#  INTROSPECTION_CFLAGS
#  INTROSPECTION_LIBS
#
# Copyright (C) 2010, Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(_GIR_GET_PKGCONFIG_VAR _outvar _varname)
  execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${_varname} gobject-introspection-1.0
    OUTPUT_VARIABLE _result
    RESULT_VARIABLE _null
  )

  if (_null)
  else()
    string(REGEX REPLACE "[\r\n]" " " _result "${_result}")
    string(REGEX REPLACE " +$" ""  _result "${_result}")
    separate_arguments(_result)
    set(${_outvar} ${_result} CACHE INTERNAL "")
  endif()
endmacro(_GIR_GET_PKGCONFIG_VAR)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  if(PACKAGE_FIND_VERSION_COUNT GREATER 0)
    set(_gir_version_cmp ">=${PACKAGE_FIND_VERSION}")
  endif()
  pkg_check_modules(_pc_gir gobject-introspection-1.0${_gir_version_cmp})
  if(_pc_gir_FOUND)
    set(INTROSPECTION_FOUND TRUE)
    _gir_get_pkgconfig_var(INTROSPECTION_SCANNER "g_ir_scanner")
    _gir_get_pkgconfig_var(INTROSPECTION_COMPILER "g_ir_compiler")
    _gir_get_pkgconfig_var(INTROSPECTION_GENERATE "g_ir_generate")
    _gir_get_pkgconfig_var(INTROSPECTION_GIRDIR "girdir")
    _gir_get_pkgconfig_var(INTROSPECTION_TYPELIBDIR "typelibdir")
    set(INTROSPECTION_CFLAGS "${_pc_gir_CFLAGS}")
    set(INTROSPECTION_LIBS "${_pc_gir_LIBS}")
  endif()
endif()

mark_as_advanced(
  INTROSPECTION_SCANNER
  INTROSPECTION_COMPILER
  INTROSPECTION_GENERATE
  INTROSPECTION_GIRDIR
  INTROSPECTION_TYPELIBDIR
  INTROSPECTION_CFLAGS
  INTROSPECTION_LIBS
)
