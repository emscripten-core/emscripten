# Copyright (C) 2010, Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(_gir_list_prefix _outvar _listvar _prefix)
  set(${_outvar})
  foreach(_item IN LISTS ${_listvar})
    list(APPEND ${_outvar} ${_prefix}${_item})
  endforeach()
endmacro(_gir_list_prefix)

macro(gir_add_introspections introspections_girs)

  set(_gir_girs)
  set(_gir_typelibs)

  foreach(gir IN LISTS ${introspections_girs})

    set(_gir_name "${gir}")

    ## Transform the gir filename to something which can reference through a variable
    ## without automake/make complaining, eg Gtk-2.0.gir -> Gtk_2_0_gir
    string(REPLACE "-" "_" _gir_name "${_gir_name}")
    string(REPLACE "." "_" _gir_name "${_gir_name}")

    # Namespace and Version is either fetched from the gir filename
    # or the _NAMESPACE/_VERSION variable combo
    set(_gir_namespace "${${_gir_name}_NAMESPACE}")
    if (_gir_namespace STREQUAL "")
      string(REGEX REPLACE "([^-]+)-.*" "\\1" _gir_namespace "${gir}")
    endif ()
    set(_gir_version "${${_gir_name}_VERSION}")
    if (_gir_version STREQUAL "")
      string(REGEX REPLACE ".*-([^-]+).gir" "\\1" _gir_version "${gir}")
    endif ()

    # _PROGRAM is an optional variable which needs it's own --program argument
    set(_gir_program "${${_gir_name}_PROGRAM}")
    if (NOT _gir_program STREQUAL "")
      set(_gir_program "--program=${_gir_program}")
    endif ()

    # Variables which provides a list of things
    _gir_list_prefix(_gir_libraries ${_gir_name}_LIBS "--library=")
    _gir_list_prefix(_gir_packages ${_gir_name}_PACKAGES "--pkg=")
    _gir_list_prefix(_gir_includes ${_gir_name}_INCLUDES "--include=")

    # Reuse the LIBTOOL variable from by automake if it's set
    set(_gir_libtool "--no-libtool")

    add_custom_command(
      COMMAND ${INTROSPECTION_SCANNER}
              ${INTROSPECTION_SCANNER_ARGS}
              --namespace=${_gir_namespace}
              --nsversion=${_gir_version}
              ${_gir_libtool}
              ${_gir_program}
              ${_gir_libraries}
              ${_gir_packages}
              ${_gir_includes}
              ${${_gir_name}_SCANNERFLAGS}
              ${${_gir_name}_CFLAGS}
              ${${_gir_name}_FILES}
              --output ${CMAKE_CURRENT_BINARY_DIR}/${gir}
      DEPENDS ${${_gir_name}_FILES}
              ${${_gir_name}_LIBS}
      OUTPUT ${gir}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      VERBATIM
    )
    list(APPEND _gir_girs ${CMAKE_CURRENT_BINARY_DIR}/${gir})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${gir} DESTINATION share/gir-1.0)

    string(REPLACE ".gir" ".typelib" _typelib "${gir}")
    add_custom_command(
      COMMAND ${INTROSPECTION_COMPILER}
              ${INTROSPECTION_COMPILER_ARGS}
              --includedir=.
              ${CMAKE_CURRENT_BINARY_DIR}/${gir}
              -o ${CMAKE_CURRENT_BINARY_DIR}/${_typelib}
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${gir}
      OUTPUT ${_typelib}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    list(APPEND _gir_typelibs ${CMAKE_CURRENT_BINARY_DIR}/${_typelib})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_typelib} DESTINATION lib${LIB_SUFFIX}/girepository-1.0)

  endforeach()

  add_custom_target(gir-girs ALL DEPENDS ${_gir_girs})
  add_custom_target(gir-typelibs ALL DEPENDS ${_gir_typelibs})

endmacro(gir_add_introspections)
